//
//  search.cpp
//  Seanet
//
//  Created by Nathaniel Corley on 5/16/16.
//
//

#include "search.hpp"
#include <algorithm>

int quiescencePly = 0;
Move killerMoves[64 * 3];     // 64 mAX plys with 3 moves each.
int historyHeuristic[64][64]; // historyHeuristic[from][to]
S_PVLINE NULL_LINE;

/**
Takes input of a state, time limit, and (optional) max depth, returns the best
move for the state.sideToMove() as an int.
 **/
void search(State &state, SearchController &sControl) {
  sControl.resetStats();
  sControl.getAllottedTime(state._fullMoveCounter);
  sControl.getAllottedTime(state._fullMoveCounter);
  std::cout << "ALLOTTED TIME: " << sControl._timeLimit << std::endl;
  initHashTable(&sControl.table);
  state._bestLine = S_PVLINE();

  // reset heuristics
  for (int i = 0; i < 192; i++) {
    killerMoves[i] = 0;
  }
  for (int from = 0; from < 64; from++) {
    for (int to = 0; to < 64; to++) {
      historyHeuristic[from][to] = 0;
    }
  }
  int alpha = INT_MIN + 1;
  int beta = INT_MAX - 1;
  for (int depth = 1; depth <= sControl._depthLimit; depth++) {
    sControl._currDepth = depth;
    sControl._maxDepth = depth;
    int eval;

    while (true) {
      if (beta - alpha > ASP_WINDOW * 4) {
        alpha = INT_MIN + 1;
        beta = INT_MAX - 1;
      }
      eval = negamax(alpha, beta, depth, state, sControl, state._bestLine);
      if (sControl._stopSearch) {
        break;
      };
      if (!DEBUG || sControl._features[ASPIRATION_WINDOWS]) {
        if (eval <= alpha) {
          alpha -= ASP_WINDOW;

          continue;
        }
        if (eval >= beta) {
          beta += ASP_WINDOW;

          continue;
        }
        alpha = eval - ASP_WINDOW;
        beta = eval + ASP_WINDOW;
        break;
      } else {
        break;
      }
    }
    if (sControl._stopSearch) {
      break;
    };

    if (eval != state._lineEval) {
      std::cout << "eval != state._lineEval: " << eval
                << " != " << state._lineEval << std::endl;
      std::cout << "alpha: " << alpha << "; beta: " << beta << std::endl;
    };
    if (DEBUG) {
      // assert(eval == state._lineEval);
    }

    timeval currTime;
    gettimeofday(&currTime, 0);
    int timeElapsed =
        (int)(timeToMS(currTime) - timeToMS(sControl._startTime)) + 1;
    if (sControl._output) {
      if (sControl._uciOutput) {
        std::cout << "info"
                  << " depth " << sControl._currDepth << " seldepth "
                  << sControl._maxDepth << " time " << timeElapsed << " nodes "
                  << sControl._totalNodes << " score cp " << state._lineEval
                  << " nps "
                  << (int)(sControl._totalNodes / (timeElapsed / 1000.0))
                  << " pv " << pvLineToString(state._bestLine) << std::endl;
      } else {
        std::cout << sControl._currDepth << " ["
                  << state._lineEval * (state._sideToMove == WHITE ? 1 : -1)
                  << "] " << pvLineToString(state._bestLine) << "; "
                  << timeElapsed << " ms; "
                  << (int)(sControl._totalNodes / (timeElapsed)) << " kn/s"
                  << "; "
                  << (float)(100.0 * sControl._fhfNodes / sControl._fhNodes)
                  << "% fhf"
                  << "; "
                  << (float)(100.0 * sControl._fhNodes / sControl._totalNodes)
                  << "% fh"
                  << "; " << (sControl._totalNodes / 1000) << "K nodes"
                  << "; seldepth " << sControl._maxDepth << std::endl;
      }
    }
  }
  sControl._stopSearch = true;
  if (sControl._uciOutput) {
    std::cout << "bestmove " << moveToUCI(state._bestLine.moves[0]);
    if (state._bestLine.moves[1]) {
      std::cout << " ponder " << moveToUCI(state._bestLine.moves[1]);
    }
    std::cout << std::endl;
  }
}

void addKillerMove(int ply, Move move) {
  int firstMoveIndex = ply * 3;
  if (M_EQUALS(move, killerMoves[firstMoveIndex])) {
  } else if (M_EQUALS(move, killerMoves[firstMoveIndex + 1])) {
    std::swap(killerMoves[firstMoveIndex], killerMoves[firstMoveIndex + 1]);
  } else {
    std::swap(killerMoves[firstMoveIndex + 1], killerMoves[firstMoveIndex + 2]);
    std::swap(killerMoves[firstMoveIndex], killerMoves[firstMoveIndex + 1]);
    killerMoves[firstMoveIndex] = move;
  }
}

void pickMove(int moveNum, std::vector<S_MOVE_AND_SCORE> &scoredMoves) {
  int bestScore = INT_MIN;
  int bestNum = moveNum;
  for (std::vector<S_MOVE_AND_SCORE>::iterator it =
           scoredMoves.begin() + moveNum;
       it != scoredMoves.end(); it++) {
    if (it->score > bestScore) {
      bestScore = it->score;
      bestNum = (int)(it - scoredMoves.begin());
    }
  }
  std::swap(scoredMoves[moveNum], scoredMoves[bestNum]);
}

int negamax(int alpha, int beta, int depth, State &state,
            SearchController &sControl, S_PVLINE &pvLine) {
  sControl._totalNodes++;
  sControl._mainNodes++;
  if ((sControl._totalNodes & 10240) == 0) {
    sControl.checkTimeLimit();
  }

  if (isThreeFoldRepetition(state) && state._ply != 0) {
    return evaluateDraw(state, sControl._contemptFactor);
  }

  // Check if TT entry exists for given state, and return stored score

  Move bestTTMove = NO_MOVE;
  if (!DEBUG || sControl._features[TT_EVAL] ||
      sControl._features[TT_REORDERING]) {
    S_HASHENTRY oldEntry = probeHashTable(sControl.table, state._zHash);
    if (oldEntry != NULL_ENTRY && oldEntry.zobrist == state._zHash) {
      sControl._transpositions++;
      if (!DEBUG || sControl._features[TT_EVAL]) {
        if (oldEntry.depth >= depth) {
          if (oldEntry.type == EXACT) {
            sControl._exactNodes++;
            pvLine.moves[0] = oldEntry.move;
            pvLine.moveCount = 1;
            if (state._ply == 0) {
              state._lineEval = oldEntry.score;
            }
            return oldEntry.score;
          }
          if (oldEntry.type == ALPHA && oldEntry.score <= alpha) {
            sControl._alphaNodes++;
            return alpha;
          }
          if (oldEntry.type == BETA && oldEntry.score >= beta) {
            sControl._betaNodes++;
            return beta;
          }
        }
      }
      if (!DEBUG || sControl._features[TT_REORDERING]) {
        bestTTMove = oldEntry.move;
      }
    }
  }

  // Check if search is at final depth
  if (depth <= 0) {
    pvLine.moveCount = 0;
    int score = qSearch(alpha, beta, state, sControl);
    return score;
  }

  if ((!DEBUG || sControl._features[NULL_MOVE]) &&
      !state.isInCheck(state._sideToMove)) {

    state.makeNullMove();
    state._ply++;
    int score =
        -negamax(-beta, -beta + 1, depth - 3, state, sControl, NULL_LINE);
    state._ply--;
    state.takeNullMove();
    if (score >= beta) {
      if (!DEBUG || sControl._features[TT_EVAL] ||
          sControl._features[TT_REORDERING]) {
        storeHashEntry(state._zHash, depth, beta, NO_MOVE, BETA,
                       sControl.table);
      }
      return beta;
    }
  }

  S_PVLINE line;

  NodeType flag = ALPHA;
  int legal = 0;

  if (bestTTMove) {
    state.makeMove(bestTTMove);

    legal++;

    state._ply++;
    int score = -negamax(-beta, -alpha, depth - 1, state, sControl, line);
    state._ply--;
    state.takeMove();
    if (score >= beta) {
      if (!DEBUG || sControl._features[TT_EVAL] ||
          sControl._features[TT_REORDERING]) {
        storeHashEntry(state._zHash, depth, beta, bestTTMove, BETA,
                       sControl.table);
      }
      return beta;
    }
    if (score > alpha) {
      alpha = score;
      flag = EXACT;
      pvLine.moves[0] = bestTTMove;
      memcpy(pvLine.moves + 1, line.moves, line.moveCount * sizeof(Move));
      pvLine.moveCount = line.moveCount + 1;
      if (state._ply == 0) {
        state._lineEval = score;
      }
    }
  }
  std::vector<S_MOVE_AND_SCORE> scoredMoves;
  std::vector<Move> moves = generatePseudoMoves(state);

  scoredMoves.reserve(moves.size());

  for (std::vector<Move>::iterator it = moves.begin(); it != moves.end();
       ++it) {

    // PV-move reorder
    if (!DEBUG || sControl._features[TT_REORDERING]) {
      if (bestTTMove && M_EQUALS(*it, bestTTMove)) {
        continue;
      }
    } else if (state._ply == 0 && sControl._features[PV_REORDERING] &&
               M_EQUALS(*it, state._bestLine.moves[state._ply])) {
      scoredMoves.push_back(S_MOVE_AND_SCORE{*it, 1000000});
      continue;
    }

    //   Reorder based on SEE
    if ((!DEBUG || sControl._features[SEE_REORDERING]) &&
        state._pieces[M_TOSQ(*it)] != EMPTY) {
      int seeEval = see(*it, state);
      if (seeEval >= 0) {
        scoredMoves.push_back(S_MOVE_AND_SCORE{*it, 950000 + seeEval});
        continue;
      } else if (seeEval < 0) {
        scoredMoves.push_back(S_MOVE_AND_SCORE{*it, seeEval});
        continue;
      }
    }

    // Killer moves
    if (!DEBUG || sControl._features[KH_REORDERING]) {
      if (M_EQUALS(*it, killerMoves[state._ply * 3])) {
        scoredMoves.push_back(S_MOVE_AND_SCORE{*it, 900000});
        continue;
      } else if (M_EQUALS(*it, killerMoves[state._ply * 3 + 1])) {
        scoredMoves.push_back(S_MOVE_AND_SCORE{*it, 850000});
        continue;
      } else if (M_EQUALS(*it, killerMoves[state._ply * 3 + 2])) {
        scoredMoves.push_back(S_MOVE_AND_SCORE{*it, 800000});
        continue;
      }
    }

    // Non captures sorted by history heuristic
    if (!DEBUG || sControl._features[HH_REORDERING]) {
      int hhScore = historyHeuristic[M_FROMSQ(*it)][M_TOSQ(*it)];
      if (hhScore > 700000) {
        hhScore = 700000;
      }
      scoredMoves.push_back(S_MOVE_AND_SCORE{*it, hhScore});
      continue;
    }
    scoredMoves.push_back(S_MOVE_AND_SCORE{*it, 0});
  }

  // pvLine.moves[0] = NO_MOVE;
  for (int moveNum = 0; moveNum < scoredMoves.size(); moveNum++) {
    // stop search if found a checkmate
    if (alpha >= CHECKMATE) {
      break;
    }
    pickMove(moveNum, scoredMoves);
    Move move = scoredMoves[moveNum].move;
    state.makeMove(move);

    if (!state.isPositionLegal()) {
      state.takeMove();
      continue;
    }
    legal++;
    if (state._ply == 0) {
      sControl._currMove = move;
      sControl._currMoveNumber++;
    }
    state._ply++;
    int score;
    if (flag == EXACT && (!DEBUG || sControl._features[PV_SEARCH])) {
      score = -negamax(-alpha - 1, -alpha, depth - 1, state, sControl, line);
      if ((score > alpha) && (score < beta)) // Check for failure.
        score = -negamax(-beta, -alpha, depth - 1, state, sControl, line);
    } else {
      score = -negamax(-beta, -alpha, depth - 1, state, sControl, line);
    }
    state._ply--;

    state.takeMove();
    if (sControl._stopSearch) {
      return 0;
    }

    if (score > alpha) {
      if (score >= beta) {
        if (!DEBUG || sControl._features[TT_EVAL] ||
            sControl._features[TT_REORDERING]) {
          storeHashEntry(state._zHash, depth, beta, move, BETA, sControl.table);
        }
        if (legal == 1) {
          sControl._fhfNodes++;
        }
        sControl._fhNodes++;
        if (!M_ISCAPTURE(move)) {

          if (!DEBUG || sControl._features[KH_REORDERING]) {
            addKillerMove(state._ply, move);
          }
          if (!DEBUG || sControl._features[HH_REORDERING]) {
            historyHeuristic[M_FROMSQ(move)][M_TOSQ(move)] += depth * depth;
          }
        }
        return beta; // Fail hard beta-cutoff
      }

      flag = EXACT;
      alpha = score; // Update value of "best path so far for maximizer"
      pvLine.moves[0] = move;
      memcpy(pvLine.moves + 1, line.moves, line.moveCount * sizeof(Move));
      pvLine.moveCount = line.moveCount + 1;
      if (state._ply == 0) {
        state._lineEval = score;
      }
    }
  }
  if (!legal) {
    pvLine.moves[0] = NO_MOVE;
    pvLine.moveCount = 0;
    flag = EXACT;
    alpha = evaluateGameOver(state, sControl._contemptFactor);
    if (state._ply == 0) {
      state._lineEval = alpha;
    }
  }
  if (!DEBUG || sControl._features[TT_EVAL] ||
      sControl._features[TT_REORDERING]) {
    if (flag == ALPHA) {
      storeHashEntry(state._zHash, depth, alpha, NO_MOVE, flag, sControl.table);
    } else {
      storeHashEntry(state._zHash, depth, alpha, pvLine.moves[0], flag,
                     sControl.table);
    }
  }
  return alpha;
}

int qSearch(int alpha, int beta, State &state, SearchController &sControl) {
  if (state._ply > sControl._maxDepth) {
    sControl._maxDepth = state._ply;
  }
  sControl._totalNodes++;

  sControl._qNodes++;
  if ((sControl._totalNodes & 10240) == 0) {
    sControl.checkTimeLimit();
  }
  if (isThreeFoldRepetition(state)) {
    return evaluateDraw(state, sControl._contemptFactor);
  }

  Move bestTTMove = NO_MOVE;
  if (!DEBUG || sControl._features[TT_EVAL] ||
      sControl._features[TT_REORDERING]) {
    S_HASHENTRY oldEntry = probeHashTable(sControl.table, state._zHash);
    if (oldEntry != NULL_ENTRY && oldEntry.zobrist == state._zHash) {
      sControl._transpositions++;
      if (!DEBUG || sControl._features[TT_EVAL]) {
        if (oldEntry.type == EXACT) {
          sControl._exactNodes++;
          return oldEntry.score;
        }
        if (oldEntry.type == ALPHA && oldEntry.score <= alpha) {
          sControl._alphaNodes++;
          return alpha;
        }
        if (oldEntry.type == BETA && oldEntry.score >= beta) {
          sControl._betaNodes++;
          return beta;
        }
      }
      if (!DEBUG || sControl._features[TT_REORDERING]) {
        bestTTMove = oldEntry.move;
      }
    }
  }

  NodeType flag = ALPHA;

  bool inCheck = state.isInCheck(state._sideToMove);

  std::vector<int> moves = generatePseudoMoves(state, inCheck);
  int legal = 0;

  int stand_pat = evaluate(state) * (state._sideToMove == WHITE ? 1 : -1);
  Move bestMove = NO_MOVE;
  if (stand_pat >= beta) {
    alpha = beta;
    flag = BETA;
  } else {

    if (alpha < stand_pat) {
      alpha = stand_pat;
    }
    std::vector<S_MOVE_AND_SCORE> scoredMoves;
    scoredMoves.reserve(moves.size());

    for (std::vector<Move>::iterator it = moves.begin(); it != moves.end();
         ++it) {
      if (!inCheck && state._pieces[M_TOSQ(*it)] == EMPTY &&
          !M_ISPROMOTION(*it)) {
        continue;
      }

      // PV-move reorder
      if (!DEBUG || (sControl._features[QS_REORDERING] &&
                     sControl._features[TT_REORDERING])) {
        if (bestTTMove && M_EQUALS(*it, bestTTMove)) {
          scoredMoves.push_back(S_MOVE_AND_SCORE{*it, 1000000});
          continue;
        }
      }

      //   Reorder based on SEE
      if ((!DEBUG || (sControl._features[QS_REORDERING] &&
                      sControl._features[SEE_REORDERING]))) {
        int seeEval = see(*it, state);
        if (seeEval >= 0) {
          scoredMoves.push_back(S_MOVE_AND_SCORE{*it, seeEval});
          continue;
        } else if (seeEval < 0) {
          if (!inCheck) {
            continue;
          }
          scoredMoves.push_back(S_MOVE_AND_SCORE{*it, seeEval});
          continue;
        }
      } else {
        if (!inCheck && see(*it, state) < 0) {
          continue;
        }
      }

      scoredMoves.push_back(S_MOVE_AND_SCORE{*it, 0});
    }
    for (int moveNum = 0; moveNum < scoredMoves.size(); moveNum++) {
      // stop search if found a checkmate
      if (alpha >= CHECKMATE) {
        break;
      }
      pickMove(moveNum, scoredMoves);
      Move move = scoredMoves[moveNum].move;

      state.makeMove(move);

      if (!state.isPositionLegal()) {
        state.takeMove();
        continue;
      }
      legal++;
      state._ply++;
      quiescencePly++;
      int score = -qSearch(-beta, -alpha, state, sControl);
      state._ply--;
      quiescencePly--;

      state.takeMove();

      if (score >= beta) {
        if (legal == 1) {
          sControl._fhfNodes++;
        }
        sControl._fhNodes++;
        alpha = beta;
        flag = BETA;
        bestMove = move;
        break;
      }
      if (score > alpha) {
        flag = EXACT;
        alpha = score;
        bestMove = move;
      }
    }
  }
  if (alpha == beta) { // check game over for beta cutoff
    if (isGameOver(state, moves)) {
      bestMove = NO_MOVE;
      flag = EXACT;
      alpha = evaluateGameOver(state, sControl._contemptFactor);
    }
  } else {
    if (!legal && (inCheck || isGameOver(state, moves))) {
      bestMove = NO_MOVE;
      flag = EXACT;
      alpha = evaluateGameOver(state, sControl._contemptFactor);
    }
  }
  if (!DEBUG || sControl._features[TT_EVAL] ||
      sControl._features[TT_REORDERING]) {
    storeHashEntry(state._zHash, 0, alpha, bestMove, flag, sControl.table);
  }
  return alpha;
}

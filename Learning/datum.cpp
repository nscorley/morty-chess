//
//  datum.cpp
//  Seanet
//
//  Created by Douglas Corley on 6/1/16.
//
//

#include "datum.hpp"

Datum::Datum(State state, Move bestMove, int contemptFactor)
    : _state(state), _bestMove(bestMove), _contemptFactor(contemptFactor) {}

int simpleQSearch(int alpha, int beta, State &state) {
  bool inCheck = state.isInCheck(state._sideToMove);

  std::vector<int> moves = generatePseudoMoves(state, inCheck);
  int legal = 0;

  int stand_pat = state._material * (state._sideToMove == WHITE ? 1 : -1);
  if (stand_pat >= beta) {
    alpha = beta;
  } else {

    if (alpha < stand_pat) {
      alpha = stand_pat;
    }
    for (Move move : moves) {

      // stop search if found a checkmate
      if (alpha >= CHECKMATE) {
        break;
      }

      if (!inCheck &&
          ((state._pieces[M_TOSQ(move)] == EMPTY && !M_ISPROMOTION(move)) ||
           see(move, state) < 0)) {
        continue;
      }

      state.makeMove(move);

      if (!state.isPositionLegal()) {
        state.takeMove();
        continue;
      }
      legal++;
      state._ply++;
      int score = -simpleQSearch(-beta, -alpha, state);
      state._ply--;

      state.takeMove();

      if (score >= beta) {
        alpha = beta;
        break;
      }
      if (score > alpha) {
        alpha = score;
      }
    }
  }
  if (alpha == beta) { // check game over for beta cutoff
    if (isGameOver(state, moves)) {
      alpha = evaluateGameOver(state, -1);
    }
  } else {
    if (!legal && (inCheck || isGameOver(state, moves))) {
      alpha = evaluateGameOver(state, -1);
    }
  }
  return alpha;
}
bool isPseudoQuiet(State &state) {
  if (state.isInCheck(state._sideToMove)) {
    //    std::cout << "NOT PSEUDO QUIET BECAUSE IN CHECK" << std::endl;
    return false;
  }

  int standPat = state._material * (state._sideToMove == WHITE ? 1 : -1);
  if (simpleQSearch(INT_MIN, INT_MAX, state) != standPat) {
    //    std::cout << "simpleQSearch(INT_MIN, -INT_MAX, state) != standPat: "
    //              << simpleQSearch(INT_MIN, INT_MAX, state) << " != " <<
    //              standPat
    //              << std::endl;
    return false;
  }

  state.makeNullMove();
  int nullMoveScore = -simpleQSearch(INT_MIN, INT_MAX, state);
  state.takeNullMove();
  if (nullMoveScore != standPat) {
    //    std::cout << "nullMoveScore != standPat: " << nullMoveScore
    //              << " != " << standPat << std::endl;
    return false;
  }
  return true;
}

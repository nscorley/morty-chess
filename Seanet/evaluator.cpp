//
//  evaluator.cpp
//  Seanet
//
//  Created by Nathaniel Corley on 5/16/16.
//
//

#include "evaluator.hpp"
#include "movegenerator.hpp"

// Piece-Square tables

int whitePawnPS[64] = {0,  0,  0,  0,  0,  0,   0,  0,  5,   10, 10, -20, -20,
                       10, 10, 5,  5,  -5, -10, 0,  0,  -10, -5, 5,  0,   0,
                       0,  20, 20, 0,  0,  0,   5,  5,  10,  25, 25, 10,  5,
                       5,  10, 10, 20, 30, 30,  20, 10, 10,  50, 50, 50,  50,
                       50, 50, 50, 50, 0,  0,   0,  0,  0,   0,  0,  0};

int whiteKnightPS[64] = {-50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,
                         5,   5,   0,   -20, -40, -30, 5,   10,  15,  15,  10,
                         5,   -30, -30, 0,   15,  20,  20,  15,  0,   -30, -30,
                         5,   15,  20,  20,  15,  5,   -30, -30, 0,   10,  15,
                         15,  10,  0,   -30, -40, -20, 0,   0,   0,   0,   -20,
                         -40, -50, -40, -30, -30, -30, -30, -40, -50};
int whiteBishopPS[64] = {-20, -10, -10, -10, -10, -10, -10, -20, -10, 5,   0,
                         0,   0,   0,   5,   -10, -10, 10,  10,  10,  10,  10,
                         10,  -10, -10, 0,   10,  10,  10,  10,  0,   -10, -10,
                         5,   5,   10,  10,  5,   5,   -10, -10, 0,   5,   10,
                         10,  5,   0,   -10, -10, 0,   0,   0,   0,   0,   0,
                         -10, -20, -10, -10, -10, -10, -10, -10, -20};
int whiteRookPS[64] = {0,  0,  0,  5,  5,  0,  0,  0,  -5, 0, 0, 0, 0, 0, 0, -5,
                       -5, 0,  0,  0,  0,  0,  0,  -5, -5, 0, 0, 0, 0, 0, 0, -5,
                       -5, 0,  0,  0,  0,  0,  0,  -5, -5, 0, 0, 0, 0, 0, 0, -5,
                       5,  10, 10, 10, 10, 10, 10, 5,  0,  0, 0, 0, 0, 0, 0, 0};

int whiteQueenPS[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20, -10, 0,   5,   0,  0,  0,   0,   -10,
    -10, 5,   5,   5,  5,  5,   0,   -10, 0,   0,   5,   5,  5,  5,   0,   -5,
    -5,  0,   5,   5,  5,  5,   0,   -5,  -10, 0,   5,   5,  5,  5,   0,   -10,
    -10, 0,   0,   0,  0,  0,   0,   -10, -20, -10, -10, -5, -5, -10, -10, -20};
int whiteKingMiddlePS[64] = {
    20,  30,  10,  0,   0,   10,  30,  20,  20,  20,  0,   0,   0,
    0,   20,  20,  -10, -20, -20, -20, -20, -20, -20, -10, -20, -30,
    -30, -40, -40, -30, -30, -20, -30, -40, -40, -50, -50, -40, -40,
    -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50,
    -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30};
int whiteKingEndPS[64] = {-50, -30, -30, -30, -30, -30, -30, -50, -30, -30, 0,
                          0,   0,   0,   -30, -30, -30, -10, 20,  30,  30,  20,
                          -10, -30, -30, -10, 30,  40,  40,  30,  -10, -30, -30,
                          -10, 30,  40,  40,  30,  -10, -30, -30, -10, 20,  30,
                          30,  20,  -10, -30, -30, -20, -10, 0,   0,   -10, -20,
                          -30, -50, -40, -30, -20, -20, -30, -40, -50};

int allPieces[65];

int calculatePhase(State &state) {
  int currentPhase =
      countSetBits(state._pieceBitboards[PAWNS]) * MATERIAL_PHASE[wP] +
      countSetBits(state._pieceBitboards[KNIGHTS]) * MATERIAL_PHASE[wK] +
      countSetBits(state._pieceBitboards[BISHOPS]) * MATERIAL_PHASE[wB] +
      countSetBits(state._pieceBitboards[ROOKS]) * MATERIAL_PHASE[wR] +
      countSetBits(state._pieceBitboards[QUEENS]) * MATERIAL_PHASE[wQ];
  return ((TOTAL_PHASE - currentPhase) * 256 + (TOTAL_PHASE / 2)) / TOTAL_PHASE;
}

int calculateMaterial(State &state) {
  allPieces[0] = -1;
  getSetBits(state.allPieces(), allPieces);

  int score = 0;
  for (int i = 0; allPieces[i] != -1; i++) {
    int pcIndex = allPieces[i];
    Piece piece = state._pieces[pcIndex];

    score += MATERIAL_WORTH[piece];
  }

  return score;
}

int evaluate(State &state) {
  allPieces[0] = -1;
  getSetBits(state.allPieces(), allPieces);

  int score = state._material;
  for (int i = 0; allPieces[i] != -1; i++) {
    int pcIndex = allPieces[i];
    Piece piece = state._pieces[pcIndex];

    switch (piece) {
    case wP:
      score += whitePawnPS[pcIndex];
      break;
    case bP:
      score -= whitePawnPS[63 - pcIndex];
      break;
    case wN:
      score += whiteKnightPS[pcIndex];
      break;
    case bN:
      score -= whiteKnightPS[63 - pcIndex];
      break;
    case wB:
      score += whiteBishopPS[pcIndex];
      break;
    case bB:
      score -= whiteBishopPS[63 - pcIndex];
      break;
    case wR:
      score += whiteRookPS[pcIndex];
      break;
    case bR:
      score -= whiteRookPS[63 - pcIndex];
      break;
    case wQ:
      score += whiteQueenPS[pcIndex];
      break;
    case bQ:
      score -= whiteQueenPS[63 - pcIndex];
      break;
    case wK:
      score += ((whiteKingMiddlePS[pcIndex] * (256 - state._phase)) +
                (whiteKingEndPS[pcIndex] * state._phase)) /
               256;
      // score += whiteKingMiddlePS[pcIndex];
      break;
    case bK:
      score -= ((whiteKingMiddlePS[63 - pcIndex] * (256 - state._phase)) +
                (whiteKingEndPS[63 - pcIndex] * state._phase)) /
               256;
      // score -= whiteKingMiddlePS[63 - pcIndex];
      break;
    default:
      break;
    }
  }
  return score;
}

int evaluateGameOver(State &state, int contempt) {
  if (state.isInCheck(state._sideToMove)) {
    return -CHECKMATE;
  } else {
    // stalemate
    return evaluateDraw(state, contempt);
  }
}

bool isGameOver(State &state, std::vector<Move> moves) {
  for (Move move : moves) {
    if (state.isLegalMove(move)) {
      return false;
    }
  }
  return true;
}

int evaluateDraw(State &state, int contempt) {
  return state._ply % 2 == 0 ? contempt : -contempt;
}

bool isThreeFoldRepetition(State &state) {
  if (state._history.empty()) {
    return false;
  }
  S_UNDO lastUndo = state._history.back();

  for (std::vector<S_UNDO>::reverse_iterator rit = state._history.rbegin();
       rit <= state._history.rbegin() + lastUndo._halfMoveClock &&
       rit != state._history.rend();
       ++rit)
    if (rit->_zHash == state._zHash) {
      return true;
    }
  return false;
}

bool isGameOver(State &state) {
  return isGameOver(state, generatePseudoMoves(state));
}

bool isIsolatedPawn(State &state, int index) {
  U64 friendlyPawns =
      state._pieceBitboards[PAWNS] &
      state._pieceBitboards[sideBitboardForPiece(state._pieces[index])];
  U64 fileBB = FILE_BB[index % 8];
  return ((RIGHT(fileBB) | LEFT(fileBB)) & friendlyPawns) == 0;
}

bool isIsolaniPawn(State &state, int index) {
  return index % 8 == 3 && isIsolatedPawn(state, index);
}
bool isDoubledPawn(State &state, int index) {
  U64 friendlyPawns =
      state._pieceBitboards[PAWNS] &
      state._pieceBitboards[sideBitboardForPiece(state._pieces[index])];
  U64 fileBB = FILE_BB[index % 8];
  return (CLRBIT(fileBB, index) & friendlyPawns);
}
bool isConnectedPawn(State &state, int index) { return false; }
bool isBackwardPawn(State &state, int index) { return false; }
bool isBlockedPawn(State &state, int index) { return false; }
bool isPassedPawn(State &state, int index) { return false; }

int countIsolatedPawns(State &state) {
  int count = 0;

  return count;
}

int countIsolaniPawns(State &state) {
  int count = 0;

  return count;
}

int countDoubledPawns(State &state) {
  int count = 0;

  return count;
}

int countConnectedPawns(State &state) {
  int count = 0;

  return count;
}

int countBackwardPawns(State &state) {
  int count = 0;

  return count;
}

int countBlockedPawns(State &state) {
  int count = 0;

  return count;
}

int countPassedPawns(State &state) {
  int count = 0;

  return count;
}

int countHoles(State &state) {
  int count = 0;

  return count;
}

int countKingPawnShields(State &state) {
  int count = 0;

  return count;
}

FList getInitialFeatures(State &state) {
  FList features(NUM_OF_BOARD_FEATURES);
  int friendlyPieces[17];
  int enemyPieces[17];
  U64 friendlyBB =
      state._pieceBitboards[state._sideToMove == WHITE ? WHITES : BLACKS];
  U64 enemyBB =
      state._pieceBitboards[state._sideToMove == WHITE ? BLACKS : WHITES];

  getSetBits(friendlyBB, friendlyPieces);
  getSetBits(enemyBB, enemyPieces);

  for (int i = 0; enemyPieces[i] != -1; i++) {
    int pcIndex = enemyPieces[i];
    Piece piece = state._pieces[pcIndex];

    switch (piece) {
    case wP:
      features[ENEMY_PAWN_PS] += whitePawnPS[pcIndex];
      break;
    case bP:
      features[ENEMY_PAWN_PS] += whitePawnPS[63 - pcIndex];
      break;
    case wN:
      features[ENEMY_KNIGHT_PS] += whiteKnightPS[pcIndex];
      break;
    case bN:
      features[ENEMY_KNIGHT_PS] += whiteKnightPS[63 - pcIndex];
      break;
    case wB:
      features[ENEMY_BISHOP_PS] += whiteBishopPS[pcIndex];
      break;
    case bB:
      features[ENEMY_BISHOP_PS] += whiteBishopPS[63 - pcIndex];
      break;
    case wR:
      features[ENEMY_ROOK_PS] += whiteRookPS[pcIndex];
      break;
    case bR:
      features[ENEMY_ROOK_PS] += whiteRookPS[63 - pcIndex];
      break;
    case wQ:
      features[ENEMY_QUEEN_PS] += whiteQueenPS[pcIndex];
      break;
    case bQ:
      features[ENEMY_QUEEN_PS] += whiteQueenPS[63 - pcIndex];
      break;
    case wK:
      features[ENEMY_KING_PS] +=
          ((whiteKingMiddlePS[pcIndex] * (256 - state._phase)) +
           (whiteKingEndPS[pcIndex] * state._phase)) /
          256;
      break;
    case bK:
      features[ENEMY_KING_PS] +=
          ((whiteKingMiddlePS[63 - pcIndex] * (256 - state._phase)) +
           (whiteKingEndPS[63 - pcIndex] * state._phase)) /
          256;
      break;
    default:
      break;
    }
  }

  for (int i = 0; friendlyPieces[i] != -1; i++) {
    int pcIndex = friendlyPieces[i];
    Piece piece = state._pieces[pcIndex];

    switch (piece) {
    case wP:
      features[FRIENDLY_PAWN_PS] += whitePawnPS[pcIndex];
      break;
    case bP:
      features[FRIENDLY_PAWN_PS] += whitePawnPS[63 - pcIndex];
      break;
    case wN:
      features[FRIENDLY_KNIGHT_PS] += whiteKnightPS[pcIndex];
      break;
    case bN:
      features[FRIENDLY_KNIGHT_PS] += whiteKnightPS[63 - pcIndex];
      break;
    case wB:
      features[FRIENDLY_BISHOP_PS] += whiteBishopPS[pcIndex];
      break;
    case bB:
      features[FRIENDLY_BISHOP_PS] += whiteBishopPS[63 - pcIndex];
      break;
    case wR:
      features[FRIENDLY_ROOK_PS] += whiteRookPS[pcIndex];
      break;
    case bR:
      features[FRIENDLY_ROOK_PS] += whiteRookPS[63 - pcIndex];
      break;
    case wQ:
      features[FRIENDLY_QUEEN_PS] += whiteQueenPS[pcIndex];
      break;
    case bQ:
      features[FRIENDLY_QUEEN_PS] += whiteQueenPS[63 - pcIndex];
      break;
    case wK:
      features[FRIENDLY_KING_PS] +=
          ((whiteKingMiddlePS[pcIndex] * (256 - state._phase)) +
           (whiteKingEndPS[pcIndex] * state._phase)) /
          256;
      break;
    case bK:
      features[FRIENDLY_KING_PS] +=
          ((whiteKingMiddlePS[63 - pcIndex] * (256 - state._phase)) +
           (whiteKingEndPS[63 - pcIndex] * state._phase)) /
          256;
      break;
    default:
      break;
    }
  }

  return features;
}

float getPhaseFactor(State &state) { return (256.0 - state._phase) / 256.0; }
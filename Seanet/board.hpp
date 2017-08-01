//
//  board.hpp
//  Seanet
//
//  Created by Stiven Deleur on 5/12/16.
//
//

#ifndef board_hpp
#define board_hpp

#include "defs.h"

#include <stack>
#include <stdio.h>
#include <string>

class State;
class Undo;

#include "util.hpp"

class State {
public:
  U64 _pieceBitboards[8] = {0LL};
  Piece _pieces[64] = {EMPTY};
  int _castleRights = 0;
  int _EPTarget = -1;
  int _halfMoveClock;
  int _fullMoveCounter;
  int _sideToMove;
  std::vector<S_UNDO> _history;
  S_PVLINE _bestLine;
  int _lineEval;
  int _ply = 0;
  U64 _zHash = 0;
  int _phase = 0;
  int _material = 0;

  void printBoard() const;
  void makeMove(Move &move);
  void takeMove();

  void makeNullMove();
  void takeNullMove();

  void clearSquare(int index);
  void addPiece(Piece p, int index);
  void movePiece(int from, int to);

  U64 allPieces() const;
  int kingPos(int side) const;
  bool canCastle(int side, bool kSide) const;
  bool isInCheck(int side) const;
  bool isPositionLegal() const;
  bool isLegalMove(Move move);
  bool isLegalCheckEvasion(Move move);
  bool isAbsolutePin(int pinnedSq, int attackedSq, int defendingSide);
};

// inline functions:

inline void State::clearSquare(int index) {
  Piece p = _pieces[index];
  if (p != EMPTY) {
    CLRBIT(_pieceBitboards[bitboardForPiece(p)], index);
    CLRBIT(_pieceBitboards[sideBitboardForPiece(p)], index);
    _pieces[index] = EMPTY;
    _material -= MATERIAL_WORTH[p];
    _phase += (MATERIAL_PHASE[p] * 256) / TOTAL_PHASE;
  }
}

inline void State::addPiece(Piece p, int index) {
  clearSquare(index);
  SETBIT(_pieceBitboards[bitboardForPiece(p)], index);
  SETBIT(_pieceBitboards[sideBitboardForPiece(p)], index);
  _pieces[index] = p;
  _material += MATERIAL_WORTH[p];
  _phase -= (MATERIAL_PHASE[p] * 256) / TOTAL_PHASE;
}

inline void State::movePiece(int from, int to) {
  Piece p = _pieces[from];
  clearSquare(from);
  addPiece(p, to);
}

inline U64 State::allPieces() const {
  return _pieceBitboards[WHITES] | _pieceBitboards[BLACKS];
};

inline int State::kingPos(int side) const {
  return LS1B(
      (side == WHITE ? _pieceBitboards[WHITES] : _pieceBitboards[BLACKS]) &
      _pieceBitboards[KINGS]);
}

inline bool State::isPositionLegal() const {
  return !(isInCheck(-_sideToMove));
}

#endif /* board_hpp */

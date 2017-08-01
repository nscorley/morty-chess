//
//  movegenerator.cpp
//  Seanet
//
//  Created by Stiven Deleur on 5/12/16.
//
//

#include "movegenerator.hpp"
#include "util.hpp"

U64 pawnPush(U64 bb, int pawnSide, const State &s, bool moveTo = false);
U64 pawnAttack(int index, int pawnSide, const State &s);
U64 rookAttacks(int index, U64 consideredPieces);
U64 rookAttacks(int index, const State &s) {
  return rookAttacks(index, s.allPieces());
};
U64 bishopAttacks(int index, U64 consideredPieces);
U64 bishopAttacks(int index, const State &s) {
  return bishopAttacks(index, s.allPieces());
};
U64 magicMovesRook[64][4096];

U64 magicMovesBishop[64][4096];
U64 occupancyVariation[64][4096];

int pieceSquares[65];
int pieceMoves[65];

std::vector<Move> generateAllMoves(const State &s) {
  std::vector<Move> moves;
  moves.reserve(64);
  U64 friendlyBB = s._sideToMove == WHITE ? s._pieceBitboards[WHITES]
                                          : s._pieceBitboards[BLACKS];

  pieceSquares[0] = -1;
  pieceMoves[0] = -1;
  // King Moves
  int kingIndex = LS1B(friendlyBB & s._pieceBitboards[KINGS]);
  getSetBits(kingAttacks[kingIndex] & ~friendlyBB, pieceMoves);
  for (int to = 0; pieceMoves[to] != -1; to++) {
    moves.push_back(NEW_MOVE(kingIndex, pieceMoves[to]));
  }

  // Castling moves
  if (s.canCastle(s._sideToMove, true)) {
    moves.push_back(NEW_MOVE(kingIndex, (kingIndex + 2)));
  }
  if (s.canCastle(s._sideToMove, false)) {
    moves.push_back(NEW_MOVE(kingIndex, (kingIndex - 2)));
  }

  // Knight Moves
  getSetBits(friendlyBB & s._pieceBitboards[KNIGHTS], pieceSquares);

  for (int from = 0; pieceSquares[from] != -1; from++) {
    getSetBits(knightAttacks[pieceSquares[from]] & ~friendlyBB, pieceMoves);
    for (int to = 0; pieceMoves[to] != -1; to++) {
      moves.push_back(NEW_MOVE(pieceSquares[from], pieceMoves[to]));
    }
  }

  // Pawn moves
  getSetBits(friendlyBB & s._pieceBitboards[PAWNS], pieceSquares);

  for (int from = 0; pieceSquares[from] != -1; from++) {
    getSetBits((pawnPush(setMask[pieceSquares[from]], s._sideToMove, s) &
                ~s.allPieces()) |
                   pawnAttack(pieceSquares[from], s._sideToMove, s),
               pieceMoves);
    for (int to = 0; pieceMoves[to] != -1; to++) {

      if (pieceMoves[to] / 8 < 1 || pieceMoves[to] / 8 >= 7) {
        Move move1 = NEW_MOVE(pieceSquares[from], pieceMoves[to]);
        Move move2 = NEW_MOVE(pieceSquares[from], pieceMoves[to]);
        Move move3 = NEW_MOVE(pieceSquares[from], pieceMoves[to]);
        Move move4 = NEW_MOVE(pieceSquares[from], pieceMoves[to]);
        M_SETPROM(move1, bQ);
        M_SETPROM(move2, bN);
        M_SETPROM(move3, bR);
        M_SETPROM(move4, bB);
        moves.push_back(move1);
        moves.push_back(move2);
        moves.push_back(move3);
        moves.push_back(move4);
      } else {
        moves.push_back(NEW_MOVE(pieceSquares[from], pieceMoves[to]));
      }
    }
  }

  // Rook Moves
  getSetBits(friendlyBB & s._pieceBitboards[ROOKS], pieceSquares);

  for (int from = 0; pieceSquares[from] != -1; from++) {
    getSetBits(rookAttacks(pieceSquares[from], s) & ~friendlyBB, pieceMoves);
    for (int to = 0; pieceMoves[to] != -1; to++) {
      moves.push_back(NEW_MOVE(pieceSquares[from], pieceMoves[to]));
    }
  }

  // Bishop Moves
  getSetBits(friendlyBB & s._pieceBitboards[BISHOPS], pieceSquares);

  for (int from = 0; pieceSquares[from] != -1; from++) {
    getSetBits(bishopAttacks(pieceSquares[from], s) & ~friendlyBB, pieceMoves);
    for (int to = 0; pieceMoves[to] != -1; to++) {
      moves.push_back(NEW_MOVE(pieceSquares[from], pieceMoves[to]));
    }
  }

  // Queen Moves
  getSetBits(friendlyBB & s._pieceBitboards[QUEENS], pieceSquares);

  for (int from = 0; pieceSquares[from] != -1; from++) {
    getSetBits((rookAttacks(pieceSquares[from], s) |
                bishopAttacks(pieceSquares[from], s)) &
                   ~friendlyBB,
               pieceMoves);
    for (int to = 0; pieceMoves[to] != -1; to++) {
      moves.push_back(NEW_MOVE(pieceSquares[from], pieceMoves[to]));
    }
  }
  return moves;
}

void printMoves(std::string title, std::vector<Move> moves) {
  std::cout << title;
  for (std::vector<int>::iterator it = moves.begin(); it != moves.end(); ++it)
    std::cout << ' ' << moveToUCI(*it);
  std::cout << '\n';
}

std::vector<Move> generateCheckEvasions(const State &s) {
  std::vector<Move> moves;
  moves.reserve(64);
  U64 friendlyBB = s._sideToMove == WHITE ? s._pieceBitboards[WHITES]
                                          : s._pieceBitboards[BLACKS];
  U64 allPieces = s.allPieces();
  pieceSquares[0] = -1;
  pieceMoves[0] = -1;
  // King Moves
  int kingIndex = LS1B(friendlyBB & s._pieceBitboards[KINGS]);
  getSetBits(kingAttacks[kingIndex] & ~friendlyBB, pieceMoves);
  for (int to = 0; pieceMoves[to] != -1; to++) {
    moves.push_back(NEW_MOVE(kingIndex, pieceMoves[to]));
  }
  // printMoves("king moving", moves);

  // if double check, only king can move
  U64 attacksToKing = attacksTo(kingIndex, s, s._sideToMove);
  if (countSetBits(attacksToKing) == 1) {
    int kingAttackerIndex = LS1B(attacksToKing);
    int kingAttacker = s._pieces[kingAttackerIndex];

    getSetBits(attacksTo(kingAttackerIndex, s, -s._sideToMove), pieceSquares);
    for (int from = 0; pieceSquares[from] != -1; from++) {
      if (pieceSquares[from] == kingIndex) {
        continue;
      }
      Piece attackingPiece = s._pieces[pieceSquares[from]];
      if (attackingPiece == wP || attackingPiece == bP) {
        if (kingAttackerIndex / 8 < 1 || kingAttackerIndex / 8 >= 7) {
          Move move1 = NEW_MOVE(pieceSquares[from], kingAttackerIndex);
          Move move2 = NEW_MOVE(pieceSquares[from], kingAttackerIndex);
          Move move3 = NEW_MOVE(pieceSquares[from], kingAttackerIndex);
          Move move4 = NEW_MOVE(pieceSquares[from], kingAttackerIndex);
          M_SETPROM(move1, bQ);
          M_SETPROM(move2, bN);
          M_SETPROM(move3, bR);
          M_SETPROM(move4, bB);

          moves.push_back(move1);
          moves.push_back(move2);
          moves.push_back(move3);
          moves.push_back(move4);
        } else {
          moves.push_back(NEW_MOVE(pieceSquares[from], kingAttackerIndex));
        }
      } else {

        moves.push_back(NEW_MOVE(pieceSquares[from], kingAttackerIndex));
      }
    }
    // printMoves("captrure attacker", moves);

    // find blocking piece moves
    int indexDelta = kingIndex - kingAttackerIndex;
    pieceSquares[0] = -1; // reset piece squares

    // if its a horizontal/vertical attack path
    if (kingAttacker == wR || kingAttacker == bR || kingAttacker == wQ ||
        kingAttacker == bQ) {
      U64 bbBlockers = 0;
      // if attacking piece is on top/right
      if (indexDelta < 0) {
        // on the top
        if (indexDelta % 8 == 0) {
          bbBlockers = bbBlockers8Way[kingIndex][0];
        }
        // on the right
        else if (-indexDelta <= 7) {
          bbBlockers = bbBlockers8Way[kingIndex][1];
        }

      } else if (indexDelta > 0) {
        // on the bottom
        if (indexDelta % 8 == 0) {
          bbBlockers = bbBlockers8Way[kingIndex][2];
        }
        // on the left
        else if (indexDelta <= 7) {
          bbBlockers = bbBlockers8Way[kingIndex][3];
        }
      }
      if (bbBlockers) {
        bbBlockers &= ~setMask[kingIndex];
        bbBlockers |= s.allPieces() & occupancyMaskRook[kingIndex];
        // printf("ROOK BB BLOCKERS:\n%s", bbToString(bbBlockers).c_str());
        int databaseIndex =
            (int)(((bbBlockers & ~border) * magicNumberRook[kingIndex]) >>
                  magicNumberShiftsRook[kingIndex]);
        // printf("IN BETWEEN
        // SQUARES:\n%s",bbToString(magicMovesRook[kingIndex][databaseIndex] &
        // ~allPieces).c_str());
        getSetBits(magicMovesRook[kingIndex][databaseIndex] & ~allPieces &
                       ~bbBlockers,
                   pieceSquares);
      }
    }

    // if its a diagonal attack path
    if (kingAttacker == wB || kingAttacker == bB || kingAttacker == wQ ||
        kingAttacker == bQ) {
      U64 bbBlockers = 0;
      // if attacking piece is on top
      if (indexDelta < 0) {
        // on the top left
        if (indexDelta % 7 == 0 && -indexDelta / 7 < 8 &&
            kingIndex / 8 != kingAttackerIndex / 8) {
          bbBlockers = bbBlockers8Way[kingIndex][4];
        }
        // on the top right
        else if (indexDelta % 9 == 0) {
          bbBlockers = bbBlockers8Way[kingIndex][5];
        }

      } else if (indexDelta > 0) {
        // on the bottom left
        if (indexDelta % 9 == 0) {
          bbBlockers = bbBlockers8Way[kingIndex][6];
        }
        // on the bottom right
        else if (indexDelta % 7 == 0 && indexDelta / 7 < 8 &&
                 kingIndex / 8 != kingAttackerIndex / 8) {
          bbBlockers = bbBlockers8Way[kingIndex][7];
        }
      }
      if (bbBlockers) {
        bbBlockers &= ~setMask[kingIndex];
        bbBlockers |= s.allPieces() & occupancyMaskBishop[kingIndex];
        // printf("BISHOP BB BLOCKERS:\n%s", bbToString(bbBlockers).c_str());
        int databaseIndex =
            (int)(((bbBlockers & ~border) * magicNumberBishop[kingIndex]) >>
                  magicNumberShiftsBishop[kingIndex]);
        //        printf(
        //            "IN BETWEEN SQUARES:\n%s",
        //            bbToString(magicMovesBishop[kingIndex][databaseIndex] &
        //            ~allPieces)
        //                .c_str());
        getSetBits(magicMovesBishop[kingIndex][databaseIndex] & ~allPieces &
                       ~bbBlockers,
                   pieceSquares);
      }
    }
    // for every square between king and attacker
    for (int to = 0; pieceSquares[to] != -1; to++) {
      // generate pieces that can move to that square (to block)
      getSetBits(movesTo(pieceSquares[to], s, s._sideToMove, allPieces),
                 pieceMoves);
      for (int from = 0; pieceMoves[from] != -1; from++) {
        if (pieceMoves[from] == kingIndex) {
          continue;
        }
        Piece movingPiece = s._pieces[pieceMoves[from]];
        if (movingPiece == wP || movingPiece == bP) {
          // handle pawn promotion
          if (pieceSquares[to] / 8 < 1 || pieceSquares[to] / 8 >= 7) {
            Move move1 = NEW_MOVE(pieceMoves[from], pieceSquares[to]);
            Move move2 = NEW_MOVE(pieceMoves[from], pieceSquares[to]);
            Move move3 = NEW_MOVE(pieceMoves[from], pieceSquares[to]);
            Move move4 = NEW_MOVE(pieceMoves[from], pieceSquares[to]);
            M_SETPROM(move1, bQ);
            M_SETPROM(move2, bN);
            M_SETPROM(move3, bR);
            M_SETPROM(move4, bB);
            moves.push_back(move1);
            moves.push_back(move2);
            moves.push_back(move3);
            moves.push_back(move4);
          } else {
            moves.push_back(NEW_MOVE(pieceMoves[from], pieceSquares[to]));
          }
        } else {
          moves.push_back(NEW_MOVE(pieceMoves[from], pieceSquares[to]));
        }
      }
    }

    // printMoves("block path", moves);
  }

  return moves;
}

std::vector<Move> generatePseudoMoves(const State &s, bool inCheck) {
  if (inCheck) {
    return generateCheckEvasions(s);
  } else {
    return generateAllMoves(s);
  }
}

std::vector<Move> generatePseudoMoves(const State &s) {
  return generatePseudoMoves(s, s.isInCheck(s._sideToMove));
}

U64 rookAttacks(int index, U64 consideredPieces) {
  U64 bbBlockers = consideredPieces & occupancyMaskRook[index];
  int databaseIndex = (int)((bbBlockers * magicNumberRook[index]) >>
                            magicNumberShiftsRook[index]);
  return magicMovesRook[index][databaseIndex];
}

U64 bishopAttacks(int index, U64 consideredPieces) {
  U64 bbBlockers = consideredPieces & occupancyMaskBishop[index];
  int databaseIndex = (int)((bbBlockers * magicNumberBishop[index]) >>
                            magicNumberShiftsBishop[index]);
  return magicMovesBishop[index][databaseIndex];
}

U64 pawnPush(U64 bb, int pawnSide, const State &s, bool moveTo) {
  U64 moves = 0ULL;
  U64 rank4 = pawnSide == WHITE ? RANK_BB[3] | (moveTo ? RANK_BB[6] : 0)
                                : RANK_BB[4] | (moveTo ? RANK_BB[1] : 0);
  U64 pawns = (pawnSide == WHITE ? UP(bb) : DOWN(bb));
  moves |= pawns;
  pawns &= ~s.allPieces();
  pawns = (pawnSide == WHITE ? UP(pawns) : DOWN(pawns));
  moves |= pawns &
           rank4; // only add double push moves if the destination is on rank 4
  return moves;
}

U64 pawnAttack(int index, int pawnSide, const State &s) {
  U64 EPTarget = 0ULL;
  U64 attack = 0ULL;
  U64 enemy = 0ULL;
  if (s._EPTarget != -1) {
    EPTarget = setMask[s._EPTarget];
  }
  if (pawnSide == WHITE) {
    enemy = s._pieceBitboards[BLACKS];
    attack = pawnAttacks[0][index];
    EPTarget = UP(EPTarget);
  } else {
    enemy = s._pieceBitboards[WHITES];
    attack = pawnAttacks[1][index];
    EPTarget = DOWN(EPTarget);
  }
  enemy |= EPTarget;

  return attack & enemy;
}

U64 attacksTo(int index, const State &s, int defendingSide,
              U64 consideredPieces) {
  if (consideredPieces == 0) {
    consideredPieces = s.allPieces();
  }
  U64 enemyBB = defendingSide == BLACK ? s._pieceBitboards[WHITES]
                                       : s._pieceBitboards[BLACKS];
  enemyBB &= consideredPieces;
  U64 knights = knightAttacks[index] & s._pieceBitboards[KNIGHTS];
  U64 kings = kingAttacks[index] & s._pieceBitboards[KINGS];
  U64 bishopsQueens = bishopAttacks(index, consideredPieces) &
                      (s._pieceBitboards[BISHOPS] | s._pieceBitboards[QUEENS]);
  U64 rooksQueens = rookAttacks(index, consideredPieces) &
                    (s._pieceBitboards[ROOKS] | s._pieceBitboards[QUEENS]);
  U64 pawns = pawnAttack(index, defendingSide, s) & s._pieceBitboards[PAWNS];

  return (knights | kings | bishopsQueens | rooksQueens | pawns) & enemyBB;
}

U64 movesTo(int index, const State &s, int movingSide, U64 consideredPieces) {
  if (consideredPieces == 0) {
    consideredPieces = s.allPieces();
  }
  U64 friendlyBB = movingSide == WHITE ? s._pieceBitboards[WHITES]
                                       : s._pieceBitboards[BLACKS];
  friendlyBB &= consideredPieces;
  U64 knights = knightAttacks[index] & s._pieceBitboards[KNIGHTS];
  U64 kings = kingAttacks[index] & s._pieceBitboards[KINGS];
  U64 bishopsQueens = bishopAttacks(index, consideredPieces) &
                      (s._pieceBitboards[BISHOPS] | s._pieceBitboards[QUEENS]);
  U64 rooksQueens = rookAttacks(index, consideredPieces) &
                    (s._pieceBitboards[ROOKS] | s._pieceBitboards[QUEENS]);
  U64 pawns =
      pawnPush(setMask[index], -movingSide, s, true) & s._pieceBitboards[PAWNS];

  //  printf("PAWNS MOVES TO %i:\n%s", index,
  //         bbToString(pawnPush(setMask[index], -movingSide, s,
  //         true)).c_str());

  return (knights | kings | bishopsQueens | rooksQueens | pawns) & friendlyBB;
}

void generateOccupancyVariations(bool isRook) {
  int i, j, bitRef;
  long mask;
  int variationCount;
  int *setBitsInMask, *setBitsInIndex;
  int bitCount[64];

  for (bitRef = 0; bitRef <= 63; bitRef++) {
    mask = isRook ? occupancyMaskRook[bitRef] : occupancyMaskBishop[bitRef];
    setBitsInMask = getSetBits(mask);
    bitCount[bitRef] = countSetBits(mask);
    variationCount = (int)(1ULL << bitCount[bitRef]);
    for (i = 0; i < variationCount; i++) {
      occupancyVariation[bitRef][i] = 0;

      // find bits set in index "i" and map them to bits in the 64 bit
      // "occupancyVariation"

      setBitsInIndex =
          getSetBits(i); // an array of integers showing which bits are set
      for (j = 0; setBitsInIndex[j] != -1; j++) {
        occupancyVariation[bitRef][i] |=
            (1ULL << setBitsInMask[setBitsInIndex[j]]);
      }
      delete setBitsInIndex;
    }
    delete setBitsInMask;
  }
}

void generateMoveDatabase(bool isRook) {
  long validMoves;
  int variations, bitCount;
  int bitRef, i, j, magicIndex;

  for (bitRef = 0; bitRef <= 63; bitRef++) {
    bitCount = isRook ? countSetBits(occupancyMaskRook[bitRef])
                      : countSetBits(occupancyMaskBishop[bitRef]);
    variations = (int)(1ULL << bitCount);

    for (i = 0; i < variations; i++) {
      validMoves = 0;
      if (isRook) {
        magicIndex =
            (int)((occupancyVariation[bitRef][i] * magicNumberRook[bitRef]) >>
                  magicNumberShiftsRook[bitRef]);

        for (j = bitRef + 8; j <= 63; j += 8) {
          validMoves |= (1ULL << j);
          if ((occupancyVariation[bitRef][i] & (1ULL << j)) != 0)
            break;
        }
        for (j = bitRef - 8; j >= 0; j -= 8) {
          validMoves |= (1ULL << j);
          if ((occupancyVariation[bitRef][i] & (1ULL << j)) != 0)
            break;
        }
        for (j = bitRef + 1; j % 8 != 0; j++) {
          validMoves |= (1ULL << j);
          if ((occupancyVariation[bitRef][i] & (1ULL << j)) != 0)
            break;
        }
        for (j = bitRef - 1; j % 8 != 7 && j >= 0; j--) {
          validMoves |= (1ULL << j);
          if ((occupancyVariation[bitRef][i] & (1ULL << j)) != 0)
            break;
        }

        magicMovesRook[bitRef][magicIndex] = validMoves;
      } else {
        magicIndex =
            (int)((occupancyVariation[bitRef][i] * magicNumberBishop[bitRef]) >>
                  magicNumberShiftsBishop[bitRef]);

        for (j = bitRef + 9; j % 8 != 0 && j <= 63; j += 9) {
          validMoves |= (1ULL << j);
          if ((occupancyVariation[bitRef][i] & (1ULL << j)) != 0)
            break;
        }
        for (j = bitRef - 9; j % 8 != 7 && j >= 0; j -= 9) {
          validMoves |= (1ULL << j);
          if ((occupancyVariation[bitRef][i] & (1ULL << j)) != 0)
            break;
        }
        for (j = bitRef + 7; j % 8 != 7 && j <= 63; j += 7) {
          validMoves |= (1ULL << j);
          if ((occupancyVariation[bitRef][i] & (1ULL << j)) != 0)
            break;
        }
        for (j = bitRef - 7; j % 8 != 0 && j >= 0; j -= 7) {
          validMoves |= (1ULL << j);
          if ((occupancyVariation[bitRef][i] & (1ULL << j)) != 0)
            break;
        }

        magicMovesBishop[bitRef][magicIndex] = validMoves;
      }
    }
  }
}

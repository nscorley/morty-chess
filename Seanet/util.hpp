//
//  util.hpp
//  Seanet
//
//  Created by Stiven Deleur on 5/12/16.
//
//

#ifndef util_hpp
#define util_hpp

#include "board.hpp"
#include "defs.h"
#include "evaluator.hpp"
#include "hash.hpp"
#include "util.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <time.h>
#include <vector>

std::vector<std::string> &split(const std::string &s, char delim,
                                std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

std::string moveToSAN(Move move, State state);
Move moveFromSAN(std::string SAN, State &state);
char *getDtTm(char *buff);
KeyInfoMap splitEDP(std::string EDP);
char pieceToChar(Piece p);
Piece charToPiece(char p);
int bitboardForPiece(Piece p);
int uciToIndex(std::string uci);
std::string indexToUCI(int index);
std::string boardToFEN(const State &b);
State boardFromFEN(std::string FEN);
void initpopCountOfByte256();
void initPresets();
void getSetBits(U64 bb, int *setBits);
int *getSetBits(U64 bb);
std::string bbToString(U64 bb);
std::string moveToUCI(int move);
int moveFromUCI(std::string uci);
std::string pvLineToString(S_PVLINE line);
std::string moveLineToString(std::vector<Move> line);
int see(Move move, const State &s);
std::string searchFeaturesToString(bool *features);
std::string historyToString(const State &state);
std::vector<std::string> exportGamesFromPGN(std::ifstream file);
std::vector<Move> getGameMoveLine(std::string game);
int getPGNGameWinner(std::string game);

// inline functions

inline int LS1B(U64 bb) {
  return bb ? index64[((bb ^ (bb - 1)) * 0x03f79d71b4cb0a89ULL) >> 58] : -1;
}

inline int countSetBits(U64 bb) {
  return popCountOfByte256[bb & 0xff] + popCountOfByte256[(bb >> 8) & 0xff] +
         popCountOfByte256[(bb >> 16) & 0xff] +
         popCountOfByte256[(bb >> 24) & 0xff] +
         popCountOfByte256[(bb >> 32) & 0xff] +
         popCountOfByte256[(bb >> 40) & 0xff] +
         popCountOfByte256[(bb >> 48) & 0xff] + popCountOfByte256[bb >> 56];
}

inline int sideBitboardForPiece(Piece p) {
  return p % 2 == 0 ? BLACKS : WHITES;
}

inline int sideOfPiece(Piece p) { return p % 2 == 0 ? BLACK : WHITE; }

inline long int timeToMS(timeval &t) {
  return t.tv_sec * 1000 + (t.tv_usec / 1000);
}

#endif /* util_hpp */

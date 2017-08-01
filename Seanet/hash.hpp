//
//  hash.hpp
//  Seanet
//
//  Created by Douglas Corley on 5/23/16.
//
//

#ifndef hash_hpp
#define hash_hpp

#include "board.hpp"
#include "defs.h"
#include "util.hpp"
#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include <unordered_map>

enum NodeType { UNSET, EXACT, ALPHA, BETA };

const int TABLE_SIZE = 5 * pow(10, 5);

extern U64
    zArray[64][12]; // Squares + piece types (2 colors * 6 pieces * 64 squares)
extern U64 zSide;
extern U64 zEP[8];
extern U64 zCastle[4]; // Castling

void initZobrists();
U64 getZobristHash(const State &s);
U64 rand64();
void updateHash(U64 &zKey, State &s, Move m);

typedef struct {
  U64 zobrist;
  int depth;
  int score;
  Move move;
  NodeType type;

} S_HASHENTRY;

bool operator!=(const S_HASHENTRY &lhs, const S_HASHENTRY &rhs);

typedef std::unordered_map<int, S_HASHENTRY> HashTable;

void initHashTable(HashTable *table);
void clearHashTable(HashTable *table);

void storeHashEntry(U64 zobrist, int depth, int score, Move move, NodeType type,
                    HashTable &table);
S_HASHENTRY probeHashTable(HashTable &table, U64 zobrist);

const S_HASHENTRY NULL_ENTRY = {0, 0, 0, 0, UNSET};

#endif /* hash_hpp */

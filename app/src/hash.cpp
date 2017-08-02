//
//  hash.cpp
//  Seanet
//
//  Created by Douglas Corley on 5/23/16.
//
//

#include "hash.hpp"

U64 zArray[64][12]; // Squares + piece types (2 colors * 6 pieces * 64
                    // squares)
U64 zSide;
U64 zEP[8];
U64 zCastle[4]; // Castling

bool operator!=(const S_HASHENTRY &lhs, const S_HASHENTRY &rhs) {
  return lhs.zobrist != rhs.zobrist || lhs.score != rhs.score ||
         lhs.depth != rhs.depth || lhs.move != rhs.move || lhs.type != rhs.type;
}

void initHashTable(HashTable *table) { table->rehash(TABLE_SIZE); }

void clearHashTable(HashTable *table) {
  for (auto itr : *table) {
    table->erase(itr.first);
  }
}

void storeHashEntry(U64 zobrist, int depth, int score, Move move, NodeType type,
                    HashTable &table) {
  int index = zobrist % TABLE_SIZE;
  assert(index >= 0 && index < TABLE_SIZE);
  S_HASHENTRY entry{zobrist, depth, score, move, type};
  if (table.find(index) == table.end()) {
    table.emplace(index, entry);
  } else if (table[index].depth <= depth) {
    table[index] = entry;
  }
}

S_HASHENTRY probeHashTable(HashTable &table, U64 zobrist) {
  int index = zobrist % TABLE_SIZE;
  assert(index >= 0 && index < TABLE_SIZE);
  if (table.find(index) != table.end()) {
    auto a = table[index];
    return a;
  } else {
    return NULL_ENTRY;
  }
}

void initZobrists() {
  timeval currTime;
  gettimeofday(&currTime, 0);
  int now = (int)(timeToMS(currTime));
  srand(now);

  for (int i = 0; i < 64; i++) {
    for (int j = 0; j < 12; j++) {
      zArray[i][j] = rand64();
    }
  }
  for (int i = 0; i < 8; i++)
    zEP[i] = rand64();
  zSide = rand64();
  for (int i = 0; i < 4; i++) {
    zCastle[i] = rand64();
  }
}

U64 rand64() {
  return rand() ^ ((U64)rand() << 15) ^ ((U64)rand() << 30) ^
         ((U64)rand() << 45) ^ ((U64)rand() << 60);
}

void updateHash(U64 &zKey, State &s, Move m) {
  int from = M_FROMSQ(m);
  int to = M_TOSQ(m);
  int pieceIndex = (int)(s._pieces[from]) - 1;
  int captureIndex = (int)(s._pieces[to]) - 1;
  zKey ^= zArray[from][pieceIndex];
  if (captureIndex > -1) {
    zKey ^= zArray[to][captureIndex];
  }
  zKey ^= zArray[to][pieceIndex];
  /* Function not complete */
}

U64 getZobristHash(const State &s) {
  U64 zKey = 0;
  for (int i = 0; i < 64; i++) {
    switch (s._pieces[i]) {
    case EMPTY:
      break;
    case wP:
      zKey ^= zArray[i][0];
      break;
    case bP:
      zKey ^= zArray[i][1];
      break;
    case wN:
      zKey ^= zArray[i][2];
      break;
    case bN:
      zKey ^= zArray[i][3];
      break;
    case wB:
      zKey ^= zArray[i][4];
      break;
    case bB:
      zKey ^= zArray[i][5];
      break;
    case wR:
      zKey ^= zArray[i][6];
      break;
    case bR:
      zKey ^= zArray[i][7];
      break;
    case wQ:
      zKey ^= zArray[i][8];
      break;
    case bQ:
      zKey ^= zArray[i][9];
      break;
    case wK:
      zKey ^= zArray[i][10];
      break;
    case bK:
      zKey ^= zArray[i][11];
      break;
    default:
      std::cout
          << "Unknown piece uncovered in getZobristHash() function. Exiting...";
      exit(0);
    }
  }
  if (s._sideToMove == WHITE) {
    zKey ^= zSide;
  }
  if ((s._castleRights & WKCA) != 0) {
    zKey ^= zCastle[0];
  }
  if ((s._castleRights & WQCA) != 0) {
    zKey ^= zCastle[1];
  }
  if ((s._castleRights & BKCA) != 0) {
    zKey ^= zCastle[2];
  }
  if ((s._castleRights & BQCA) != 0) {
    zKey ^= zCastle[3];
  }
  if (s._EPTarget != -1) {
    zKey ^= zEP[s._EPTarget % 8];
  }
  return zKey;
}

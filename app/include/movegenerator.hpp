/**
 * Generates possible moves given the current state
 **/

#ifndef MOVE_GENERATOR_HPP_INCLUDE
#define MOVE_GENERATOR_HPP_INCLUDE

#include "defs.h"
#include "state.hpp"
#include <stdio.h>
#include <vector>

std::vector<Move> generatePseudoMoves(const State &s);
std::vector<Move> generatePseudoMoves(const State &s, bool inCheck);
void generateMoveDatabase(bool isRook);
void generateOccupancyVariations(bool isRook);
U64 attacksTo(int index, const State &s, int defendingSide,
              U64 consideredPieces = 0);
U64 movesTo(int index, const State &s, int movingSide,
            U64 consideredPieces = 0);

#endif /* movegenerator_hpp */

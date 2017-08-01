//
//  evaluator.hpp
//  Seanet
//
//  Created by Nathaniel Corley on 5/16/16.
//
//

#ifndef __Seanet__evaluator__
#define __Seanet__evaluator__

#include "board.hpp"
#include "util.hpp"
#include <stdio.h>
#include <vector>

int calculatePhase(State &state);
int calculateMaterial(State &state);
int evaluate(State &state);
int evaluateGameOver(State &state, int contempt);
bool isGameOver(State &state);
bool isGameOver(State &state, std::vector<Move> moves);
bool isThreeFoldRepetition(State &state);
int evaluateDraw(State &state, int contempt);

bool isIsolatedPawn(State &state, int index);
bool isIsolaniPawn(State &state, int index);
bool isDoubledPawn(State &state, int index);
bool isConnectedPawn(State &state, int index);
bool isBackwardPawn(State &state, int index);
bool isBlockedPawn(State &state, int index);
bool isPassedPawn(State &state, int index);

int countHoles(State &state);
int countKingPawnShields(State &state);

FList getInitialFeatures(State &state);
float getPhaseFactor(State &state);

#endif /* defined(__Seanet__evaluator__) */

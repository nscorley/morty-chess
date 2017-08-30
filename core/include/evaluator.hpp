/**
 * Defines functions for evaluating state based on
 * features and piece-square tables.
 **/

#ifndef EVALUATOR_HPP_INCLUDE
#define EVALUATOR_HPP_INCLUDE

#include "state.hpp"
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

#endif

/**
 * The search controller holds methods and member variables
 * that dictate when the search stops,
 * how long it takes, how many nodes have been searched, etc.
 **/

#ifndef SEARCH_CONTROLLER_HPP_INCLUDE
#define SEARCH_CONTROLLER_HPP_INCLUDE

#include "hash.hpp"
#include "state.hpp"
#include "util.hpp"
#include <stdio.h>
#include <sys/time.h>

class SearchController {
public:
  int _depthLimit = MAX_DEPTH;

  int _uciOutput = false;
  bool _output = true;

  // {PV_REORDERING, SEE_REORDERING, KH_REORDERING, HH_REORDERING, NULL_MOVE,
  // TT_EVAL, TT_REORDERING, PV_SEARCH, ASPIRATION_WINDOWS, QS_REORDERING}
  bool _features[NUM_OF_FEATURES] = {true, true, true, true, true,
                                     true, true, true, true, true};
  // UCI timing
  int _moveTime;  // i.e. seconds.move
  int _wTime;     // time (for game) white has left
  int _bTime;     // time (for game)black has left
  int _wInc;      // white increment/move
  int _bInc;      // black increment/move
  int _moveToGo;  // number of moves before next time control
  int _nodeLimit; // limit to how many nodes searched

  // which side the engine is playing
  int _analysisSide = WHITE;

  // game timing computations
  double _dampeningFactor = 0.03; // factor to lower time after middle game
  int _upperMoveBound = 100; // expected number of moves per game (upper bound)

  // because nobody likes a draw
  int _contemptFactor = -50;

  bool _stopSearch = false;
  timeval _startTime;

  // search statistics
  int _totalNodes = 0;
  int _qNodes = 0;
  int _mainNodes = 0;
  int _fhNodes = 0;
  int _fhfNodes = 0;
  int _currDepth = 0;
  int _transpositions = 0;
  int _exactNodes = 0;
  int _alphaNodes = 0;
  int _betaNodes = 0;
  int _maxDepth = 0;

  // move initializations
  Move _currMove = 0;
  int _currMoveNumber = 0;

  // what is this for? TT?
  HashTable table;

  void getAllottedTime(int totalMoves);
  void checkTimeLimit();
  void resetStats();
  std::string featuresToString();
};

#endif

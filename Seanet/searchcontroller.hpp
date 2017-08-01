//
//  searchcontroller.hpp
//  Seanet
//
//  Created by Nathaniel Corley on 5/17/16.
//
//

#ifndef __Seanet__searchcontroller__
#define __Seanet__searchcontroller__

#include "board.hpp"
#include "hash.hpp"
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
  // Timing
  int _timeLimit = 10; // i.e. seconds.move
  int _wTime;          // Amount of time white has left
  int _bTime;          // Amount of time black has left
  int _wInc = 0;       // White increment/move
  int _bInc = 0;       // Black increment/move
  int _moveToGo;       // Number of moves before next time control
  int _nodeLimit;      // Limit to how many nodes searched
  int _analysisSide = WHITE;
  double _dampeningFactor = 0.03;
  int _upperMoveBound = 100;

  int _contemptFactor = -50;

  bool _stopSearch = false;
  timeval _startTime;
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
  Move _currMove = 0;
  int _currMoveNumber = 0;
  HashTable table;

  void getAllottedTime(int totalMoves);
  void checkTimeLimit();
  void resetStats();
  std::string featuresToString();
};

#endif /* defined(__Seanet__searchcontroller__) */

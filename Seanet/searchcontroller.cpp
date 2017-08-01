//
//  searchcontroller.cpp
//  Seanet
//
//  Created by Nathaniel Corley on 5/17/16.
//
//

#include "searchcontroller.hpp"

void SearchController::checkTimeLimit() {
  /*if ((time(NULL) - _startTime.tv_sec) >= _timeLimit) {
    _stopSearch = true;
  }*/
  timeval currTime;
  gettimeofday(&currTime, 0);
  if ((timeToMS(currTime) - timeToMS(_startTime) >= _timeLimit)) {
    _stopSearch = true;
  }
}

void SearchController::resetStats() {
  gettimeofday(&_startTime, 0);
  _totalNodes = 0;
  _fhNodes = 1;
  _fhfNodes = 1;
  _currDepth = 0;
  _maxDepth = 0;
  _currMove = 0;
  _currMoveNumber = 0;
  _stopSearch = false;
}

std::string SearchController::featuresToString() {
  return searchFeaturesToString(_features);
}

void SearchController::getAllottedTime(int totalMoves) {

  if (_wTime <= 0) {
    return;
  }
  int time = _analysisSide == WHITE ? _wTime : _bTime;

  double scalar =
      (_dampeningFactor * time *
       pow(e, (_dampeningFactor * (_upperMoveBound + totalMoves)))) /
      (pow(e, (_upperMoveBound * _dampeningFactor)) -
       pow(e, (_dampeningFactor * totalMoves)));

  int result = (int)(scalar * pow(e, (-_dampeningFactor * totalMoves)));
  _timeLimit = result;
}
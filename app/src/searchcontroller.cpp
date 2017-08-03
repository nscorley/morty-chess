/**
 * Implementations for basic search control functions. e.g. time checking, etc.
 **/

#include "searchcontroller.hpp"
using namespace std;

void SearchController::checkTimeLimit() {
  timeval currTime;
  gettimeofday(&currTime, 0);
  if ((timeToMS(currTime) - timeToMS(_startTime) >= _moveTime)) {
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

string SearchController::featuresToString() {
  return searchFeaturesToString(_features);
}

/**
 * Fetch or calculate alloted time for move
 **/
void SearchController::getAllottedTime(int totalMoves) {

  // check if time per move set
  if (_moveTime > 0) {
    return;
  }

  // TODO: IDK what this is about? Why just white?
  if (_wTime <= 0) {
    return;
  }
  int time = _analysisSide == WHITE ? _wTime : _bTime;

  // integrate to find alloted time
  double scalar =
      (_dampeningFactor * time *
       pow(e, (_dampeningFactor * (_upperMoveBound + totalMoves)))) /
      (pow(e, (_upperMoveBound * _dampeningFactor)) -
       pow(e, (_dampeningFactor * totalMoves)));

  int result = (int)(scalar * pow(e, (-_dampeningFactor * totalMoves)));
  _moveTime = result;
}

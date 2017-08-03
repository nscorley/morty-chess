/**
 * The UCI object can be used to run all relevant UCI commands
 * Instantiated in main.cpp
 **/

#ifndef UCI_HPP_INCLUDE
#define UCI_HPP_INCLUDE

// external dependencies
#include <algorithm>
#include <ctime>
#include <iostream>
#include <thread>
#include <vector>

// internal dependencies
#include "evaluator.hpp"
#include "hash.hpp"
#include "movegenerator.hpp"
#include "search.hpp"
#include "searchcontroller.hpp"
#include "state.hpp"
#include "util.hpp"

void uciGo(std::vector<std::string> inputParts,
           SearchController uciSearchControl);
void uciSetPosition(std::vector<std::string> inputParts, std::string input,
                    State &uciGameState);

class UCI {
private:
  State _gameState;
  SearchController _sControl;

public:
  void startUCI();
  void takeUCIInput();
  void takeAction();
};

#endif

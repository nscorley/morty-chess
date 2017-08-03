/**
 * Main entrypoint
 **/

// external dependencies
#include <algorithm>
#include <ctime>
#include <iostream>
#include <thread>
#include <vector>
using namespace std;

// internal dependencies
#include "evaluator.hpp"
#include "hash.hpp"
#include "movegenerator.hpp"
#include "search.hpp"
#include "searchcontroller.hpp"
#include "state.hpp"
#include "uci.hpp"
#include "util.hpp"

void takeAction();
void welcomeScreen();
State gameState;
SearchController sControl;

int main(int argc, const char *argv[]) {
  initPresets();
  welcomeScreen();
  return 0;
}

// determines the mode for the engine to run in
void welcomeScreen() {
  std::cout << "Welcome to MortyChess. Developed by Nathaniel Corley and "
               "Stiven Deleur."
            << std::endl;
  std::cout << "Please enter engine mode (uci or test)" << std::endl;
  std::string mode;
  std::getline(std::cin, mode);
  if (mode == "uci") {
    UCI uci;
    uci.startUCI();
    uci.takeUCIInput();
  } else if (mode == "test") {
    std::cout << "Entering test mode..." << std::endl;
    std::string FEN = "startpos";
    FEN = "rn1q1rk1/p4ppp/1ppb4/3p4/2PP2b1/2PB1N2/P4PPP/1RBQ1RK1 w - -";
    gameState = boardFromFEN(FEN);
    while (true) {
      takeAction();
    }
  } else {
    std::cout << "Sorry, please enter \"uci\" or \"test\"" << std::endl;
  }
}

// take move for testing, not used in UCI communication
void takeAction() {
  gameState.printBoard();

  // display current information
  std::cout << "FEN: " << boardToFEN(gameState) << "; Hash:" << gameState._zHash
            << std::endl;
  std::cout << "Static board evaluation: " << evaluate(gameState) << std::endl;

  // get legal moves
  std::vector<int> pseudoMoves = generatePseudoMoves(gameState);
  std::vector<int> legalMoves;
  for (auto it = pseudoMoves.begin(); it != pseudoMoves.end(); ++it) {
    if (gameState.isLegalMove(*it)) {
      legalMoves.push_back(*it);
    }
  }

  // take user move
  std::string userMove;
  std::cout << "Enter the next action:" << std::endl;
  std::getline(std::cin, userMove);

  // execute action
  if (userMove == "undo") {
    gameState.takeMove();
  } else if (userMove == "search") {

    // sControl._depthLimit = 10;
    sControl._uciOutput = true;
    sControl._moveTime = 1000;
    startSearch(gameState, sControl);
    std::cout << "\nEvaluation:\n["
              << gameState._lineEval * (gameState._sideToMove == WHITE ? 1 : -1)
              << "] " << pvLineToString(gameState._bestLine) << "\n"
              << std::endl;
  } else {
    int move = moveFromUCI(userMove);
    if (std::find(legalMoves.begin(), legalMoves.end(), move) !=
        legalMoves.end()) {
      gameState.makeMove(move);
    } else {
      std::cout << "Illegal Move!\n";
    }
  }
}

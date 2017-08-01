//
//  main.cpp
//  Seanet
//
//  Created by Stiven Deleur on 5/12/16.
//
//

#include "board.hpp"
#include "evaluator.hpp"
#include "hash.hpp"
#include "movegenerator.hpp"
#include "search.hpp"
#include "searchcontroller.hpp"
#include "util.hpp"
#include <algorithm>
#include <ctime>
#include <iostream>
#include <thread>
#include <vector>

void startUCI();
void takeUCIInput();

void takeAction();
State gameState;
SearchController sControl;

int main(int argc, const char *argv[]) {
  // insert code here...

  std::string FEN = "startpos";
  // FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0
  // 1";
  // FEN = "8/8/4K3/4P3/8/4k3/8/8 w - - 0 1";
  // mirrored positions
  //  FEN = "K7/8/8/3Q4/4q3/8/8/7k b - - 0 1";
  //   FEN = "1rb1k2q/1p2r1bp/3n1p2/n1p4P/p4P1N/2P1N3/PB1R2P1/Q2K1BR1 b - - 0
  //   1";
  //  FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBKQBNR w - - 0 1";
  // FEN = "2r1k2r/2pn1pp1/1p3n1p/p3PP2/4q2B/P1P5/2Q1N1PP/R4RK1 w q -";
  // FEN = "4k2r/3n1pp1/1pr2n1p/p1p1PP2/4QR1B/P1P5/4N1PP/R5K1 w q - 2 2";
  // FEN = "6K1/8/7P/8/8/3k4/1r6/8 b - - 3 78";
  // FEN = "8/1Q6/8/8/5K2/8/2R5/6k1 b - - 7 77";
  // FEN = "6R1/5Qbk/3p4/1P1Pp2P/3pP1p1/3P1K1P/1Pq2P2/5B2 w - - 0 65";
  // FEN = "6k1/ppp2ppp/2b5/2P3P1/1q3P1P/6Q1/PP2r3/3R2K1 b - - 2 34";
  // FEN = "r3k3/2p2p1P/p1n4b/P3Nb1p/1p4P1/5n1P/1P4R1/R1BK1B2 b q -";
  initPresets();

  FEN = "rn1q1rk1/p4ppp/1ppb4/3p4/2PP2b1/2PB1N2/P4PPP/1RBQ1RK1 w - -";

  gameState = boardFromFEN(FEN);

  Move m = moveFromSAN("cxd5", gameState);
  std::cout << moveToUCI(m) << std::endl;
  exit(0);

  while (true) {
    takeAction();
  }
}

void takeAction() {
  gameState.printBoard();
  std::cout << "FEN: " << boardToFEN(gameState) << "; Hash:" << gameState._zHash
            << std::endl;
  std::cout << "Static board evaluation: " << evaluate(gameState) << std::endl;

  std::vector<int> pseudoMoves = generatePseudoMoves(gameState);
  std::vector<int> legalMoves;

  for (auto it = pseudoMoves.begin(); it != pseudoMoves.end(); ++it) {
    if (gameState.isLegalMove(*it)) {
      legalMoves.push_back(*it);
    }
  }

  std::string userMove;
  std::cout << "Enter the next action:" << std::endl;
  std::getline(std::cin, userMove);
  if (userMove == "undo") {
    gameState.takeMove();
  } else if (userMove == "uci") {
    startUCI();
  } else if (userMove == "search") {

    // sControl._depthLimit = 10;
    sControl._uciOutput = true;
    sControl._timeLimit = 1000;
    search(gameState, sControl);
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

void startUCI() {
  std::cout << "id name Seanet 0.1b\n";
  std::cout << "id author Stiven Deleur, Nathaniel Corley\n";

  std::cout << "uciok\n" << std::endl;
  takeUCIInput();
}

void takeUCIInput() {
  std::string input;

  SearchController uciStateControl;
  State uciGameState;
  std::thread searchThread;

  bool searching = false;

  uciStateControl._uciOutput = true;
  while (std::getline(std::cin, input)) {

    if (uciStateControl._stopSearch == true) {
      if (searchThread.joinable()) {
        searchThread.join();
      }
    }
    std::vector<std::string> inputParts;
    inputParts = split(input, ' ');
    std::string commandName = inputParts.at(0);
    if (commandName == "isready") {
      uciGameState = boardFromFEN("startpos");
      std::cout << "readyok\n";
    } else if (commandName == "ucinewgame") {
      uciGameState = boardFromFEN("startpos");
    } else if (commandName == "position") {
      std::string FEN = inputParts.at(1);
      if (FEN == "startpos") {
        uciGameState = boardFromFEN("startpos");
      } else {
        FEN = input.substr(13);
        uciGameState = boardFromFEN(FEN);
      }
      int i;

      for (i = 2; i < inputParts.size(); i++) {
        if (inputParts.at(i) == "moves") {
          i++;
          break;
        }
      }
      for (; i < inputParts.size(); i++) {
        Move move = moveFromUCI(inputParts.at(i));
        uciGameState.makeMove(move);
      }
    } else if (commandName == "go") {

      for (int i = 1; i < inputParts.size(); i++) {
        if (inputParts.at(i) == "searchmoves") {
        } else if (inputParts.at(i) == "ponder") {
          uciStateControl._timeLimit = INT_MAX;
          uciStateControl._depthLimit = INT_MAX;
        } else if (inputParts.at(i) == "wtime") {
          uciStateControl._wTime = std::stoi(inputParts.at(i + 1));

        } else if (inputParts.at(i) == "btime") {
          uciStateControl._bTime = std::stoi(inputParts.at(i + 1));

        } else if (inputParts.at(i) == "winc") {
          uciStateControl._wInc = std::stoi(inputParts.at(i + 1));

        } else if (inputParts.at(i) == "binc") {
          uciStateControl._bInc = std::stoi(inputParts.at(i + 1));

        } else if (inputParts.at(i) == "movestogo") {
          uciStateControl._moveToGo = std::stoi(inputParts.at(i + 1));

        } else if (inputParts.at(i) == "depth") {
          uciStateControl._maxDepth = std::stoi(inputParts.at(i + 1));

        } else if (inputParts.at(i) == "nodes") {
          uciStateControl._nodeLimit = std::stoi(inputParts.at(i + 1));

        } else if (inputParts.at(i) == "mate") {
        } else if (inputParts.at(i) == "movetime") {

          uciStateControl._timeLimit = std::stoi(inputParts.at(i + 1)) / 1000;

        } else if (inputParts.at(i) == "infinite") {
          uciStateControl._timeLimit = INT_MAX;
          uciStateControl._depthLimit = INT_MAX;
        }
      }
      searching = true;
      uciStateControl._analysisSide = uciGameState._sideToMove;
      searchThread = std::thread(search, std::ref(uciGameState),
                                 std::ref(uciStateControl));

    } else if (commandName == "stop") {
      uciStateControl._stopSearch = true;
      if (searchThread.joinable()) {
        searchThread.join();
      }
      // stop search
    } else if (commandName == "ponderhit") {

    } else if (commandName == "quit") {
      uciStateControl._stopSearch = true;
      if (searchThread.joinable()) {
        searchThread.join();
      }
      break;
    } else if (commandName == "uci") {
      // Using UCI protocol...
    } else {
      std::cout << "Unrecognized command: " << input << "\n";
    }
  }
}
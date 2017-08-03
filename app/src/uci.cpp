/**
 * Implementations of UCI functions
 **/

#include "../include/uci.hpp"

// value-definitions for mapping UCI commands to hashable values
enum UCICommands {
  C_IS_READY,
  C_UCI_NEW_GAME,
  C_POSITION,
  C_GO,
  C_STOP,
  C_PONDER_HIT,
  C_QUIT,
  C_UCI,
};

// map to associate the strings with the enum values
static std::map<std::string, UCICommands> mapCommands;

// inits the mapCommands map
static void init() {
  mapCommands["isready"] = C_IS_READY;
  mapCommands["ucinewgame"] = C_UCI_NEW_GAME;
  mapCommands["position"] = C_POSITION;
  mapCommands["go"] = C_GO;
  mapCommands["stop"] = C_STOP;
  mapCommands["ponderhit"] = C_PONDER_HIT;
  mapCommands["quit"] = C_QUIT;
  mapCommands["uci"] = C_UCI;
}

void uciGo(std::vector<std::string> inputParts,
           SearchController uciSearchControl) {
  for (int i = 1; i < inputParts.size(); i++) {
    if (inputParts.at(i) == "searchmoves") {
    } else if (inputParts.at(i) == "ponder") {
      uciSearchControl._timeLimit = INT_MAX;
      uciSearchControl._depthLimit = INT_MAX;
    } else if (inputParts.at(i) == "wtime") {
      uciSearchControl._wTime = std::stoi(inputParts.at(i + 1));

    } else if (inputParts.at(i) == "btime") {
      uciSearchControl._bTime = std::stoi(inputParts.at(i + 1));

    } else if (inputParts.at(i) == "winc") {
      uciSearchControl._wInc = std::stoi(inputParts.at(i + 1));

    } else if (inputParts.at(i) == "binc") {
      uciSearchControl._bInc = std::stoi(inputParts.at(i + 1));

    } else if (inputParts.at(i) == "movestogo") {
      uciSearchControl._moveToGo = std::stoi(inputParts.at(i + 1));

    } else if (inputParts.at(i) == "depth") {
      uciSearchControl._maxDepth = std::stoi(inputParts.at(i + 1));

    } else if (inputParts.at(i) == "nodes") {
      uciSearchControl._nodeLimit = std::stoi(inputParts.at(i + 1));

    } else if (inputParts.at(i) == "mate") {
    } else if (inputParts.at(i) == "movetime") {

      uciSearchControl._timeLimit = std::stoi(inputParts.at(i + 1)) / 1000;

    } else if (inputParts.at(i) == "infinite") {
      uciSearchControl._timeLimit = INT_MAX;
      uciSearchControl._depthLimit = INT_MAX;
    }
  }
}

// sets the position based on passed string FEN and other parameters
static void uciSetPosition(std::vector<std::string> inputParts,
                           std::string input, State &uciGameState) {
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
}

/**
 * Prints initial welcome message and information for UCI
 **/
void UCI::startUCI() {
  // initialize command map
  init();
  // inform user
  std::cout << "id name MortyChess 0.1b\n";
  std::cout << "id author Stiven Deleur, Nathaniel Corley\n";
  std::cout << "uciok\n" << std::endl;
}

/**
 * Waits for command-line UCI input and parses appropriately
 **/
void UCI::takeUCIInput() {
  // input string from user
  std::string input;

  // initializations of state, controller, and thread
  SearchController uciSearchControl;
  State uciGameState;

  // perform search in separate thread
  std::thread searchThread;

  // whether a search is in progress
  bool searching = false;

  // whether the search should output UCI information
  uciSearchControl._uciOutput = true;

  // wait for input
  while (std::getline(std::cin, input)) {

    // join threads if search is complete
    if (uciSearchControl._stopSearch == true) {
      if (searchThread.joinable()) {
        searchThread.join();
      }
    }

    // parse input
    std::vector<std::string> inputParts;
    inputParts = split(input, ' ');
    std::string commandName = inputParts.at(0);

    // take action based on command
    switch (mapCommands[commandName]) {
    case C_IS_READY:
      uciGameState = boardFromFEN("startpos");
      std::cout << "readyok\n";
      break;

    case C_UCI_NEW_GAME:
      uciGameState = boardFromFEN("startpos");
      break;

    case C_POSITION:
      uciSetPosition(inputParts, input, uciGameState);
      break;

    case C_GO:
      uciGo(inputParts, uciSearchControl);
      searching = true;
      uciSearchControl._analysisSide = uciGameState._sideToMove;
      searchThread = std::thread(search, std::ref(uciGameState),
                                 std::ref(uciSearchControl));
      break;
    case C_STOP:
      // stop search
      uciSearchControl._stopSearch = true;
      if (searchThread.joinable()) {
        searchThread.join();
      }
      break;
    case C_PONDER_HIT:
      // not implemented
      break;
    case C_QUIT:
      // quit UCI
      uciSearchControl._stopSearch = true;
      if (searchThread.joinable()) {
        searchThread.join();
      }
      break;
    case C_UCI:
      // not implemented
      break;
    default:
      std::cout << "Unrecognized command: " << input << "\n";
      break;
    }
  }
}

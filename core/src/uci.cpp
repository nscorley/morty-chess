/**
 * Implementations of UCI functions
 **/
#include "../include/uci.hpp"
#include <string.h>
#include <limits.h>
using namespace std;

// value-definitions for mapping UCI commands to hashable values
enum UCICommands {
  C_DEFAULT,
  C_IS_READY,
  C_UCI_NEW_GAME,
  C_POSITION,
  C_GO,
  C_STOP,
  C_PONDER_HIT,
  C_QUIT,
  C_UCI,
};

// enum options for UCI "go" command to hashable values
enum goOptions {
  G_DEFAULT,
  G_SEARCHMOVES,
  G_PONDER,
  G_WTIME,
  G_BTIME,
  G_WINC,
  G_BINC,
  G_MOVESTOGO,
  G_DEPTH,
  G_NODES,
  G_MATE,
  G_MOVETIME,
  G_INFINITE,
};

// map to associate the strings with the enum values
map<string, UCICommands> mapCommands;
map<string, goOptions> mapGoOptions;

// inits the mapCommands map
void init() {
  // init UCI commands
  mapCommands["isready"] = C_IS_READY;
  mapCommands["ucinewgame"] = C_UCI_NEW_GAME;
  mapCommands["position"] = C_POSITION;
  mapCommands["go"] = C_GO;
  mapCommands["stop"] = C_STOP;
  mapCommands["ponderhit"] = C_PONDER_HIT;
  mapCommands["quit"] = C_QUIT;
  mapCommands["uci"] = C_UCI;

  // init go options
  mapGoOptions["searchmoves"] = G_SEARCHMOVES;
  mapGoOptions["ponder"] = G_PONDER;
  mapGoOptions["wtime"] = G_WTIME;
  mapGoOptions["btime"] = G_BTIME;
  mapGoOptions["winc"] = G_WINC;
  mapGoOptions["binc"] = G_BINC;
  mapGoOptions["movestogo"] = G_MOVESTOGO;
  mapGoOptions["depth"] = G_DEPTH;
  mapGoOptions["nodes"] = G_NODES;
  mapGoOptions["mate"] = G_MATE;
  mapGoOptions["movetime"] = G_MOVETIME;
  mapGoOptions["infinite"] = G_INFINITE;
}

/**
 * Prints initial welcome message and information for UCI
 **/
void UCI::startUCI(ostream &outputStr) {
  // initialize command map
  init();

  // inform user
  outputStr << "id name MortyChess 0.1b\n";
  outputStr << "id author Stiven Deleur, Nathaniel Corley\n";
  outputStr << "uciok\n";
}

/**
 * Waits for command-line UCI input and parses appropriately
 **/
void UCI::takeUCIInput(istream &inputStr, ostream &outputStr) {
  // input string from user
  string input;

  // initialize search thread
  thread searchThread;

  // whether a search is in progress
  bool searching = false;

  // whether the search should output UCI information
  _uciSearchControl._uciOutput = true;

  // wait for input
  while (getline(inputStr, input)) {

    // join threads if search is complete
    if (_uciSearchControl._stopSearch == true) {
      if (searchThread.joinable()) {
        searchThread.join();
      }
    }

    // parse input
    vector<string> inputParts;
    inputParts = split(input, ' ');
    string commandName = inputParts.at(0);

    // take action based on command
    switch (mapCommands[commandName]) {
    case C_IS_READY:
      outputStr << "readyok\n";
      break;
    case C_UCI_NEW_GAME:
      _uciGameState = stateFromFEN("startpos");
      break;
    case C_POSITION:
      uciSetPosition(inputParts, input, _uciGameState);
      break;
    case C_GO:
      uciGo(inputParts, _uciSearchControl);
      searching = true;
      _uciSearchControl._analysisSide = _uciGameState._sideToMove;
      searchThread =
          thread(startSearch, ref(_uciGameState), ref(_uciSearchControl));
      break;
    case C_STOP:
      // stop search
      _uciSearchControl._stopSearch = true;
      if (searchThread.joinable()) {
        searchThread.join();
      }
      break;
    case C_PONDER_HIT:
      // not implemented
      break;
    case C_QUIT:
      // quit UCI
      _uciSearchControl._stopSearch = true;
      if (searchThread.joinable()) {
        searchThread.join();
      }
      break;
    case C_UCI:
      // not implemented
      break;
    default:
      outputStr << "Unrecognized UCI command: " << input << endl;
      break;
    }
  }
}

void uciGo(vector<string> inputParts, SearchController &_uciSearchControl) {
  for (int i = 1; i < inputParts.size(); i++) {
    // TODO: add i++ to those that need it (skip next item i.e. movetime 5000)
    switch (mapGoOptions[inputParts.at(i)]) {
    case G_SEARCHMOVES:
      // not implemented
      break;
    case G_PONDER:
      _uciSearchControl._moveTime = INT_MAX;
      _uciSearchControl._depthLimit = INT_MAX;
      break;
    case G_WTIME:
      _uciSearchControl._wTime = stoi(inputParts.at(i + 1));
      i++;
      break;
    case G_BTIME:
      _uciSearchControl._bTime = stoi(inputParts.at(i + 1));
      i++;
      break;
    case G_WINC:
      _uciSearchControl._wInc = stoi(inputParts.at(i + 1));
      i++;
      break;
    case G_BINC:
      _uciSearchControl._bInc = stoi(inputParts.at(i + 1));
      i++;
      break;
    case G_MOVESTOGO:
      _uciSearchControl._moveToGo = stoi(inputParts.at(i + 1));
      i++;
      break;
    case G_DEPTH:
      _uciSearchControl._maxDepth = stoi(inputParts.at(i + 1));
      i++;
      break;
    case G_NODES:
      _uciSearchControl._nodeLimit = stoi(inputParts.at(i + 1));
      i++;
      break;
    case G_MATE:
      // not implemented
      break;
    case G_MOVETIME:
      _uciSearchControl._moveTime = stoi(inputParts.at(i + 1));
      i++;
      break;
    case G_INFINITE:
      _uciSearchControl._moveTime = INT_MAX;
      _uciSearchControl._depthLimit = INT_MAX;
      break;
    default:
      cout << "Did not recognize \'go\' command parameters." << endl;
      break;
    }
  }
}

// sets the position based on passed string FEN and other parameters
void uciSetPosition(vector<string> inputParts, string input,
                    State &_uciGameState) {
  // index tracking how much has been parsed i.e. where to start looking
  int parsedIndex = 0;
  // check if startpos was sent
  string FEN = inputParts.at(1);
  if (FEN == "startpos") {
    _uciGameState = stateFromFEN("startpos");
    // start looking for more information at index 2
    parsedIndex = 2;
  } else {
    // compose FEN (vector indices 1-6 inclusive, and first part already there)
    for (int i = 2; i < 7; i++) {
      FEN += " " + inputParts.at(i);
    }
    _uciGameState = stateFromFEN(FEN);
    // start looking for move information at index 8
    parsedIndex = 7;
  }
  int i;
  for (i = parsedIndex; i < inputParts.size(); i++) {
    if (inputParts.at(i) == "moves") {
      i++;
      break;
    }
  }
  for (; i < inputParts.size(); i++) {
    Move move = moveFromUCI(inputParts.at(i));
    cout << moveToUCI(move) << endl;
    _uciGameState.makeMove(move);
  }
}

/**
 * Implementations of UCI functions
 **/
#include "../include/uci.hpp"
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
void UCI::startUCI() {
  // initialize command map
  init();

  // inform user
  cout << "id name MortyChess 0.1b\n";
  cout << "id author Stiven Deleur, Nathaniel Corley\n";
  cout << "uciok\n";
}

/**
 * Waits for command-line UCI input and parses appropriately
 **/
void UCI::takeUCIInput() {
  // input string from user
  string input;

  // initializations of state, controller, and thread
  SearchController uciSearchControl;
  State uciGameState;

  // create separate thread for search
  thread searchThread;

  // whether a search is in progress
  bool searching = false;

  // whether the search should output UCI information
  uciSearchControl._uciOutput = true;

  // wait for input
  while (getline(cin, input)) {

    // join threads if search is complete
    if (uciSearchControl._stopSearch == true) {
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
      cout << "readyok\n";
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
      searchThread =
          thread(startSearch, ref(uciGameState), ref(uciSearchControl));
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
      cout << "Unrecognized UCI command: " << input << endl;
      break;
    }
  }
}

void uciGo(vector<string> inputParts, SearchController uciSearchControl) {
  for (int i = 1; i < inputParts.size(); i++) {
    switch (mapGoOptions[inputParts.at(i)]) {
    case G_SEARCHMOVES:
      // not implemented
      break;
    case G_PONDER:
      uciSearchControl._moveTime = INT_MAX;
      uciSearchControl._depthLimit = INT_MAX;
      break;
    case G_WTIME:
      uciSearchControl._wTime = stoi(inputParts.at(i + 1));
      break;
    case G_BTIME:
      uciSearchControl._bTime = stoi(inputParts.at(i + 1));
      break;
    case G_WINC:
      uciSearchControl._wInc = stoi(inputParts.at(i + 1));
      break;
    case G_BINC:
      uciSearchControl._bInc = stoi(inputParts.at(i + 1));
      break;
    case G_MOVESTOGO:
      uciSearchControl._moveToGo = stoi(inputParts.at(i + 1));
      break;
    case G_DEPTH:
      uciSearchControl._maxDepth = stoi(inputParts.at(i + 1));
      break;
    case G_NODES:
      uciSearchControl._nodeLimit = stoi(inputParts.at(i + 1));
      break;
    case G_MATE:
      // not implemented
      break;
    case G_MOVETIME:
      uciSearchControl._moveTime = stoi(inputParts.at(i + 1)) / 1000;
      break;
    case G_INFINITE:
      uciSearchControl._moveTime = INT_MAX;
      uciSearchControl._depthLimit = INT_MAX;
      break;
    default:
      cout << "Did not recognize \'go\' command parameters." << endl;
      break;
    }
  }
}

// sets the position based on passed string FEN and other parameters
void uciSetPosition(vector<string> inputParts, string input,
                    State &uciGameState) {
  string FEN = inputParts.at(1);
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

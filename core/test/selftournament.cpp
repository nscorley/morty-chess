/**
 * Pits the current version of the executable against the prior standard
 * version. Determines the likelhood that any improvememts provide concrete
 * benefits. Should be used as a sanity test after alterations to search,
 * evaluations, etc.
 **/

// external dependencies
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <catch.hpp>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

// internal dependencies
#include "search.hpp"
#include "searchcontroller.hpp"
#include "state.hpp"
#include "util.hpp"

namespace bp = boost::process;
using namespace std;

int matchStatistics(int experimentalWins, int referenceWins, int draws);

/**
 * Test current engine against prior version of executable and evaluate
 * likelihood of superiority
 **/
TEST_CASE("Self Tournament", "[self-test]") {

  // list of equal positions (immedietly after opening moves) to provide
  // variance. engines evaluate both sides (white, then black for reference)
  string positions[19] = {
      "startpos",
      "rnbqkb1r/ppp1pppp/5n2/3p4/3P4/2P5/PP2PPPP/RNBQKBNR w KQkq - 0 0",
      "rnbqkb1r/ppp1pppp/5n2/3p4/3P4/2P5/PP2PPPP/RNBQKBNR b KQkq - 0 0",
      "rnbqkbnr/ppp1pppp/8/3p4/8/1P6/P1PPPPPP/RNBQKBNR w KQkq - 0 0",
      "rnbqkbnr/ppp1pppp/8/3p4/8/1P6/P1PPPPPP/RNBQKBNR b KQkq - 0 0",
      "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 0",
      "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 0",
      "rnbqkbnr/ppp1pppp/8/3p4/8/6P1/PPPPPP1P/RNBQKBNR w KQkq - 0 0",
      "rnbqkbnr/ppp1pppp/8/3p4/8/6P1/PPPPPP1P/RNBQKBNR b KQkq - 0 0",
      "rnbqkbnr/ppp1pppp/8/3p4/8/P7/1PPPPPPP/RNBQKBNR w KQkq - 0 0",
      "rnbqkbnr/ppp1pppp/8/3p4/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 0",
      "rnbqkb1r/pp1ppppp/5n2/2p5/4P3/2N5/PPPPNPPP/R1BQKB1R b KQkq - 0 0",
      "rnbqkb1r/pp1ppppp/5n2/2p5/4P3/2N5/PPPPNPPP/R1BQKB1R w KQkq - 0 0",
      "rnbqkbnr/1ppppppp/p7/8/4P3/2P5/PP1P1PPP/RNBQKBNR b KQkq - 0 0",
      "rnbqkbnr/1ppppppp/p7/8/4P3/2P5/PP1P1PPP/RNBQKBNR w KQkq - 0 0",
      "rnbqkbnr/ppp2ppp/8/3pp3/4PP2/8/PPPP2PP/RNBQKBNR w KQkq - 0 0",
      "rnbqkbnr/ppp2ppp/8/3pp3/4PP2/8/PPPP2PP/RNBQKBNR w KQkq - 0 0",
      "rnbqkbnr/pppp1ppp/8/4p3/4PP2/8/PPPP2PP/RNBQKBNR b KQkq - 0 0",
      "rnbqkbnr/pppp1ppp/8/4p3/4PP2/8/PPPP2PP/RNBQKBNR w KQkq - 0 0"};

  // path to benchmark executable
  boost::filesystem::path execPath = "../app/bin/morty-chess-benchmark";

  // setup test version
  initPresets();

  // count wins
  int experimentalWins = 0;
  int referenceWins = 0;
  int draws = 0;

  // loop through all silent positions
  for (string &position : positions) {
    int movetime = 500;

    // reading pipe-stream
    bp::ipstream out;

    // writing pipe-stream
    bp::opstream in;

    // setup state and search controller
    State state = stateFromFEN(position);
    SearchController sControl;
    sControl._moveTime = movetime;
    sControl._uciOutput = false;
    sControl._output = false;
    clearHashTable(&sControl.table);

    // start process
    bp::child c(execPath, bp::std_out > out, bp::std_in < in);

    // line to read input into
    std::string line;

    // whether UCI has been initialized in the engine
    bool uciInitialized = false;

    cout << "Starting simulation from FEN " << position << endl;

    // TODO: play from both sides (white & black)
    // TODO: multithreading to speed up testing
    while (c.running() && std::getline(out, line)) {
      // check for uci initialization
      if (line == "uciok") {
        uciInitialized = true;
        // trigger executable
        in << "position " << position << endl;
        in << "go movetime " << movetime << endl;
        continue;
      } else if (!uciInitialized) {
        // send "uci" until engine response with "uciok"
        in << "uci" << endl;
        continue;
      }

      // consider it a draw with more than 200 moves
      if (state._fullMoveCounter > 200) {
        cout << "Terminating game because too many moves without "
                "checkmate."
             << endl;
        c.terminate();
        draws++;
        break;
      }

      // split line into parts
      vector<string> inputParts;
      inputParts = split(line, ' ');

      // check if bestmove was sent
      if (inputParts.at(0) == "bestmove") {
        // make the move
        Move m = moveFromUCI(inputParts.at(1));
        state.makeMove(m);

        if (state._fullMoveCounter % 25 == 0) {
          state.printState();
          cout << "Move " << state._fullMoveCounter
               << " Evaluation: " << state._lineEval << endl;
        }

        // check for mate
        if (state.isCheckmate()) {
          referenceWins++;
          cout << "Reference MortyChess won in " << state._fullMoveCounter
               << " moves." << endl;
          state.printState();
          cout << "Experimental " << experimentalWins << " - " << referenceWins
               << " Reference" << endl;
          c.terminate();
          break;
        }

        // perform search
        startSearch(state, sControl);
        state.makeMove(state._bestLine.moves[0]);

        // check for mate
        if (state.isCheckmate()) {
          experimentalWins++;
          cout << "Experimental MortyChess won in " << state._fullMoveCounter
               << " moves." << endl;
          state.printState();
          cout << "Experimental " << experimentalWins << " - " << referenceWins
               << " Reference" << endl;
          c.terminate();
          break;
        }

        // send updated state to executable and start search
        in << "position " << stateToFEN(state) << endl;
        in << "go movetime " << movetime << endl;
      }
    }
  }

  // calculate and prin match Statistics
  cout << "### FINISHED SELF TOURNAMENT TESTING" << endl;
  int los = matchStatistics(experimentalWins, referenceWins, draws);

  // require >= 55% likelhood of superiority
  REQUIRE(los >= 0.55);
}

/**
 * Calculates the probability that the engine with more wins is superior
 * i.e. whether the null hypothesis can be rejected (p-value)
 * See https://chessprogramming.wikispaces.com/Match+Statistics
 * @return the LOS score
 **/
int matchStatistics(int wins, int losses, int draws) {
  double games = wins + losses + draws;
  std::printf("Number of games: %g\n", games);
  double winning_fraction = (wins + 0.5 * draws) / games;
  std::printf("Winning fraction: %g\n", winning_fraction);
  double elo_difference =
      -std::log(1.0 / winning_fraction - 1.0) * 400.0 / std::log(10.0);
  std::printf("Elo difference: %+g\n", elo_difference);
  double los =
      .5 + .5 * std::erf((wins - losses) / std::sqrt(2.0 * (wins + losses)));
  std::printf("LOS: %g\n", los);
  return los;
}

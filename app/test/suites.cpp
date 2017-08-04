/**
 * Runs the engine against a series of test positions and compares
 * search results with the accepted best move. Do not optimize on these results.
 * Only use these tests as a sanity check after altering search functionality.
 **/
#include "catch.hpp"
#include "search.hpp"
#include "searchcontroller.hpp"
#include "state.hpp"
#include "util.hpp"

// declaration of reusable function to text positions file
int testPositions(std::string testPath, int secondsPerPosition);

/**
 * The Bratko-Kopec tests are 24 positions designed to estimate elo.
 * Run at 25 seconds/position
 **/
TEST_CASE("Bratko-Kopec Tests", "[Bratko-Kopec][test-suites]") {
  initPresets();
  int numCorrect = testPositions("app/test/res/Bratko-Kopec.text", 1);
  std::cout << "Completed Bratko-Kopec tests with " << numCorrect
            << " correct responses." << std::endl;
}

/**
 * The WAN (Win at Chess) suite are 300 tactical positions.
 * Run at 6 seconds/position
 **/
TEST_CASE("WAC Tests", "[WAC][test-suites]") {
  initPresets();
  int numCorrect = testPositions("app/test/res/WAC.text", 6);
  std::cout << "Completed Win-At-Chess tests with " << numCorrect
            << " correct responses." << std::endl;
}

int testPositions(std::string testPath, int secondsPerPosition = 6) {
  // initialize state and number correct
  State state;
  int numCorrect = 0;

  // loop through positions
  std::ifstream file(testPath);
  std::string line;
  while (std::getline(file, line)) {
    // parse line into components
    KeyInfoMap info = splitEDP(line);
    state = boardFromFEN(info["fen"]);

    // setup controller
    SearchController sControl;
    sControl._moveTime = secondsPerPosition * 1000;
    sControl._output = false;

    // perform search
    startSearch(state, sControl);

    // get results
    std::string result = moveToSAN(state._bestLine.moves[0], state).c_str();

    // confirm results and increment correct
    CHECK(result == info["bm"].c_str());

    if (result == info["bm"]) {
      numCorrect++;
    }

    // log results
    printf("FEN: %s, Engine BM (eval): %s (%i), Given BM: %s\n",
           info["fen"].c_str(), result.c_str(), state._lineEval,
           info["bm"].c_str());
  }

  return numCorrect;
}

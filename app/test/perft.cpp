/**
 * The PERFT test runs through a set of particularly complicated test positions
 * and generates moves for the first 6 depths. The total number of generated
 * moves is compared against the accepted number. This should be run after any
 * changes to move generation.
 **/

#include "catch.hpp"
#include "movegenerator.hpp"
#include "state.hpp"
#include "util.hpp"
using namespace std;

// recursive accessory function for perft tests
int perftTest(State state, int depth) {
  // first depth the current node is the leaf node
  if (depth <= 0) {
    return 1;
  }

  // calculate legal moves
  vector<int> moves = generatePseudoMoves(state);
  int runningSum = 0;
  for (vector<int>::iterator moveIt = moves.begin(); moveIt != moves.end();
       ++moveIt) {
    state.makeMove(*moveIt);
    if (state.isPositionLegal()) {
      runningSum += perftTest(state, depth - 1);
    }
    state.takeMove();
  }
  return runningSum;
}

/**
 * Contains 126 test positions.
 **/
TEST_CASE("PERFT Test", "[movegeneration]") {
  // populate hash tables, etc.
  initPresets();

  // state object to perform operations on
  State state;

  // PERFT test constants
  int TEST_LIMIT = 200;
  int maxDepth = 6;
  int perftStart = 1;
  const clock_t startTime = clock();

  // get position strings
  ifstream file("app/test/res/perfttests.text");

  string line;
  vector<string> data;
  while (getline(file, line)) {
    data.push_back(line);
  }

  // remove unwanted tests
  data.erase(data.begin(), data.begin() + (perftStart - 1));

  // keep track of which test is being performed
  int testNum = perftStart - 1;

  // loop through positions
  for (vector<string>::iterator it = data.begin(); it != data.end(); ++it) {
    // increment test and check for limit
    testNum++;
    if (testNum > TEST_LIMIT) {
      break;
    }

    // get FEN and move information
    vector<string> subLine = split(*it, ';');
    string FEN = subLine[0];
    FEN.erase(FEN.find_last_not_of(" \n\r\t") + 1);

    // get correct results
    int depths[6] = {0};
    for (int i = 1; i < subLine.size(); i++) {
      depths[i - 1] = stoi(subLine[i].substr(3));
    }

    // alter state to represent position
    state = boardFromFEN(FEN);

    // run test
    printf("\n### Running Test #%d ###\n", testNum);
    // log the board and print test information
    // state.printBoard();
    for (int i = 0; i < 6 && depths[i] != 0; i++) {
      // time test
      const clock_t iStartTime = clock();

      // check depth
      if (i >= maxDepth) {
        break;
      }

      printf("Starting test to depth %d on FEN %s\n", i + 1, FEN.c_str());

      // initialize node count at depth (what PERFT counts)
      int leafNodes = 0;

      // get pseudo moves
      vector<int> moves = generatePseudoMoves(state);
      printf("%lu root moves:\n", moves.size());

      // check for legal moves
      int moveNum = 0;
      for (vector<int>::iterator moveIt = moves.begin(); moveIt != moves.end();
           ++moveIt) {
        int oldNodes = leafNodes;
        state.makeMove(*moveIt);
        if (state.isPositionLegal()) {
          // if move is legal, increment move number
          moveNum++;
          // enter recursive loop to calculate leaf nodes
          leafNodes += perftTest(state, i);
          cout << "Move " << moveNum << " " << moveToUCI(*moveIt).c_str() << " "
               << leafNodes - oldNodes << endl;
        }
        state.takeMove();
      }
      printf("Leaf nodes: %d, expected: %d; Finished in %f seconds\n",
             leafNodes, depths[i],
             float(clock() - iStartTime) / CLOCKS_PER_SEC);
      REQUIRE((int)depths[i] == leafNodes);
    }
  }
  printf("PERFT tests finished successfully in %f minutes\n",
         (float(clock() - startTime) / (CLOCKS_PER_SEC * 60.0)));
}

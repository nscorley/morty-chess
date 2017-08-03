#include "catch.hpp";
#include "movegenerator.hpp"
#include "state.hpp"
#include "util.hpp"

// recursive accessory function for perft tests
int perftTest(State &state, int depth, int leafNodes) {
  // first depth the current node is the leaf node
  if (depth == 0) {
    return leafNodes + 1;
  }

  // calculate legal moves
  std::vector<int> moves = generatePseudoMoves(state);
  for (std::vector<int>::iterator moveIt = moves.begin(); moveIt != moves.end();
       ++moveIt) {
    state.makeMove(*moveIt);
    if (state.isPositionLegal()) {
      perftTest(state, depth - 1, leafNodes + 1);
    }
    state.takeMove();
  }
}

TEST_CASE("PERFT Test Succeeds", "[perft, movegeneration]") {
  // populate hash tables, etc.
  initPresets();

  // state object to perform operations on
  State state;

  // PERFT test constants
  int TEST_LIMIT = 200;
  int maxDepth = 6;
  int perftStart = 1;

  // get position strings
  std::ifstream file("perfttests.text");
  std::string line;
  std::vector<std::string> data;
  while (std::getline(file, line)) {
    data.push_back(line);
  }

  // remove unwanted tests
  data.erase(data.begin(), data.begin() + (perftStart - 1));

  // keep track of which test is being performed
  int testNum = perftStart - 1;

  // loop through positions
  for (std::vector<std::string>::iterator it = data.begin(); it != data.end();
       ++it) {
    // increment test and check for limit
    testNum++;
    if (testNum > TEST_LIMIT) {
      break;
    }

    // get FEN and move information
    std::vector<std::string> subLine = split(*it, ';');
    std::string FEN = subLine[0];
    FEN.erase(FEN.find_last_not_of(" \n\r\t") + 1);

    // get correct results
    int depths[6] = {0};
    for (int i = 1; i < subLine.size(); i++) {
      depths[i - 1] = std::stoi(subLine[i].substr(3));
    }

    // alter state to represent position
    state = boardFromFEN(FEN);

    // run test
    printf("\n### Running Test #%d ###\n", testNum);
    for (int i = 0; i < 6 && depths[i] != 0; i++) {
      // time test
      const clock_t startTime = clock();

      // check depth
      if (i >= maxDepth) {
        break;
      }

      // log the board and print test information
      state.printBoard();
      printf("Starting test to depth %d on FEN %s\n", i + 1, FEN.c_str());

      // initialize node count at depth (what PERFT counts)
      leafNodes = 0;

      // get pseudo moves
      std::vector<int> moves = generatePseudoMoves(state);
      printf("%lu root moves:\n", moves.size());

      // check for legal moves
      int moveNum = 0;
      for (std::vector<int>::iterator moveIt = moves.begin();
           moveIt != moves.end(); ++moveIt) {
        int oldNodes = leafNodes;
        state.makeMove(*moveIt);
        if (state.isPositionLegal()) {
          // if move is legal, increment move number
          moveNum++;
          // enter recursive loop to calculate leaf nodes
          perftTest(state, i, leafNodes);
          printf("Move %d: %s %i\n", (moveNum), moveToUCI(*moveIt).c_str(),
                 leafNodes - oldNodes);
        }
        state.takeMove();
      }
      printf("Leaf nodes: %d, expected: %d; Finished in %f seconds\n",
             leafNodes, depths[i],
             float(clock() - iStartTime) / CLOCKS_PER_SEC);
      REQUIRE((int)depths[i] == leafNodes);
    }
  }

  REQUIRE(Factorial(1) == 1);
  REQUIRE(Factorial(2) == 2);
  REQUIRE(Factorial(3) == 6);
  REQUIRE(Factorial(10) == 3628800);
}

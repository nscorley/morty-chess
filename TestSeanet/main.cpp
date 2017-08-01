#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
                          // in one cpp file
#include "board.hpp"
#include "catch.hpp"
#include "hash.hpp"
#include "movegenerator.hpp"
#include "search.hpp"
#include "searchcontroller.hpp"
#include "util.hpp"
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

int leafNodes = 0;
void perftTest(State &b, int depth);
void divide(std::string FEN, int depth);
void engineTest(std::string testPath, std::string tag, std::string comments,
                int secondsPerPosition = 25);
void runSearch(std::string FEN, SearchController &sControl);
void speedTest(std::string testPath);

TEST_CASE("Checking moveToSAN() and moveFromSAN()", "[SAN-functions]") {
  initPresets();
  std::vector<std::string> files;
  files.push_back("LCT-II.text");
  files.push_back("Bratko-Kopec.text");
  files.push_back("san-checking.text");

  std::string line;
  State state;

  for (auto it : files) {
    std::ifstream file(it);

    while (std::getline(file, line)) {
      KeyInfoMap info = splitEDP(line);
      state = boardFromFEN(info["fen"]);
      std::string move = info["bm"];
      Move m1 = moveFromSAN(move, state);
      std::string m2 = moveToSAN(m1, state);
      REQUIRE(move == m2);
    }
  }
}

TEST_CASE("Ensuring Zobrist functions properly", "[Zobrist-Checking]") {
  initPresets();
  std::ifstream file("zobrist-checking.text");
  std::string line;
  std::vector<std::string> data;
  std::vector<U64> firstRun;
  std::vector<U64> secondRun;
  while (std::getline(file, line)) {
    data.push_back(line);
  }
  for (auto fen : data) {
    State s = boardFromFEN(fen);
    firstRun.push_back(getZobristHash(s));
    std::cout << "FEN: " << fen << " " << getZobristHash(s) << std::endl;
    secondRun.push_back(getZobristHash(s));
  }

  std::set<U64> Y(firstRun.begin(), firstRun.end());
  REQUIRE(firstRun.size() == Y.size());
  std::set<U64> Z(secondRun.begin(), secondRun.end());
  REQUIRE(secondRun.size() == Z.size());

  for (int i = 0; i < firstRun.size(); i++) {
    REQUIRE(firstRun[i] == secondRun[i]);
  }
}

TEST_CASE("Checking boardToFEN() and boardFromFEN()", "[fenFunctions]") {
  State state;

  std::ifstream file("zobrist-checking.text");
  std::string line;
  while (std::getline(file, line)) {
    std::string correctFEN;
    std::istringstream is(line);
    std::getline(is, correctFEN, ';');
    correctFEN.erase(correctFEN.find_last_not_of(" \n\r\t") + 1);
    state = boardFromFEN(correctFEN);
    std::string testFEN = boardToFEN(state);
    REQUIRE(testFEN == correctFEN);
  }
}

TEST_CASE("Running Bratko-Kopec Tests", "[Bratko-Kopec]") {
  engineTest("Bratko-Kopec.text", "[Bratko-Kopec]",
             "Stiven - PV_R SEE_R KH_R HH_R NULL_M TT_EVAL TT_R PVS"); // Add
  // descriptive
  // comments
  // with each
  // full run
}
TEST_CASE("Running LCT-II Tests", "[LCT-II]") {
  engineTest("LCT-II.text", "[LCT-II]",
             "Stiven - PV_R SEE_R KH_R HH_R NULL_M TT_EVAL TT_R PVS"); // Add
  // descriptive
  // comments
  // with each full run
}

TEST_CASE("Running PERFT tests", "[perft]") {
  State state;
  initPresets();
  int TEST_LIMIT = 200;
  int maxDepth = 6;
  int perftStart = 1;
  std::string divideFEN = "";
  const clock_t startTime = clock();
  // divideFEN = "1K6/8/8/3Q4/4q3/8/7k/8 w - - 2 1";
  // 1";

  if (divideFEN != "") {
    divide(divideFEN, 2);
    return;
  }

  std::ifstream file("perfttests.text");
  std::string line;
  std::vector<std::string> data;
  while (std::getline(file, line)) {
    data.push_back(line);
  }
  data.erase(data.begin(), data.begin() + (perftStart - 1));
  int testNum = perftStart - 1;
  for (std::vector<std::string>::iterator it = data.begin(); it != data.end();
       ++it) {
    testNum++;
    if (testNum > TEST_LIMIT) {
      break;
    }
    std::vector<std::string> subLine = split(*it, ';');
    std::string FEN = subLine[0];
    FEN.erase(FEN.find_last_not_of(" \n\r\t") + 1);
    int depths[6] = {0};
    for (int i = 1; i < subLine.size(); i++) {
      depths[i - 1] = std::stoi(subLine[i].substr(3));
    }
    state = boardFromFEN(FEN);
    printf("\n### Running Test #%d ###\n", testNum);
    for (int i = 0; i < 6 && depths[i] != 0; i++) {
      const clock_t iStartTime = clock();
      if (i >= maxDepth) {
        break;
      }
      state.printBoard();
      printf("Starting test to depth %d on FEN %s\n", i + 1, FEN.c_str());
      leafNodes = 0;
      std::vector<int> moves = generatePseudoMoves(state);
      printf("%lu root moves:\n", moves.size());
      int moveNum = 0;
      for (std::vector<int>::iterator moveIt = moves.begin();
           moveIt != moves.end(); ++moveIt) {
        int oldNodes = leafNodes;
        state.makeMove(*moveIt);
        if (state.isPositionLegal()) {
          moveNum++;
          perftTest(state, i);
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
  printf("PERFT test finished successfully in %f minutes\n",
         (float(clock() - startTime) / (CLOCKS_PER_SEC * 60.0)));
}

TEST_CASE("Running Feature Speed Test on Opening Positions",
          "[Speed-Opening]") {
  speedTest("Silver-Suite.text");
}

TEST_CASE("Running Feature Speed Test on Endgame Positions",
          "[Speed-Endgame]") {
  speedTest("Sune-Larson-Endgame.text");
}

void speedTest(std::string testPath) {
  initPresets();

  std::ifstream file(testPath);
  std::vector<KeyInfoMap> positions;
  std::string line;

  int posLimit = 20;

  while (std::getline(file, line)) {
    positions.push_back(splitEDP(line));
  }
  if (posLimit < positions.size()) {
    std::random_shuffle(positions.begin(), positions.end());
  }

  bool control[NUM_OF_FEATURES];
  control[PV_REORDERING] = true;
  control[SEE_REORDERING] = true;
  control[KH_REORDERING] = true;
  control[HH_REORDERING] = true;
  control[NULL_MOVE] = true;
  control[TT_EVAL] = true;
  control[TT_REORDERING] = true;
  control[PV_SEARCH] = true;
  control[ASPIRATION_WINDOWS] = true;
  control[QS_REORDERING] = false;

  const int degreesOfFreedom = 1;
  bool allFeatures[] = {true, true, true, true, true,
                        true, true, true, true, true};
  std::vector<bool *> featureConfigs;

  bool configs[(int)std::pow(2, degreesOfFreedom) - 1][NUM_OF_FEATURES];
  for (int i = 1; i < std::pow(2, degreesOfFreedom) - 1; i++) {

    std::string s = std::bitset<degreesOfFreedom>(i).to_string();
    int degreesUsed = 0;
    for (int j = 0; j < NUM_OF_FEATURES; j++) {
      configs[i][j] = true;
      if (!control[j]) {
        if (s[degreesUsed] == '0') {
          configs[i][j] = false;
        }
        degreesUsed++;
      }
    }
    featureConfigs.push_back(configs[i]);
  }

  std::random_shuffle(featureConfigs.begin(), featureConfigs.end());
  featureConfigs.insert(featureConfigs.begin(), control);
  featureConfigs.push_back(allFeatures);

  std::vector<int> totalNodes(featureConfigs.size());
  std::vector<int> totalTime(featureConfigs.size());
  int posNum = 0;
  for (KeyInfoMap info : positions) {
    std::string FEN = info["fen"];
    posNum++;
    if (posNum > posLimit) {
      break;
    }
    std::cout << "\n##### TESTING NEW POSITION #####" << std::endl;
    std::cout << posNum << ". [" << info["id"] << "] " << FEN << std::endl
              << std::endl;
    for (int i = 0; i < featureConfigs.size(); i++) {
      bool *config = featureConfigs[i];
      SearchController sControl;
      // sControl._output = false;
      clearHashTable(&sControl.table);
      std::copy(config, config + NUM_OF_FEATURES, sControl._features);
      sControl._timeLimit = INT_MAX;
      sControl._depthLimit = 7;

      runSearch(FEN, sControl);
      timeval currTime;
      gettimeofday(&currTime, 0);
      int timeElapsed =
          (int)(timeToMS(currTime) - timeToMS(sControl._startTime)) + 1;

      totalNodes[i] += sControl._totalNodes;
      totalTime[i] += timeElapsed;
      std::cout << "[ " << sControl.featuresToString() << "]" << std::endl;
      std::cout << timeElapsed << " ms; "
                << (int)(sControl._totalNodes / (timeElapsed)) << " kn/s"
                << "; "
                << (float)(100.0 * sControl._fhfNodes / sControl._fhNodes)
                << "% fhf"
                << "; "
                << (float)(100.0 * sControl._fhNodes / sControl._totalNodes)
                << "% fh"
                << "; " << (sControl._totalNodes / 1000) << "K nodes; "
                << sControl._transpositions << " transpositions"
                << "; seldepth " << sControl._maxDepth << std::endl;
      std::cout << "qNodes: " << sControl._qNodes
                << "; Main Nodes: " << sControl._mainNodes
                << "; Exact Nodes Used: " << sControl._exactNodes
                << "; Alpha Nodes Used: " << sControl._alphaNodes
                << "; Beta Nodes Used: " << sControl._betaNodes << "\n"
                << std::endl;
    }
  }

  float timeElapsedControl = totalTime[0] / 1000.0;
  int avgSpeedControl = (int)(totalNodes[0] / totalTime[0]);
  int nodesControl = totalNodes[0] / 1000;

  double bestTimeChange = 0;
  bool *bestTimeConfig = featureConfigs[0];

  double bestSpeedChange = 0;
  bool *bestSpeedConfig = featureConfigs[0];

  double bestNodeChange = 0;
  bool *bestNodeConfig = featureConfigs[0];
  for (int i = 0; i < featureConfigs.size(); i++) {
    float timeElapsed = totalTime[i] / 1000.0;
    int avgSpeed = (int)(totalNodes[i] / totalTime[i]);
    int nodes = totalNodes[i] / 1000;

    double timeChange =
        100.0 * (timeElapsedControl - timeElapsed) / timeElapsedControl;
    double speedChange = 100.0 * (avgSpeed - avgSpeedControl) / avgSpeedControl;
    double nodeChange = 100.0 * (nodesControl - nodes) / nodesControl;

    std::cout << "Features: " << searchFeaturesToString(featureConfigs[i])
              << std::endl;
    std::cout << "Time elapsed: " << timeElapsed << " sec; ";
    std::cout << timeChange << "% change\n";
    std::cout << "Average speed: " << avgSpeed << " kn/s; ";
    std::cout << speedChange << "% change\n";
    std::cout << "Nodes computed: " << nodes << "K; ";
    std::cout << nodeChange << "% change\n";
    std::cout << std::endl;

    if (timeChange > bestTimeChange) {
      bestTimeChange = timeChange;
      bestTimeConfig = featureConfigs[i];
    }
    if (speedChange > bestSpeedChange) {
      bestSpeedChange = speedChange;
      bestSpeedConfig = featureConfigs[i];
    }
    if (nodeChange > bestNodeChange) {
      bestNodeChange = nodeChange;
      bestNodeConfig = featureConfigs[i];
    }
  }
  std::cout << "Highest time change: " << bestTimeChange << "% [ "
            << searchFeaturesToString(bestTimeConfig) << "]" << std::endl;
  std::cout << "Highest speed change: " << bestSpeedChange << "% [ "
            << searchFeaturesToString(bestSpeedConfig) << "]" << std::endl;
  std::cout << "Highest node change: " << bestNodeChange << "% [ "
            << searchFeaturesToString(bestNodeConfig) << "]" << std::endl;
}

void perftTest(State &state, int depth) {

  if (depth == 0) {
    leafNodes++;
    return;
  }

  std::vector<int> moves = generatePseudoMoves(state);

  for (std::vector<int>::iterator moveIt = moves.begin(); moveIt != moves.end();
       ++moveIt) {
    state.makeMove(*moveIt);
    if (state.isPositionLegal()) {
      perftTest(state, depth - 1);
    }
    state.takeMove();
  }
}

void runSearch(std::string FEN, SearchController &sControl) {
  State state = boardFromFEN(FEN);
  search(state, sControl);
}

void engineTest(std::string testPath, std::string tag, std::string comments,
                int secondsPerPosition) {
  State state;
  initPresets();
  std::ifstream file(testPath);
  std::string line;
  int numCorrect = 0;
  while (std::getline(file, line)) {
    KeyInfoMap info = splitEDP(line);
    state = boardFromFEN(info["fen"]);
    SearchController sControl;
    sControl._timeLimit = secondsPerPosition;
    sControl._depthLimit = INT_MAX;
    search(state, sControl);
    std::string result = moveToSAN(state._bestLine.moves[0], state).c_str();
    printf("FEN: %s, Engine BM (eval): %s (%i), Given BM: %s\n",
           info["fen"].c_str(), result.c_str(), state._lineEval,
           info["bm"].c_str());
    CHECK(result == info["bm"].c_str());
    if (result == info["bm"]) {
      numCorrect++;
    }
  }
  char buff[DTTMSZ];
  std::fstream records;
  records.open("tactics-records.text", std::fstream::out | std::fstream::app);
  records << "test = " << tag << "; date = " << getDtTm(buff)
          << "; correct = " << numCorrect
          << "; seconds/position = " << secondsPerPosition
          << "; comments = " << comments << ";\n";
  records.close();
}

void divide(std::string FEN, int depth) {

  State state = boardFromFEN(FEN);
  state.printBoard();
  printf("Starting divide to depth %d on FEN %s\n", depth, FEN.c_str());
  leafNodes = 0;
  std::vector<int> moves = generatePseudoMoves(state);
  printf("%lu root moves:\n", moves.size());
  int moveNum = 0;
  for (std::vector<int>::iterator moveIt = moves.begin(); moveIt != moves.end();
       ++moveIt) {
    int oldNodes = leafNodes;
    state.makeMove(*moveIt);
    if (state.isPositionLegal()) {
      moveNum++;
      perftTest(state, depth - 1);
      printf("Move %d: %s %i\n", (moveNum), moveToUCI(*moveIt).c_str(),
             leafNodes - oldNodes);
    }

    state.takeMove();
  }
  printf("Leaf nodes: %d\n", leafNodes);
}

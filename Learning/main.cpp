//
//  main.cpp
//  Learning
//
//  Created by Stiven Deleur on 5/31/16.
//
//

#include "datum.hpp"
#include "neuralnet.hpp"
#include "perceptron.hpp"
#include <fstream>
#include <iostream>

float alpha = 0.5;
NeuralNet *midgameNeuralNet;
NeuralNet *endgameNeuralNet;
void loadExistingWeights(NeuralNet *net, std::string name);
void recordWeights(NeuralNet *net, std::string name);
void loadRandomWeights(NeuralNet *net);
std::vector<Datum> loadData(std::vector<std::string> pgnFiles);

int main(int argc, const char *argv[]) {

  initPresets();

  // Randomly seed rand() function
  timeval currTime;
  gettimeofday(&currTime, 0);
  int now = (int)(timeToMS(currTime));
  srand(now);

  std::vector<int> layers = {NUM_OF_BOARD_FEATURES, 5,
                             1}; // layers[0] is initial features
  midgameNeuralNet = new NeuralNet(layers);
  endgameNeuralNet = new NeuralNet(layers);

  loadRandomWeights(midgameNeuralNet);
  loadRandomWeights(endgameNeuralNet);
  std::cout << "Loaded wieghts." << std::endl;
  recordWeights(midgameNeuralNet, "midgame");
  std::cout << "Recorded weights." << std::endl;

  std::vector<std::string> pgnGames;
  pgnGames.push_back("Carlsen.pgn");

  std::cout << "Loading data..." << std::endl;
  std::vector<Datum> data = loadData(pgnGames);
  std::cout << "Loaded " << data.size() << " pseudo quiet training examples."
            << std::endl;

  // Counter for number of examples tested in current training session
  int numExamples = 0;

  std::vector<Datum> trainingData(data.begin(),
                                  data.begin() + 9 * data.size() / 10);
  std::vector<Datum> testData(data.begin() + 9 * data.size() / 10, data.end());

  while (true) {
    for (Datum datum : trainingData) {
      // Increment counter
      numExamples++;

      // Perform training
      trainTwoNets(datum._state, datum._bestMove, midgameNeuralNet,
                   endgameNeuralNet, alpha);

      // Record the current weights every n training positions
      if (numExamples % 50000 == 0) {
        recordWeights(midgameNeuralNet, "midgame");
        recordWeights(endgameNeuralNet, "endgame");
        std::cout << "Recorded weights at training example #" << numExamples
                  << std::endl;
        int correct = 0;
//        for (Datum testDatum : trainingData) {
//
//          // Perform training
//          if (trainTwoNets(testDatum._state, testDatum._bestMove,
//                           midgameNeuralNet, endgameNeuralNet, alpha, false)) {
//            correct++;
//          }
//        }
//        std::cout << correct << "/" << trainingData.size() << " ("
//                  << 100.0 * correct / trainingData.size()
//                  << "%) correct on training set" << std::endl;
//
//        correct = 0;
        for (Datum testDatum : testData) {

          // Perform training
          if (trainTwoNets(testDatum._state, testDatum._bestMove,
                           midgameNeuralNet, endgameNeuralNet, alpha, false)) {
            correct++;
          }
        }
        std::cout << correct << "/" << testData.size() << " ("
                  << 100.0 * correct / testData.size()
                  << "%) correct on test set" << std::endl;
      }
    }
  }
  delete midgameNeuralNet;
  delete endgameNeuralNet;
  return 0;
}

void loadRandomWeights(NeuralNet *net) {
  std::vector<std::vector<FList>> weights = net->getWeights();
  for (int layer = 0; layer < weights.size(); layer++) {
    for (int pn = 0; pn < weights[layer].size(); pn++) {
      for (int f = 0; f < weights[layer][pn].size(); f++) {
        net->_perceptrons[layer][pn]._weights[f] =
            static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      }
    }
  }
}

void loadExistingWeights(NeuralNet *net, std::string name) {}

void recordWeights(NeuralNet *net, std::string name) {
  char buff[DTTMSZ];
  std::fstream records;
  records.open("weight-records.text", std::fstream::out | std::fstream::app);
  records << "<network name=" << name << ">\n"
          << "<info>Weights as of " << getDtTm(buff) << "</info>\n<weights>\n";
  std::vector<std::vector<FList>> weights = net->getWeights();
  for (int layer = 0; layer < weights.size(); layer++) {
    records << "<layer>\n";
    for (int pn = 0; pn < weights[layer].size(); pn++) {
      records << "(";
      for (int f = 0; f < weights[layer][pn].size(); f++) {
        records << weights[layer][pn][f];
        if (f < weights[layer][pn].size() - 1) {
          records << ", ";
        }
      }
      records << ")";
    }
    records << "\n</layer>\n";
  }
  records << "</weights>\n";
  records << "</network>\n";
  records.close();
}

std::vector<Datum> loadData(std::vector<std::string> pgnFiles) {
  std::vector<Datum> data;

  for (std::string fileName : pgnFiles) {
    std::vector<std::string> games =
        exportGamesFromPGN(std::ifstream(fileName));
    for (std::string game : games) {

      std::vector<Move> moveLine = getGameMoveLine(game);
      int winner = getPGNGameWinner(game);
      State state = boardFromFEN("startpos");
      for (Move move : moveLine) {
        if (winner == NONE || state._sideToMove == winner) {
          if (isPseudoQuiet(state)) {
            data.emplace_back(state, move, 0);
          }
        }
        state.makeMove(move);
      }
    }
  }

  return data;
}

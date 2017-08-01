//
//  perceptron.cpp
//  Seanet
//
//  Created by Stiven Deleur on 5/31/16.
//
//

#include "perceptron.hpp"
#include <iostream>
#include <math.h>

Perceptron::Perceptron(){};
Perceptron::Perceptron(int size) { _weights.resize(size); }

float Perceptron::calculateActivation(FList features) {
  float sum = 0;

  int numOfFeatures = (int)features.size();

  if (numOfFeatures != _weights.size()) {
    std::cout << "WARNING: Length of features and weights don't match!";
  }

  // iterate through features and sum
  for (int i = 0; i < numOfFeatures; i++) {
    sum += features[i] * _weights[i];
  }

  return sum;
}

void Perceptron::setWeights(FList weights) { _weights = weights; }

void Perceptron::updateWeights(FList correctFeatures, FList wrongFeatures,
                               float alpha) {

  int numOfFeatures = (int)correctFeatures.size();

  if (numOfFeatures != _weights.size() ||
      numOfFeatures != wrongFeatures.size()) {
    std::cout << "WARNING: Length of features and weights don't match!";
  }

  // iterate through weights and update according to features

  for (int i = 0; i < numOfFeatures; i++) {
    _weights[i] += alpha * (correctFeatures[i] - wrongFeatures[i]);
  }
}

float activationFunction(float sum) {
  // Calculate activation
  return 1.0 / (1.0 + exp(-sum));
}
//
//  perceptron.hpp
//  Seanet
//
//  Created by Stiven Deleur on 5/31/16.
//
//

#ifndef perceptron_hpp
#define perceptron_hpp

#include "defs.h"
#include <stdio.h>
#include <vector>

class Perceptron {

public:
  Perceptron();
  Perceptron(int size);
  FList _weights;
  float calculateActivation(FList features);
  void setWeights(FList weights);
  void updateWeights(FList correctFeatures, FList wrongFeatures, float alpha);
};

float activationFunction(float sum);

#endif /* perceptron_hpp */

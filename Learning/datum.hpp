//
//  datum.hpp
//  Seanet
//
//  Created by Douglas Corley on 6/1/16.
//
//

#ifndef datum_hpp
#define datum_hpp

#include "board.hpp"
#include "search.hpp"
#include <stdio.h>

class Datum {
public:
  State _state;
  Move _bestMove;
  int _contemptFactor;

  Datum(State state, Move bestMove, int contemptFactor);
};

bool isPseudoQuiet(State &state);

#endif /* datum_hpp */

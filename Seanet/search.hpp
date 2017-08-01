//
//  search.h
//  Seanet
//
//  Created by Nathaniel Corley on 5/16/16.
//
//

#ifndef __Seanet__search__
#define __Seanet__search__

#include "evaluator.hpp"
#include "hash.hpp"
#include "movegenerator.hpp"
#include "searchcontroller.hpp"
#include "util.hpp"
#include <stdio.h>
#include <sys/time.h>

void search(State &state, SearchController &sControl);
int negamax(int alpha, int beta, int depth, State &state,
            SearchController &sControl, S_PVLINE &pvLine);
int qSearch(int alpha, int beta, State &state, SearchController &sControl);

#endif /* defined(__Seanet__search__) */

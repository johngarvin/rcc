/*! \file
  
  \brief Description

  \authors Andy Stone (aistone@gmail.com)

  Copyright (c) 2007, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "Loop.hpp"
#include <assert.h>
#include <iostream>
using namespace std;

namespace OA {

Loop::Loop(OA_ptr<LoopIndex> loopIndex) {
    mLoopIndex = loopIndex;
}

StmtIterator Loop::getStmtIterator() {
    cerr << "TODO: Implementation" << endl;
    assert(false);
}

int Loop::getStmtOrder(StmtHandle stm1, StmtHandle stm2) {
    cerr << "TODO: Implementation" << endl;
    assert(false);
}

} // namespaces


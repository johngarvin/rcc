/*! \file
  
  \brief Implementation of loop abstraction.

  \authors Andy Stone (aistone@gmail.com)

  Copyright (c) 2007, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "LoopAbstraction.hpp"
#include <assert.h>
#include <iostream>
using namespace std;

namespace OA {
  namespace Loop {

int LoopAbstraction::sAbstractionsBuilt = 0;

LoopAbstraction::LoopAbstraction(
    OA_ptr<LoopIndex> loopIndex,
    StmtHandle loopStmt,
    StmtHandle endStmt,
    OA_ptr<LoopAbstraction> parent)
    :
    mLoopIndex(loopIndex),
    mLoopStatement(loopStmt),
    mEndStatement(endStmt),
    mParent(parent)
{
    mOrder = sAbstractionsBuilt++;
}

bool LoopAbstraction::operator<(LoopAbstraction &other) {
    return(mOrder < other.mOrder);
}

bool LoopAbstraction::operator==(LoopAbstraction &other) {
    return(mOrder == other.mOrder);
}

} } // namespaces


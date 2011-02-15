/*! \file
  
  \brief Implementation of LoopIndex class

  \authors Andy Stone (aistone@gmail.com)

  Copyright (c) 2007, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "LoopIndex.hpp"

namespace OA {
  namespace Loop {

LoopIndex::LoopIndex(
    OA_ptr<NamedLoc> variable,
    int initialBound,
    int terminalBound,
    int step)
{
    mVariable = variable;
    mInitialBound = initialBound;
    mTerminalBound = terminalBound;
    mStep = step;
}
    
// - [accessor methods] -
OA_ptr<NamedLoc> LoopIndex::getVariable() {
    return mVariable;
}

int LoopIndex::getInitialBound() {
    return mInitialBound;
}

int LoopIndex::getTerminalBound() {
    return mTerminalBound;
}

int LoopIndex::getStep() {
    return mStep;
}

} } // end namespaces


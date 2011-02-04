/*! \file
  
  \brief Abstraction to represent loop index variables and loop bounds.

  \authors Andy Stone (aistone@gmail.com)

  Copyright (c) 2007, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef LoopIndex_H
#define LoopIndex_H

#include <OpenAnalysis/Location/Locations.hpp>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>

namespace OA {
  namespace Loop {

//! abstracts a loop index variable
class LoopIndex : public virtual Annotation {
  public:
    LoopIndex(
        OA_ptr<NamedLoc> variable,
        int initialBound,
        int terminalBound,
        int step);
    
    // - [accessor methods] -
    OA_ptr<NamedLoc> getVariable();
    int getInitialBound();
    int getTerminalBound();
    int getStep();
    
    // - [output methods] - (derived from Annotation)
    virtual void output(IRHandlesIRInterface &ir);

  private:
  OUTPUT
    GENOUT  OA_ptr<NamedLoc> mVariable;
    GENOUT  int mInitialBound;
    GENOUT  int mTerminalBound;
    GENOUT  int mStep;
};

} } // end namespaces

#endif


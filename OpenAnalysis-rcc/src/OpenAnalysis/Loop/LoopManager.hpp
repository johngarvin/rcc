/*! \file
  
  \brief Manager to perform loop detection analysis

  \authors Andy Stone (aistone@gmail.com)

  Copyright (c) 2007, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef LoopManager_hpp
#define LoopManager_hpp

#include <OpenAnalysis/IRInterface/LoopIRInterface.hpp>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/AffineExpr/ManagerAffineExpr.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>
#include <list>
#include "LoopResults.hpp"
#include "LoopAbstraction.hpp"
using namespace std;

namespace OA {
  namespace Loop {

class LoopManager {
  public:
    /** Construct a new loop manager object given the passed IR-interface */
    LoopManager(OA_ptr<LoopIRInterface> _ir)
    {
        mIR = _ir;
    }

    /** Perform a loop detection analysis on the specified procedure */
    OA_ptr<LoopResults> performLoopDetection(ProcHandle proc);

  private:
    /** Gather statistics on the type of array accesses, and type of subscripts
        of these accesses, in the specified procedure */
    void gatherArrayAccessStatistics(ProcHandle proc);

    /** Given an array access analyze and gather statistics about it */
    void analyzeArrayAccess(OA_ptr<IdxExprAccess> access);

    OA_ptr<LoopIRInterface> mIR;
};

} } // end namespaces

#endif

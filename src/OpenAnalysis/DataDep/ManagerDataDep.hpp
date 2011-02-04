#ifndef ManagerDataDep_h
#define ManagerDataDep_h

/* Before DataDep is implemented what analyses are needed?
    - Loop Invariant detection
        - UD and DU Chains (how mature is this analysis?)
    - Induction variable detection
        - Induction variable tuples
            - Is this a safe assumption? One basic induction var for loop.

  Expansion of GCD:
    - Detect loops
*/

#include <OpenAnalysis/Loop/LoopAbstraction.hpp>
#include <OpenAnalysis/IRInterface/DataDepIRInterface.hpp>
#include "DataDepResults.hpp"
using namespace OA::Loop;

namespace OA {
namespace DataDep { 

//! Abstract base of data dep manager classes
class ManagerDataDep {
  public:
    ManagerDataDep(OA_ptr<DataDepIRInterface> _ir);

    /*! perform the data dependence analysis algorithm associated with the
        the given class on the specified procedure */
    virtual OA_ptr<DataDepResults> performAnalysis(
        ProcHandle p, OA_ptr<LoopAbstraction> loop) = 0;

  protected:
    OA_ptr<DataDepIRInterface> mIR;
};


    // needed for GCD:

    // getExprTree(ExprHandle h)
    // getConstValIntVal
    // getOpType

    // bool isInLoop(StmtHandle h, CFG cfg);
} } // namespaces

#endif

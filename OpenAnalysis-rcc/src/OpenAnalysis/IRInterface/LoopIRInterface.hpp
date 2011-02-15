#ifndef LoopIRInterface_hpp
#define LoopIRInterface_hpp

#include <list>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Loop/LoopAbstraction.hpp>
#include "AffineExprIRInterface.hpp"
using namespace std;

namespace OA {
  namespace Loop {

/** IR interface for loop detection analyses. */
class LoopIRInterface : public virtual IRHandlesIRInterface,
                        public virtual AffineExpr::AffineExprIRInterface
{
  public:
    LoopIRInterface() {}
    virtual ~LoopIRInterface() {}

    virtual OA_ptr<list<OA_ptr<LoopAbstraction> > >
        gatherLoops(const ProcHandle &proc) = 0;

    /*! given a statement determine the nearest loop which encloses it. */
    virtual StmtHandle findEnclosingLoop(const StmtHandle &stmt) = 0;

    virtual OA_ptr<IRStmtIterator> getStmtIterator(ProcHandle h) = 0;
    virtual OA_ptr<MemRefHandleIterator> getAllMemRefs(StmtHandle stmt) = 0;
    virtual OA_ptr<MemRefExprIterator>
        getMemRefExprIterator(MemRefHandle h) = 0;
};

} } // end namespaces

#endif


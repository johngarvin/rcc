/* Liveness.hpp
   NOTE: This file was automatically generated by DFAGen.  It is the interface
         for the Liveness analysis results.
*/

#ifndef Liveness_hpp
#define Liveness_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/IRInterface/auto_LivenessIRInterface.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>
#include <OpenAnalysis/Alias/AliasTag.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSetImpl.hpp>
#include <OpenAnalysis/DataFlow/DFSetBitImpl.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>
#include <OpenAnalysis/Utils/Util.hpp>

namespace OA {
  namespace Liveness {

typedef DataFlow::DataFlowSetImpl<Alias::AliasTag >  LivenessDFSet;

class Liveness : public virtual Annotation {
  public:
    //*****************************************************************
    // Construction methods
    //*****************************************************************
    Liveness(ProcHandle p, OA_ptr<LivenessIRInterface> _ir) {
        mIR = _ir;
        mExitLiveness = new LivenessDFSet();
    }
    ~Liveness() {}


    //*****************************************************************
    // Query methods
    //*****************************************************************
    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag > > getLivenessIterator(StmtHandle s) const;

    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag > > getExitLivenessIterator() const;

    OA_ptr<LivenessDFSet> getLivenessSet(StmtHandle s) {
        map<StmtHandle, OA_ptr<LivenessDFSet> >::iterator element =
            mLiveness.find(s);

        if(element == mLiveness.end()) {
            mLiveness[s] = new LivenessDFSet();
            element = mLiveness.find(s);
        }

        return (*element).second;
    }


    //*****************************************************************
    // Modifier methods
    //*****************************************************************
    void insert(StmtHandle s, Alias::AliasTag  val) {
        if(mLiveness[s].ptrEqual(0)) {
            mLiveness[s] = new LivenessDFSet();
        }
        mLiveness[s]->insert(val);
    }

    void insertExit(Alias::AliasTag  val) {
        mExitLiveness->insert(val);
    }


    //*****************************************************************
    // Output
    //*****************************************************************
    void output(IRHandlesIRInterface& pIR) const;

    void dump(std::ostream& os,
        OA_ptr<IRHandlesIRInterface> ir,
        Alias::Interface& aliasResults) const;



  private:
    map<StmtHandle, OA_ptr<LivenessDFSet> > mLiveness;
    OA_ptr<LivenessDFSet>  mExitLiveness;
    OA_ptr<LivenessIRInterface> mIR;
};

  } // end of Liveness namespace
} // end of OA namespace

#endif

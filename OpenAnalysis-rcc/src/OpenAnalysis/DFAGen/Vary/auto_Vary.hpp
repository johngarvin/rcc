/* Vary.hpp
   NOTE: This file was automatically generated by DFAGen.  It is the interface
         for the Vary analysis results.
*/

#ifndef Vary_hpp
#define Vary_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/IRInterface/auto_VaryIRInterface.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>
#include <OpenAnalysis/Alias/AliasTag.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSetImpl.hpp>
#include <OpenAnalysis/DataFlow/DFSetBitImpl.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>
#include <OpenAnalysis/Utils/Util.hpp>

namespace OA {
  namespace Vary {

typedef DataFlow::DataFlowSetImpl<Alias::AliasTag >  VaryDFSet;

class Vary : public virtual Annotation {
  public:
    //*****************************************************************
    // Construction methods
    //*****************************************************************
    Vary(ProcHandle p, OA_ptr<VaryIRInterface> _ir) {
        mIR = _ir;
        mExitVary = new VaryDFSet();
    }
    ~Vary() {}


    //*****************************************************************
    // Query methods
    //*****************************************************************
    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag > > getVaryIterator(StmtHandle s) const;

    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag > > getExitVaryIterator() const;

    OA_ptr<VaryDFSet> getVarySet(StmtHandle s) {
        map<StmtHandle, OA_ptr<VaryDFSet> >::iterator element =
            mVary.find(s);

        if(element == mVary.end()) {
            mVary[s] = new VaryDFSet();
            element = mVary.find(s);
        }

        return (*element).second;
    }


    //*****************************************************************
    // Modifier methods
    //*****************************************************************
    void insert(StmtHandle s, Alias::AliasTag  val) {
        if(mVary[s].ptrEqual(0)) {
            mVary[s] = new VaryDFSet();
        }
        mVary[s]->insert(val);
    }

    void insertExit(Alias::AliasTag  val) {
        mExitVary->insert(val);
    }


    //*****************************************************************
    // Output
    //*****************************************************************
    void output(IRHandlesIRInterface& pIR) const;

    void dump(std::ostream& os,
        OA_ptr<IRHandlesIRInterface> ir,
        Alias::Interface& aliasResults) const;



  private:
    map<StmtHandle, OA_ptr<VaryDFSet> > mVary;
    OA_ptr<VaryDFSet>  mExitVary;
    OA_ptr<VaryIRInterface> mIR;
};

  } // end of Liveness namespace
} // end of OA namespace

#endif

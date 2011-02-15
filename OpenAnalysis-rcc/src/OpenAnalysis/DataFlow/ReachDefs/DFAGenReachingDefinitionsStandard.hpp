
 // DFAGenReachingDefinitions.hpp

#ifndef DFAGenReachingDefinitionsStandard_hpp
#define LivenessStandard_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>
#include <OpenAnalysis/Location/Location.hpp>
#include <OpenAnalysis/DataFlow/LocDFSet.hpp>
#include <OpenAnalysis/Utils/Util.hpp>

namespace OA {
  namespace DFAGenReachingDefinitions {



class DFAGenReachingDefinitionsStandard : public virtual Annotation 
{
  public:
    DFAGenReachingDefinitionsStandard(ProcHandle p){} 
    ~DFAGenReachingDefinitionsStandard() {}

    OA_ptr<LocIterator> getDFAGenReachingDefinitionsIterator(StmtHandle s); 

    OA_ptr<LocIterator> getExitDFAGenReachingDefinitionsIterator(); 
// TODO
 //    Live will be a set of locations
    void insertDFAGenReachingDefinitions(StmtHandle s, OA_ptr<Location> Live)
    { 
      if(mDFAGenReachingDefinitions[s].ptrEqual(0)) {
        mDFAGenReachingDefinitions[s] = new LocSet;
      }
      mDFAGenReachingDefinitions[s]->insert(Live);    
    }

    void insertExitDFAGenReachingDefinitions(OA_ptr<Location> Live)
    { 
      mExitDFAGenReachingDefinitions->insert(Live); 
    }


    void output(IRHandlesIRInterface& pIR);

    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

  private:
    std::map<StmtHandle,OA_ptr<LocSet> > mDFAGenReachingDefinitions;
    OA_ptr<LocSet> mExitDFAGenReachingDefinitions;
}; 

  } // end of Liveness namespace
} // end of OA namespace

#endif

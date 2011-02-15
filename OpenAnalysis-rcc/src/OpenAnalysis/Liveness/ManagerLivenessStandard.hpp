// ManagerLivenessStandard.hpp

#ifndef LivenessManagerStandard_h
#define LivenessManagerStandard_h

//--------------------------------------------------------------------
// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/LivenessIRInterface.hpp>
#include <OpenAnalysis/Liveness/LivenessStandard.hpp>

#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/Alias/AliasTag.hpp>

#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectInterface.hpp>
#include <OpenAnalysis/DataFlow/CFGDFSolver.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>

namespace OA {
  namespace Liveness {

typedef DataFlow::DataFlowSetImpl<Alias::AliasTag> TagsDFSet;
typedef DataFlow::DataFlowSetImplIterator<Alias::AliasTag> TagsDFSetIter;

class ManagerLivenessStandard 
    : public DataFlow::CFGDFProblem { 
public:
  ManagerLivenessStandard(OA_ptr<LivenessIRInterface> _ir);
  ~ManagerLivenessStandard () {}

  OA_ptr<LivenessStandard> performAnalysis(ProcHandle, 
        OA_ptr<CFG::CFGInterface> cfg, OA_ptr<Alias::Interface> alias, 
        OA_ptr<SideEffect::InterSideEffectInterface> interSE,
        DataFlow::DFPImplement algorithm);

private:
  OA_ptr<DataFlow::DataFlowSet> initializeTop();
  OA_ptr<DataFlow::DataFlowSet> initializeBottom();

  OA_ptr<DataFlow::DataFlowSet>
           initializeNodeIN(OA_ptr<CFG::NodeInterface> n);
  
  OA_ptr<DataFlow::DataFlowSet>
           initializeNodeOUT(OA_ptr<CFG::NodeInterface> n);
                     

  OA_ptr<DataFlow::DataFlowSet> 
  meet (OA_ptr<DataFlow::DataFlowSet> set1, OA_ptr<DataFlow::DataFlowSet> set2); 

  OA_ptr<DataFlow::DataFlowSet> 
  transfer(OA_ptr<DataFlow::DataFlowSet> in, OA::StmtHandle stmt); 

private: // member variables

  OA_ptr<LivenessIRInterface> mIR;
  OA_ptr<Alias::Interface> mAlias;
  OA_ptr<LivenessStandard> mLiveMap;
  std::map<StmtHandle, OA_ptr<TagsDFSet> > mStmtMustDefMap; 
  std::map<StmtHandle, OA_ptr<TagsDFSet> > mStmtMayUseMap;
  OA_ptr<DataFlow::CFGDFSolver> mSolver;
    
};

  } // end of Liveness namespace
} // end of OA namespace

#endif

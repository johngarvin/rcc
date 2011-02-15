
// ManagerDFAGenReachingDefinitionsStandard.hpp

#ifndef DFAGenReachingDefinitionsManagerStandard_h
#define DFAGenReachingDefinitionsManagerStandard_h

//--------------------------------------------------------------------
// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/DFAGenReachingDefinitionsIRInterface.hpp>
#include <OpenAnalysis/DFAGenReachingDefinitions/DFAGenReachingDefinitionsStandard.hpp>


#include <OpenAnalysis/Alias/Interface.hpp>

#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/Location/Location.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectInterface.hpp>
#include <OpenAnalysis/DataFlow/CFGDFSolver.hpp>

#include <OpenAnalysis/CFG/CFG.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>

namespace OA {
  namespace DFAGenReachingDefinitions {


class ManagerDFAGenReachingDefinitionsStandard 
    : public virtual DataFlow::CFGDFProblem { 
public:
  ManagerDFAGenReachingDefinitionsStandard(OA_ptr<DFAGenReachingDefinitionsIRInterface> _ir);
  ~ManagerDFAGenReachingDefinitionsStandard () {}

  OA_ptr<DFAGenReachingDefinitionsStandard> performAnalysis(ProcHandle, 
        OA_ptr<CFG::CFGInterface> cfg, OA_ptr<Alias::Interface> alias, 
        OA_ptr<SideEffect::InterSideEffectInterface> interSE);

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
  transfer(OA_ptr<DataFlow::DataFlowSet> in, OA::StmtHandle Stmt); 

private: // member variables

  OA_ptr<DFAGenReachingDefinitionsIRInterface> mIR;
  OA_ptr<Alias::Interface> mAlias;
  OA_ptr<DFAGenReachingDefinitionsStandard> mDFAGenReachingDefinitionsMap;
  std::map<StmtHandle,std::set<OA_ptr<Location> > mStmtMustDefMap;
  std::map<StmtHandle,std::set<OA_ptr<Location> > mStmtMustUseMap;
  std::map<StmtHandle,std::set<OA_ptr<Location> > mStmtMayDefMap;
  std::map<StmtHandle,std::set<OA_ptr<Location> > mStmtMayUseMap;
  OA_ptr<DataFlow::CFGDFSolver> mSolver;
    
};

  } // end of DFAGenReachingDefinitions namespace
} // end of OA namespace

#endif

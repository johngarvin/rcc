

#ifndef ManagerReachConsts_h
#define ManagerReachConsts_h

#include <OpenAnalysis/ReachConsts/ReachConsts.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSetImpl.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>
#include <OpenAnalysis/DataFlow/CFGDFSolver.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/ExprTree/EvalToConstVisitor.hpp>
#include <OpenAnalysis/Alias/AliasTagSet.hpp>

namespace OA {
  namespace ReachConsts {


class ManagerReachConsts : private DataFlow::CFGDFProblem 
{
 
public:
  ManagerReachConsts(OA_ptr<ReachConstsIRInterface> _ir);
  ~ManagerReachConsts() {}


  OA_ptr<ReachConstsStandard> performAnalysis(ProcHandle,
        OA_ptr<CFG::CFGInterface> cfg, OA_ptr<Alias::Interface> alias,
        DataFlow::DFPImplement algorithm);


  //! DataFlow Functions

  OA_ptr<DataFlow::DataFlowSet>  initializeTop();
  OA_ptr<DataFlow::DataFlowSet>  initializeBottom();
  
  //! Should generate an in and out DataFlowSet for node
  OA_ptr<DataFlow::DataFlowSet>
            initializeNodeIN(OA_ptr<CFG::NodeInterface> n);
  OA_ptr<DataFlow::DataFlowSet>
            initializeNodeOUT(OA_ptr<CFG::NodeInterface> n);

  OA_ptr<DataFlow::DataFlowSet>
  meet (OA_ptr<DataFlow::DataFlowSet> set1,
        OA_ptr<DataFlow::DataFlowSet> set2);

  OA_ptr<DataFlow::DataFlowSet>
  transfer(OA_ptr<DataFlow::DataFlowSet> in, OA::StmtHandle stmt);


private: // member variables

  OA_ptr<ReachConstsIRInterface> mIR;
  ProcHandle mProc;
  OA_ptr<Alias::Interface> mAlias;
  OA_ptr<CFG::CFGInterface> mCFG;
  OA_ptr<ReachConstsStandard> mRCS;
  OA_ptr<DataFlow::CFGDFSolver> mSolver;
  OA_ptr<RCPairSet> mAllTop;
};

 } // end of ReachConsts namespace
} // end of OA namespace

#endif



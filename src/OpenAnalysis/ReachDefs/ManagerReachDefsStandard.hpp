/*! \file
  
  \brief Declarations of the AnnotationManager that generates a ReachDefsStandard

  \authors Michelle Strout
  \version $Id: ManagerReachDefsStandard.hpp,v 1.15 2005/03/18 18:14:16 ntallent Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ReachDefsManagerStandard_h
#define ReachDefsManagerStandard_h

//--------------------------------------------------------------------
// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/ReachDefsIRInterface.hpp>
#include <OpenAnalysis/ReachDefs/ReachDefsStandard.hpp>

//#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>

#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectInterface.hpp>
#include <OpenAnalysis/DataFlow/CFGDFSolver.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>

namespace OA {
  namespace ReachDefs {

typedef DataFlow::IRHandleDataFlowSet<StmtHandle> StmtDFSet;
typedef DataFlow::DataFlowSetImpl<Alias::AliasTag> TagsDFSet;

/*! 
   The AnnotationManager for ReachDefsStandard.
   This class can build an ReachDefsStandard, 
   (eventually) read one in from a file, and write one out to a file.
*/
class ManagerReachDefsStandard
    : public virtual DataFlow::CFGDFProblem {
      //??? eventually public OA::AnnotationManager
public:
  ManagerReachDefsStandard(OA_ptr<ReachDefsIRInterface> _ir);
  //{ CFGDFProblem<IRHandleDataFlowSet<StmtHandle> >( DataFlow::Forward ); } 
  //{ CFGDFProblem( DataFlow::Forward ); } 
  ~ManagerReachDefsStandard () {}

  //! Used to perform analysis when not using AQM
  OA_ptr<ReachDefsStandard> performAnalysis(ProcHandle, 
        OA_ptr<CFG::CFGInterface> cfg, OA_ptr<Alias::Interface> alias, 
        OA_ptr<SideEffect::InterSideEffectInterface> interSE,
        DataFlow::DFPImplement algorithm);

  //! this method will be used when the AQM is working because the CFG
  //! and MemRefExpr information will be queried from AnnotationQueryManager
  //virtual Alias::AliasMap* performAnalysis(ProcHandle);

  //------------------------------------------------------------------
  // Implementing the callbacks for CFGDFProblem
  //------------------------------------------------------------------
protected:
  OA_ptr<DataFlow::DataFlowSet> initializeTop();
  OA_ptr<DataFlow::DataFlowSet> initializeBottom();

  //void initializeNode(OA_ptr<CFG::Interface::Node> n);
  // Added by PLM 07/26/06
  //! Should generate an in and out DataFlowSet for node
  OA_ptr<DataFlow::DataFlowSet>
           initializeNodeIN(OA_ptr<CFG::NodeInterface> n);
  
  OA_ptr<DataFlow::DataFlowSet>
           initializeNodeOUT(OA_ptr<CFG::NodeInterface> n);
                     

  OA_ptr<DataFlow::DataFlowSet> 
  meet (OA_ptr<DataFlow::DataFlowSet> set1, OA_ptr<DataFlow::DataFlowSet> set2); 

  OA_ptr<DataFlow::DataFlowSet> 
  transfer(OA_ptr<DataFlow::DataFlowSet> in, OA::StmtHandle stmt); 

protected: // member variables

  OA_ptr<ReachDefsIRInterface> mIR;
  OA_ptr<Alias::Interface> mAlias;
  OA_ptr<ReachDefsStandard> mReachDefMap;

  // all memrefs that a statement may and must define
  std::map<StmtHandle, TagsDFSet> mStmtMayDefMap;
  std::map<StmtHandle, TagsDFSet> mStmtMustDefMap;
 // Added by PLM 07/26/06
  OA_ptr<DataFlow::CFGDFSolver> mSolver;
    
};

  } // end of ReachDefs namespace
} // end of OA namespace

#endif

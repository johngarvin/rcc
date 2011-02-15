/*! \file
  
  \brief The AnnotationManager that generates a LinearityMatrixStandard.

  \authors Luis Ramos
  \version $Id: ManagerLinearityStandard.hpp

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

//--------------------------------------------------------------------
//--------------------------------------------------------------------

#ifndef _MANAGERLINEARITYMATRIX_H
#define _MANAGERLINEARITYMATRIX_H

//--------------------------------------------------------------------
// STL headers
#include <iostream>
#include <map>
#include <set>
#include <list>

// Local headers
#include "LinearityPair.hpp"
#include "LinearityDepsSet.hpp"
#include "LinearityMatrixStandard.hpp"

//OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Location/Locations.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/IRInterface/LinearityIRInterface.hpp>

#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/DataFlow/ParamBindings.hpp>

#include <OpenAnalysis/ExprTree/LinearityLocsVisitor.hpp>

#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectInterface.hpp>
#include <OpenAnalysis/DataFlow/CFGDFSolver.hpp>

//! Namespace for the whole OpenAnalysis Toolkit
namespace OA {
  namespace Linearity {

class ManagerLinearity : public virtual DataFlow::CFGDFProblem {
public:
  ManagerLinearity(OA_ptr<LinearityIRInterface> _ir);
  virtual ~ManagerLinearity() {}

  virtual OA_ptr<LinearityMatrix>
        performAnalysis(ProcHandle proc,
                         OA_ptr<CFG::CFGInterface> cfg,
                         OA_ptr<Alias::Interface> alias,
                         OA_ptr<DataFlow::ParamBindings> paramBind,
                         DataFlow::DFPImplement algorithm);
  virtual OA_ptr<LinearityMatrix>
        performAnalysis2(SymHandle);
  virtual OA_ptr<LinearityMatrix>
        performAnalysis3(ProcHandle proc,
                         OA_ptr<CFG::CFGInterface> cfg,
                         OA_ptr<Alias::Interface> alias,
                         OA_ptr<DataFlow::ParamBindings> paramBind);

  //------------------------------------------------------------------
  // Implementing the callbacks for CFGDFProblem
  //------------------------------------------------------------------
private:
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

//public:
//  OA_ptr<IRHandlesIRInterface> getIR() {
//    return mIR;
//  }

  
private:
  OA_ptr<LinearityIRInterface> mIR; 
  OA_ptr<LinearityMatrix> mLM;
  ProcHandle mProc;
  OA_ptr<CFG::CFGInterface> mCFG;
  OA_ptr<Alias::Interface> mAlias;
  OA_ptr<DataFlow::ParamBindings> mParamBind;
  OA_ptr<DataFlow::CFGDFSolver> mSolver;
  OA_ptr<CFG::NodeInterface> mExitNode;

};

  } // end of Linearity namespace
} // end of OA namespace

#endif


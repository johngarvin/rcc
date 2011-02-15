/*! \file
  
  \brief Declarations of the AnnotationManager that generates a DepStandard

  \authors Michelle Strout
  \version $Id: ManagerDepStandard.hpp,v 1.5 2005/06/10 02:32:02 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ManagerDepStandard_H
#define ManagerDepStandard_H

//--------------------------------------------------------------------
// OpenAnalysis headers
#include "DepStandard.hpp"
#include "InterDep.hpp"
#include "DepDFSet.hpp"
#include <OpenAnalysis/IRInterface/ActivityIRInterface.hpp>
#include <OpenAnalysis/ExprTree/DifferentiableLocsVisitor.hpp>
#include <OpenAnalysis/ExprTree/EvalToMemRefVisitor.hpp>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/LocDFSet.hpp>
#include <OpenAnalysis/DataFlow/ParamBindings.hpp>
#include <OpenAnalysis/DataFlow/CFGDFSolver.hpp>

namespace OA {
  namespace Activity {

/*! 
   The AnnotationManager for a DepStandard.
   This class can build a DepStandard, (eventually) read one in from a file, 
   and write one out to a file.
*/
class ManagerDepStandard 
    : private DataFlow::CFGDFProblem { 
public:
  ManagerDepStandard(OA_ptr<ActivityIRInterface> _ir);
  ~ManagerDepStandard () {}

  //! interDep is used to get Dep results from procedures we call
  //! FIXME: should use side-effect results as backup?
  OA_ptr<DepStandard> performAnalysis(ProcHandle, 
        OA_ptr<Alias::Interface> alias,
        OA_ptr<CFG::CFGInterface> cfg, OA_ptr<InterDep> interDep,
        OA_ptr<DataFlow::ParamBindings> paramBind,
        DataFlow::DFPImplement algorithm);

  //------------------------------------------------------------------
  // Implementing the callbacks for CFGDFProblem
  //------------------------------------------------------------------
private:
  OA_ptr<DataFlow::DataFlowSet> initializeTop();
  OA_ptr<DataFlow::DataFlowSet> initializeBottom();

 // void initializeNode(OA_ptr<CFG::Interface::Node> n);

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

  
private: // member variables

  OA_ptr<ActivityIRInterface> mIR;
  OA_ptr<DepStandard> mDep;
  OA_ptr<Alias::Interface> mAlias;
  OA_ptr<InterDep> mInterDep;
  OA_ptr<CFG::CFGInterface> mCFG;
  OA_ptr<DataFlow::ParamBindings> mParamBind;
  // Added by PLM 07/26/06
  OA_ptr<DataFlow::CFGDFSolver> mSolver;
  
};

  } // end of Activity namespace
} // end of OA namespace

#endif

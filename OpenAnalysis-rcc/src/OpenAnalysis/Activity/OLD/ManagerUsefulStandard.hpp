/*! \file
  
  \brief Declarations of the AnnotationManager that generates a UsefulStandard

  \authors Michelle Strout
  \version $Id: ManagerUsefulStandard.hpp,v 1.3 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef UsefulManagerStandard_h
#define UsefulManagerStandard_h

//--------------------------------------------------------------------
// OpenAnalysis headers
#include "ICFGDep.hpp"
#include "UsefulStandard.hpp"

#include <OpenAnalysis/IRInterface/ActivityIRInterface.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/CFGDFSolver.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSetImpl.hpp>

namespace OA {
  namespace Activity {


/*! 
   The AnnotationManager for UsefulStandard.
   This class can build an UsefulStandard, 
   (eventually) read one in from a file, and write one out to a file.
*/
class ManagerUsefulStandard 
    : private DataFlow::CFGDFProblem { 
      //??? eventually public OA::AnnotationManager
public:
  ManagerUsefulStandard(OA_ptr<ActivityIRInterface> _ir);
  ~ManagerUsefulStandard () {}

  //! depLocIter is a location iterator over dependent locations
  //! that the algorithm should calculate InUseful from
  OA_ptr<UsefulStandard> performAnalysis(ProcHandle, 
        OA_ptr<CFG::CFGInterface> cfg, OA_ptr<ICFGDep> dep,
        OA_ptr<DataFlow::DataFlowSetImpl<int> > depTagSet,
        DataFlow::DFPImplement type);

  //------------------------------------------------------------------
  // Implementing the callbacks for CFGDFProblem
  //------------------------------------------------------------------
private:
  OA_ptr<DataFlow::DataFlowSet> initializeTop();
  OA_ptr<DataFlow::DataFlowSet> initializeBottom();

//  void initializeNode(OA_ptr<CFG::Interface::Node> n);

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
  OA_ptr<ICFGDep> mDep;
  OA_ptr<UsefulStandard> mUsefulMap;
  OA_ptr<DataFlow::DataFlowSetImplIterator<int> > mDepTagIter;
  OA_ptr<CFG::CFGInterface> mCFG;
  // Added by PLM 07/26/06
  OA_ptr<DataFlow::CFGDFSolver> mSolver;
  
};

  } // end of Activity namespace
} // end of OA namespace

#endif

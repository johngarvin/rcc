/*! \file
  
  \brief Abstract class for solving dataflow analysis problems on CFGs.

  \authors Michelle Strout (April 2004)
           similar to John Mellor-Crummey's CallGraphDFProblem.h
  \version $Id: CFGDFProblem.hpp,v 1.17 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  To use this for performing an intra-procedural dataflow analysis
  problem, privately inherit from the CFGDFProblem class and
  implement the callbacks.
 
  Modified by Priyadarshini Malusare 07/26/2006 
*/

#ifndef CFGDFProblem_h
#define CFGDFProblem_h


#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>


//#include <OpenAnalysis/DataFlow/DGraphIterativeDFP.hpp>
#include <OpenAnalysis/DataFlow/DGraphSolverDFP.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/IRInterface/CFGIRInterface.hpp>
#include <map>

namespace OA {
  namespace DataFlow {

  
//*********************************************************************
// class CFGDFProblem
//*********************************************************************
class CFGDFProblem 
{

public:
  //--------------------------------------------------------
  // constructor/destructor
  //--------------------------------------------------------
  CFGDFProblem() { }
  virtual ~CFGDFProblem() {}

  //--------------------------------------------------------
  // initialization callbacks
  //--------------------------------------------------------

  //! Return an initialized top set
  virtual OA_ptr<DataFlowSet>  initializeTop() = 0;

  //! Return an initialized bottom set
  virtual OA_ptr<DataFlowSet>  initializeBottom() = 0;

  //! Should generate an in and out DataFlowSet for node
  //! and store these in mNodeInSetMap and mNodeOutSetMap.
  //virtual void initializeNode(OA_ptr<CFG::Interface::Node> n)  = 0;
  //virtual void initializeEdge(DGraph::Interface::Edge *e)  = 0;

  
   //! Should generate an in and out DataFlowSet for node
   virtual OA_ptr<DataFlowSet>
         initializeNodeIN(OA_ptr<CFG::NodeInterface> n)  = 0;
   virtual OA_ptr<DataFlowSet>
         initializeNodeOUT(OA_ptr<CFG::NodeInterface> n)  = 0;

  
  //--------------------------------------------------------
  // solver callbacks 
  //--------------------------------------------------------
  
  //! OK to modify set1 and return it as result, because solver
  //! only passes a tempSet in as set1
  virtual OA_ptr<DataFlowSet> meet(OA_ptr<DataFlowSet> set1, 
                                   OA_ptr<DataFlowSet> set2) = 0; 

  //! OK to modify in set and return it again as result because
  //! solver clones the BB in sets
  virtual OA_ptr<DataFlowSet> transfer(OA_ptr<DataFlowSet> in, 
                                       OA::StmtHandle stmt) = 0;

};

  } // end of DataFlow namespace
}  // end of OA namespace

#endif

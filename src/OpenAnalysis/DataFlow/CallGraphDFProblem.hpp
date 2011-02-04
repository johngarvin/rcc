/*! \file
  
  \brief Class for solving dataflow analysis problems on CallGraphs in a flow
         and context insensitive fashion.

  \authors Michelle Strout (Jan 2005)
           similar to John Mellor-Crummey's CallGraphFlowInsensitiveDFProblem.h
  \version $Id: CallGraphDFProblemNew.hpp,v 1.3 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef CallGraphDFProblem_h
#define CallGraphDFProblem_h

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>
#include <OpenAnalysis/CallGraph/CallGraphInterface.hpp>

namespace OA {
  namespace DataFlow {


/*! Interface for a class that performs data-flow analysis on a call
    graph.
    
    To use this for performing interprocedural dataflow analysis, privately
    inherit from the CallGraphDFProblemNew class and implement the callbacks.

    It is flow insensitive because for top down analyses it uses the analysis
    results for the whole caller at each of the caller's callsites.  It is
    context-insensitive because it meets all the data flow information for
    callers when working top-down and callees when working bottom-up.
*/
class CallGraphDFProblem {
public:
  //--------------------------------------------------------
  // constructor/destructor
  //--------------------------------------------------------
  /*! Destructor (virtual since we want base-type pointers that actually point
      to objects of derived types to call the destructor of the derived
      version. This is a safety precaution.) */
  virtual ~CallGraphDFProblem() {}

  //--------------------------------------------------------
  // initialization callbacks
  //--------------------------------------------------------
  //! Return an initialized top set
  virtual OA_ptr<DataFlowSet>  initializeTop() = 0;

  //! Return an initialized bottom set
  virtual OA_ptr<DataFlowSet>  initializeBottom() = 0;

  //! Should generate an in and out DataFlowSet for node
  virtual OA_ptr<DataFlowSet>
      initializeNodeIN(OA_ptr<CallGraph::NodeInterface> n)  = 0;
  virtual OA_ptr<DataFlowSet>
      initializeNodeOUT(OA_ptr<CallGraph::NodeInterface> n)  = 0;

  //! Should generate an initial DataFlowSet for a procedure
  //! Nodes that we don't have procedure definition for will get ProcHandle(0)
  virtual OA_ptr<DataFlowSet> initializeNode(ProcHandle proc) = 0;
  
  //! Should generate an initial DataFlowSet, use if for a call if both caller
  //! and callee are defined
  virtual OA_ptr<DataFlowSet> initializeEdge(CallHandle call,
                                             ProcHandle caller,
                                             ProcHandle callee) = 0;
  //! Should generate an initial DataFlowSet for a call,
  //! called when callee is not defined in call graph and therefore
  //! doesn't have a procedure definition handle
  virtual OA_ptr<DataFlowSet> initializeEdge(CallHandle call,
                                             ProcHandle caller,
                                             SymHandle callee) = 0;

  //--------------------------------------------------------
  // solver callbacks
  //--------------------------------------------------------

  //! OK to modify set1 and return it as result, because solver
  //! only passes a tempSet in as set1
  virtual OA_ptr<DataFlowSet>  meet(
    OA_ptr<DataFlowSet> set1,
    OA_ptr<DataFlowSet> set2)  = 0;

  //! What the analysis does for the particular procedure
  virtual OA_ptr<DataFlowSet>  atCallGraphNode(
    OA_ptr<DataFlowSet> inSet,
    OA::ProcHandle proc) = 0;
  
  //! What the analysis does for a particular call
  //! use if both caller and callee are defined
  virtual OA_ptr<DataFlowSet>  atCallGraphEdge(
    OA_ptr<DataFlowSet> inSet,
    CallHandle call,
    ProcHandle caller,
    ProcHandle callee) = 0;

  //! use if callee is not defined in the call graph
  virtual OA_ptr<DataFlowSet>  atCallGraphEdge(
    OA_ptr<DataFlowSet> inSet,
    CallHandle call,
    ProcHandle caller,
    SymHandle callee) = 0;
  
  //! translate results from procedure node to callee edge if top-down
  //! or to caller edge if bottom-up using data-flow set at procedure node
  //! the proc could be ProcHandle(0) if the called procedure has not
  //! been defined
  virtual OA_ptr<DataFlowSet>  nodeToEdge(
    ProcHandle proc,
    OA_ptr<DataFlow::DataFlowSet> procDFSet,
    CallHandle call) = 0;
  
  //! translate results from caller edge to procedure node if top-down
  //! or from callee edge if bottom-up using call data flow set
  virtual OA_ptr<DataFlowSet>  edgeToNode(
    CallHandle call,
    OA_ptr<DataFlow::DataFlowSet> callDFSet,
    ProcHandle proc) = 0;
};


  } // end of DataFlow namespace
}  // end of OA namespace

#endif

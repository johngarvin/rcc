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


  To use this for performing interprocedural dataflow analysis,
  privately inherit from the CallGraphDFProblemNew class and
  implement the callbacks.
  It is flow insensitive because for top down analyses it uses the analysis
  results for the whole caller at each of the caller's callsites.
  It is context-insensitive because it meets all the data flow information
  for callers when working top-down and callees when working bottom-up.
*/

#ifndef CallGraphDFSolver_h
#define CallGraphDFSolver_h

#include <OpenAnalysis/Utils/OA_ptr.hpp>
//#include <OpenAnalysis/DataFlow/DGraphIterativeDFP.hpp>
#include <OpenAnalysis/DataFlow/DGraphSolverDFP.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>
#include <OpenAnalysis/DataFlow/CallGraphDFProblem.hpp>
#include <OpenAnalysis/CallGraph/CallGraphInterface.hpp>
#include <map>

namespace OA {
  namespace DataFlow {

  
//*********************************************************************
// class CallGraphDFProblemNew
//*********************************************************************
class CallGraphDFSolver  : private DataFlow::DGraphSolverDFP {
public:
  typedef enum { TopDown, BottomUp } CallGraphDirectionType;
private:
  OA_ptr<DataFlowSet> mTop;
//  OA_ptr<DataFlowSet> mBottom;
  CallGraphDirectionType mDirection;

public:
  //--------------------------------------------------------
  // constructor/destructor
  //--------------------------------------------------------
  CallGraphDFSolver(CallGraphDirectionType pDirection, CallGraphDFProblem& prob);
  virtual ~CallGraphDFSolver() {}

  //! solves data-flow problem, after done in and out nodes will
  //! have correct data-flow values
  void solve(OA_ptr<CallGraph::CallGraphInterface> callGraph, 
                    DFPImplement algorithm);

  OA_ptr<DataFlowSet> getInSet(OA_ptr<CallGraph::NodeInterface> n);
  OA_ptr<DataFlowSet> getOutSet(OA_ptr<CallGraph::NodeInterface> n);  

  //========================================================
  // implementation of DGraphIterativeDFP callbacks
  // These are implemented in CallGraphDFProblem.cpp for a 
  // CallGraph.
  //========================================================
  
  //--------------------------------------------------------
  // initialization upcall 
  //--------------------------------------------------------
  void initialize(OA_ptr<DGraph::DGraphInterface> dg);
  
  //--------------------------------------------------------
  // solver upcalls
  //--------------------------------------------------------
  bool atDGraphNode(OA_ptr<DGraph::NodeInterface> node, 
                    DGraph::DGraphEdgeDirection pOrient);
  bool atDGraphEdge(OA_ptr<DGraph::EdgeInterface>, 
                    DGraph::DGraphEdgeDirection);
  
  //--------------------------------------------------------
  // finalization upcalls
  //--------------------------------------------------------
  void finalizeNode(OA_ptr<DGraph::NodeInterface> node);
  void finalizeEdge(OA_ptr<DGraph::EdgeInterface> edge);


  //--------------------------------------------------------
  // debugging
  //--------------------------------------------------------
public:
  void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);


private:
  // In and Out in this context refer to the data-flow direction
  // which in the case of bottom-up is in the reverse direction
  // of edges in the graph
  std::map<OA_ptr<CallGraph::NodeInterface>,OA_ptr<DataFlowSet> > 
      mNodeOutSetMap;
  std::map<OA_ptr<CallGraph::NodeInterface>,OA_ptr<DataFlowSet> > 
      mNodeInSetMap;
  std::map<OA_ptr<CallGraph::EdgeInterface>,OA_ptr<DataFlowSet> > 
      mEdgeOutSetMap;
  std::map<OA_ptr<CallGraph::EdgeInterface>,OA_ptr<DataFlowSet> > 
      mEdgeInSetMap;

  // whether or not the node has had the transfer function applied
  // to all statements at least once
  std::map<OA_ptr<CallGraph::NodeInterface>,bool> mNodeInitTransApp;

  CallGraphDFProblem& mDFProb;
};

  } // end of DataFlow namespace
}  // end of OA namespace

#endif

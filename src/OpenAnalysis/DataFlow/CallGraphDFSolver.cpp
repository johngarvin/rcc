/*! \file
  
  \brief Definition of a dataflow algorithm for CallGraphs

  \authors Michelle Strout (Nov 2004)
           adapted from John Mellor-Crummey's dsystem 
           CallGraphFlowInsensitiveDFProblem
  \version $Id: CallGraphDFSolver.cpp,v 1.3 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "CallGraphDFSolver.hpp"
#include <Utils/Util.hpp>

namespace OA {
  namespace DataFlow {

static bool debug = false;


CallGraphDFSolver::CallGraphDFSolver(CallGraphDirectionType pDirection, CallGraphDFProblem& prob) 
      : mDirection(pDirection), mDFProb(prob)
{
  OA_DEBUG_CTRL_MACRO("DEBUG_CallGraphDFSolver:ALL", debug);
}


void 
CallGraphDFSolver::solve(OA_ptr<CallGraph::CallGraphInterface> callGraph, 
                         DFPImplement algorithm)
{
  if (debug) {
      /*
    std::cout << "CallGraphDFProblem::solve: ";
    std::cout << "callgraph = " << std::endl;
    callGraph->dump(std::cout,mIR);
    */
  }

  mTop = mDFProb.initializeTop();

  DataFlow::DGraphSolverDFP::solve(callGraph, 
          ((mDirection==TopDown) ? DGraph::DEdgeOrg : DGraph::DEdgeRev),
          algorithm);
}

OA_ptr<DataFlowSet> CallGraphDFSolver::getInSet(OA_ptr<CallGraph::NodeInterface> n)
{
  return mNodeInSetMap[n];
}

OA_ptr<DataFlowSet> CallGraphDFSolver::getOutSet(OA_ptr<CallGraph::NodeInterface> n)
{
  return mNodeOutSetMap[n];
}

//========================================================
// implementation of DGraphIterativeDFP callbacks
//========================================================
  
//--------------------------------------------------------
// initialization upcall 
//--------------------------------------------------------
void CallGraphDFSolver::initialize(OA_ptr<DGraph::DGraphInterface> dg)
{
    OA_ptr<CallGraph::CallGraphInterface> callGraph = dg.convert<CallGraph::CallGraphInterface>();

    // iterate over all nodes and call initialization routine
    // that sets up initial DataFlowSets so we know if things have changed
    OA_ptr<CallGraph::NodesIteratorInterface> nodeIterPtr;
    nodeIterPtr = callGraph->getCallGraphNodesIterator();

    for ( ;nodeIterPtr->isValid(); ++(*nodeIterPtr) ) {
        OA_ptr<CallGraph::NodeInterface> node = nodeIterPtr->currentCallGraphNode();
        
        // In and Out in this context refer to the data-flow direction
        // which in the case of bottom-up is in the reverse direction
        // of edges in the graph
        mNodeInSetMap[node] = mDFProb.initializeTop();
        mNodeOutSetMap[node] = mDFProb.initializeNode(node->getProc());
    }

    // iterate over all edges and call initialization routine
    // that sets up initial DataFlowSets so we know if things have changed
    
    OA_ptr<CallGraph::EdgesIteratorInterface> edgeIterPtr;
    edgeIterPtr = callGraph->getCallGraphEdgesIterator();
    for ( ;edgeIterPtr->isValid(); ++(*edgeIterPtr) ) {
        OA_ptr<CallGraph::EdgeInterface> edge = edgeIterPtr->currentCallGraphEdge();
        
        // In is in direction of data-flow
        mEdgeInSetMap[edge] = mDFProb.initializeTop();

        // get procedure handle for caller
        OA_ptr<CallGraph::NodeInterface> caller = edge->getCallGraphSource();
        
        // get procedure handle for callee if there is one
        OA_ptr<CallGraph::NodeInterface> callee = edge->getCallGraphSink();
        
        if (callee->isDefined()) {
            mEdgeOutSetMap[edge] = mDFProb.initializeEdge(edge->getCallHandle(),
                                                  caller->getProc(),
                                                  callee->getProc());
        } else {
            mEdgeOutSetMap[edge] = mDFProb.initializeEdge(edge->getCallHandle(),
                                                  caller->getProc(),
                                                  callee->getProcSym());
        }
    }

}

//--------------------------------------------------------
// solver upcalls
//--------------------------------------------------------

bool CallGraphDFSolver::atDGraphEdge( OA_ptr<DGraph::EdgeInterface> pEdge, 
                                       DGraph::DGraphEdgeDirection pOrient)
{
    if (debug) {
      std::cout << "CallGraphDFSolver::atDGraphEdge" << std::endl;
    }

    // cast edge to a CallGraph edge
    OA_ptr<CallGraph::EdgeInterface> edge 
        = pEdge.convert<CallGraph::EdgeInterface>();

    // get procedure handle for caller
    OA_ptr<CallGraph::NodeInterface> caller = edge->getCallGraphSource();

    // get procedure handle for callee if there is one
    OA_ptr<CallGraph::NodeInterface> callee = edge->getCallGraphSink();

    // get data-flow set for edge based on procedure
    // whether getting it from caller or callee depends on 
    // the direction of the analysis
    OA_ptr<DataFlowSet> edgeIn;
    if (pOrient==DGraph::DEdgeOrg) { // top down
      edgeIn = mDFProb.nodeToEdge(caller->getProc(), mNodeOutSetMap[caller],
                          edge->getCallHandle());
    } else { // bottom up
      edgeIn = mDFProb.nodeToEdge(callee->getProc(), mNodeOutSetMap[callee],
                          edge->getCallHandle());
    }

    // do computation at actual call
    OA_ptr<DataFlowSet> edgeOut;
    if (callee->isDefined()) {
      edgeOut = mDFProb.atCallGraphEdge(edgeIn, edge->getCallHandle(),
					caller->getProc(), callee->getProc());
    } else {
      edgeOut = mDFProb.atCallGraphEdge(edgeIn, edge->getCallHandle(),
					caller->getProc(), callee->getProcSym());
    }
    mEdgeInSetMap[edge] = edgeIn;

    bool changed = false;
    // check if a change occured
    if (edgeOut != mEdgeOutSetMap[edge]) {
      if (debug) { 
          std::cout << "%%% CallGraphDFSolver::atDGraphEdge, "
                    << "mEdgeOutSetMap != edgeOut" << std::endl;
      }
      mEdgeOutSetMap[edge] = edgeOut;
      changed = true;
    } 
     
    return changed;
}

/*!
   NOTE: The CallGraphDFSolver code treats nodeIn, nodeOut, edgeIn, and
         edgeOut differently than the other data-flow frameworks.
         FIXME: to make maintenance easier they should all use similar logic
*/

bool CallGraphDFSolver::atDGraphNode( 
                                       OA_ptr<DGraph::NodeInterface> pNode, 
                                       DGraph::DGraphEdgeDirection pOrient)
{
    OA_ptr<CallGraph::NodeInterface> node 
        = pNode.convert<OA::CallGraph::NodeInterface>();

    if (debug) {
        std::cout << "CallGraphDFSolver::atDGraphNode: CallGraph node = ";
        std::cout << node->getId() << std::endl;
    }

    //-----------------------------------------------------
    // for each incoming edge translate edgeOut to nodeIn
    // and then do meet of all nodeIns with each other and
    // current nodeIn 
    //-----------------------------------------------------
    OA_ptr<DataFlowSet> nodeInSet = mNodeInSetMap[node]->clone();

    // set up iterator for predecessor nodes
    OA_ptr<CallGraph::EdgesIteratorInterface>  it;
    if (pOrient==DGraph::DEdgeOrg) {
       it = node->getCallGraphIncomingEdgesIterator(); // stupid Sun compiler
    } else {
       it = node->getCallGraphOutgoingEdgesIterator(); // stupid Sun compiler
    }
    // iterate over predecessors, translate from edge to node, 
    // and do meet operation
    for (; it->isValid(); ++(*it)) {
      OA_ptr<CallGraph::EdgeInterface> edge = it->currentCallGraphEdge();
      OA_ptr<DataFlowSet> temp  = mDFProb.edgeToNode(edge->getCallHandle(),
 					              mEdgeOutSetMap[edge], node->getProc());
      nodeInSet = mDFProb.meet(nodeInSet, temp);
    }

    if (debug) { 
    /*
      std::cout << "%%% CallGraphDFSolver::atDGraphNode, ";
      std::cout << "nodeInSet = ";
      nodeInSet->dump(std::cout);
      std::cout << "mNodeInSetMap[node] = ";
      mNodeInSetMap[node]->dump(std::cout);*/
    }
    // calculate the output data-flow set for this procedure
    OA_ptr<DataFlowSet> nodeOutSet;
    if (node->getProc() != ProcHandle(0)) {

        nodeOutSet = mDFProb.atCallGraphNode(nodeInSet, node->getProc());
    } else {
        nodeOutSet = mNodeOutSetMap[node];
    }
    mNodeInSetMap[node] = nodeInSet;

    // update the out set for this node if has changed
    bool changed = false;
    if ( mNodeOutSetMap[node] != nodeOutSet ) {
        if (debug) { 
        /*
          std::cout << "%%% CallGraphDFSolver::atDGraphNode, "
                    << "mNodeOutSetMap != nodeOutSet" << std::endl;
          std::cout << "mNodeOutSetMap = ";
          mNodeOutSetMap[node]->dump(std::cout);
          std::cout << "nodeOutSet = ";
          nodeOutSet->dump(std::cout);
          */
        }
        mNodeOutSetMap[node] = nodeOutSet;
        changed = true;
    }
    if (debug) { std::cout << "CallGraphDFSolver::atDGraphNode, changed = "
                           << changed << std::endl;
    }

    return changed;
}

  
//--------------------------------------------------------
// finalization upcalls
//--------------------------------------------------------
void 
CallGraphDFSolver::finalizeNode(
    OA_ptr<DGraph::NodeInterface> node)
{
}

void 
CallGraphDFSolver::finalizeEdge(
    OA_ptr<DGraph::EdgeInterface> edge)
{
}

//--------------------------------------------------------
// debugging upcall 
//--------------------------------------------------------
void CallGraphDFSolver::dump(std::ostream& os, 
    OA_ptr<IRHandlesIRInterface> ir)
{
}
 


  } // end of DataFlow namespace
}  // end of OA namespace


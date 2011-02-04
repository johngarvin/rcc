/*! \file
  
  \brief Definition of Kildall iterative data flow solver for directed graphs.

  \authors John Mellor-Crummey (July 1994), 
           adapted for OA by Michelle Strout (April 2004)
  \version $Id: DGraphIterativeDFP.cpp,v 1.22 2005/06/15 03:32:01 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "DGraphSolverDFP.hpp"
#include <Utils/Util.hpp>

#include <iostream>

#include <queue>

namespace OA {
  namespace DataFlow {

static bool debug = false;

//*********************************************************************
// class DGraphIterativeDFP
//*********************************************************************

DGraphSolverDFP::DGraphSolverDFP()
{ 
    OA_DEBUG_CTRL_MACRO("DEBUG_DGraphIterativeDFP:ALL", debug);
    numIter = 0;
}


DGraphSolverDFP::~DGraphSolverDFP()
{
}


//-----------------------------------------------------------------------
// general default solver 
//-----------------------------------------------------------------------
void DGraphSolverDFP::solve(OA_ptr<DGraph::DGraphInterface> dg, 
                            DGraph::DGraphEdgeDirection alongFlow,
                            DFPImplement algorithm) 
{
    if(algorithm == ITERATIVE) {
        Iterative_Solve(dg,alongFlow);
    } else {
        WorkList_Solve(dg,alongFlow,algorithm);
    }
}

void DGraphSolverDFP::Iterative_Solve(OA_ptr<DGraph::DGraphInterface> dg,
                               DGraph::DGraphEdgeDirection alongFlow)
{
  int numNodeAccess=0;
  //---------------------------------------------------------------
  // initialize dataflow information at each of the nodes and edges
  //---------------------------------------------------------------
  initialize(dg);
  //---------------------------------------------------------------
  // Kildall style iterative solver: iterate until dataflow
  // information at each node and edge stabilizes
  //---------------------------------------------------------------
  unsigned int changed;

  OA_ptr<DGraph::NodeInterface> node;

  OA_ptr<DGraph::NodesIteratorInterface> nodeIterPtr
     = dg->getReversePostDFSIterator(alongFlow);

  // loop until there are no changes
  int iterCnt = 0;

  do {
    changed = 0;
    iterCnt++;
    if (debug) {
        std::cout << "DGraphIterativeDFP (in loop), iterCnt = " << iterCnt << std::endl;
    }
    for (; nodeIterPtr->isValid(); ++(*nodeIterPtr)) {
      node = nodeIterPtr->current();

      numNodeAccess++;
      if (debug) {
        std::cout << "node: Id = " << node->getId();
        std::cout << ", num_outgoing = " << node->num_outgoing();
        std::cout << ", OA_ptr::dump = ";
        node.dump(std::cout);
        std::cout << std::endl;
      }

      //--------------------------------------------------
      // compute dataflow information at node based on
      // what is coming into the node along the dataflow
      // direction
      //--------------------------------------------------

      changed |= atDGraphNode(node, alongFlow);

      //--------------------------------------------------
      // compute dataflow information going out of the node
      // along dataflow direction
      //--------------------------------------------------

      OA_ptr<DGraph::EdgesIteratorInterface> edgeIterPtr;
      if (alongFlow==DGraph::DEdgeOrg) {  // follows original orientation of edges
        OA_ptr<DGraph::EdgesIteratorInterface> it =
          node->getOutgoingEdgesIterator();
        edgeIterPtr = it;
      } else {
        OA_ptr<DGraph::EdgesIteratorInterface> it =
          node->getIncomingEdgesIterator();
        edgeIterPtr = it;
      }
      for (; edgeIterPtr->isValid(); ++(*edgeIterPtr)) {
        changed |= atDGraphEdge(edgeIterPtr->current(), alongFlow);
      }

    } // end for all nodes

    nodeIterPtr->reset();

  } while (changed);

  
   //---------------------------------------------------------------
  // finalize dataflow information at each of the nodes and edges
  //---------------------------------------------------------------
  for (; nodeIterPtr->isValid(); ++(*nodeIterPtr)) {
    node = nodeIterPtr->current();
    finalizeNode(node);

    OA::OA_ptr<DGraph::EdgesIteratorInterface> edgeIterPtr;
    if (alongFlow==DGraph::DEdgeOrg) {  // follows original orientation of edges

        OA::OA_ptr<DGraph::EdgesIteratorInterface> it =
            node->getOutgoingEdgesIterator();
      edgeIterPtr = it;
    } else {

      OA_ptr<DGraph::EdgesIteratorInterface> it =
          node->getIncomingEdgesIterator();
      edgeIterPtr = it;
    }
    for (; edgeIterPtr->isValid(); ++(*edgeIterPtr)) {
      finalizeEdge(edgeIterPtr->current());
    }

  }

  numIter = iterCnt; // set value for retrieval

  if (debug) {
      std::cout << "DGraphIterativeDFP: iterCnt = " << iterCnt << std::endl;
  }

  return;
 
}


void DGraphSolverDFP::WorkList_Solve(OA_ptr<DGraph::DGraphInterface> dg,
                               DGraph::DGraphEdgeDirection alongFlow,
                               DFPImplement algorithm)
{

  int numNodesAccessed=0;
  //---------------------------------------------------------------
  // initialize dataflow information at each of the nodes and edges
  //---------------------------------------------------------------
  initialize(dg);
  //---------------------------------------------------------------
  // Kildall style iterative solver: iterate until dataflow
  // information at each node and edge stabilizes
  //---------------------------------------------------------------
  unsigned int changed;

  OA_ptr<DGraph::NodeInterface> node;


  OA_ptr<DGraph::NodesIteratorInterface> nodeIterPtr
     = dg->getReversePostDFSIterator(alongFlow);

  OA_ptr<WorkList> wlist;

  if(  algorithm == WORKLIST_PRIORITY_QUEUE) {

        wlist = new Worklist_PQueue(dg,alongFlow);

  } else if(  algorithm == WORKLIST_QUEUE ) {

        wlist = new Worklist_Queue(dg,alongFlow);

  }

  // loop until there are no changes
  int iterCnt = 0;
  while (!wlist->isEmpty()) {
    node = wlist->getNext();

    if (debug) {
        std::cout << "node: Id = " << node->getId();
        std::cout << ", num_outgoing = " << node->num_outgoing();
        std::cout << ", OA_ptr::dump = ";
        node.dump(std::cout);
        std::cout << std::endl;
    }

    //--------------------------------------------------
    // compute dataflow information at node based on
    // what is coming into the node along the dataflow
    // direction
    //--------------------------------------------------
    changed = atDGraphNode(node, alongFlow);

    //--------------------------------------------------
    // compute dataflow information going out of the node
    // along dataflow direction
    //--------------------------------------------------
    OA_ptr<DGraph::EdgesIteratorInterface> edgeIterPtr;
    if (alongFlow==DGraph::DEdgeOrg) {  // follows original orientation of edges
        OA_ptr<DGraph::EdgesIteratorInterface> it =
          node->getOutgoingEdgesIterator();
        edgeIterPtr = it;
    } else {
        OA_ptr<DGraph::EdgesIteratorInterface> it =
          node->getIncomingEdgesIterator();
        edgeIterPtr = it;
    }
    for (; edgeIterPtr->isValid(); ++(*edgeIterPtr)) {
        changed |= atDGraphEdge(edgeIterPtr->current(), alongFlow);
    }

    OA_ptr<DGraph::NodesIteratorInterface> neighIter;
    if (alongFlow==DGraph::DEdgeOrg) {
      neighIter = node->getSinkNodesIterator();
    } else {
      neighIter = node->getSourceNodesIterator();
    }

    if (changed) {
      if (debug) {
        std::cout << "DGraphIterativeDFP::solve: adding: ";
      }
      for (; neighIter->isValid(); ++(*neighIter)) {
        OA_ptr<DGraph::NodeInterface> n;
        n = neighIter->current();
        wlist->add(n);
        if (debug) {
            std::cout << n->getId() << " ";
        }
      }
    }

    } // end for all nodes

    nodeIterPtr->reset();
    //---------------------------------------------------------------
    // finalize dataflow information at each of the nodes and edges
    //---------------------------------------------------------------
   for (; nodeIterPtr->isValid(); ++(*nodeIterPtr)) {
    node = nodeIterPtr->current();
    finalizeNode(node);

    OA::OA_ptr<DGraph::EdgesIteratorInterface> edgeIterPtr;
    if (alongFlow==DGraph::DEdgeOrg) {  // follows original orientation of edges
        OA::OA_ptr<DGraph::EdgesIteratorInterface> it =
            node->getOutgoingEdgesIterator();
        edgeIterPtr = it;
    } else {
        OA_ptr<DGraph::EdgesIteratorInterface> it =
            node->getIncomingEdgesIterator();
        edgeIterPtr = it;
    }
    for (; edgeIterPtr->isValid(); ++(*edgeIterPtr)) {
      finalizeEdge(edgeIterPtr->current());
    }

  }

}

//-----------------------------------------------------------------------
// solver callbacks
//-----------------------------------------------------------------------

bool DGraphSolverDFP::atDGraphNode
     (OA_ptr<DGraph::NodeInterface>, DGraph::DGraphEdgeDirection)
{
    
    return false;
}


bool DGraphSolverDFP::atDGraphEdge
     (OA_ptr<DGraph::EdgeInterface>, DGraph::DGraphEdgeDirection)
{
    return false;
}



//-----------------------------------------------------------------------
// finalization callbacks
//-----------------------------------------------------------------------


 void DGraphSolverDFP::finalizeEdge(OA_ptr<DGraph::EdgeInterface>)
{
}


 void DGraphSolverDFP::finalizeNode(OA_ptr<DGraph::NodeInterface>)
{
}

  } // end of DataFlow namespace
}  // end of OA namespace


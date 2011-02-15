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

#include "DGraphIterativeDFP.hpp"
#include <Utils/Util.hpp>

#include <iostream>

namespace OA {
  namespace DataFlow {

static bool debug = false;

//*********************************************************************
// class DGraphIterativeDFP
//*********************************************************************

DGraphIterativeDFP::DGraphIterativeDFP()
{ 
    OA_DEBUG_CTRL_MACRO("DEBUG_DGraphIterativeDFP:ALL", debug);
    numIter = 0;
}


DGraphIterativeDFP::~DGraphIterativeDFP()
{
}


//-----------------------------------------------------------------------
// general default solver 
//-----------------------------------------------------------------------
void DGraphIterativeDFP::solve(OA_ptr<DGraph::DGraphInterface> dg, 
                               DGraph::DGraphEdgeDirection alongFlow) 
{

  //---------------------------------------------------------------
  // initialize dataflow information at each of the nodes and edges
  //---------------------------------------------------------------
  initialize(dg);


  //---------------------------------------------------------------
  // Kildall style iterative solver: iterate until dataflow 
  // information at each node and edge stabilizes
  //---------------------------------------------------------------
  unsigned int changed;

  /*! commented out by PLM 08/11/06
  OA_ptr<DGraph::DGraphInterface::NodeInterface> node;
  */

  OA_ptr<DGraph::NodeInterface> node;

  // get iterator for desired direction
  /*! commented out by PLM 08/11/06
  OA_ptr<DGraph::DGraphInterface::NodesIterator> nodeIterPtr 
    = dg->getReversePostDFSIterator(alongFlow);
    */


  
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

      /*! commented out by PLM 08/11/06
      OA::OA_ptr<DGraph::DGraphInterface::EdgesIterator> edgeIterPtr;
      */

      OA_ptr<DGraph::EdgesIteratorInterface> edgeIterPtr;
      if (alongFlow==DGraph::DEdgeOrg) {  // follows original orientation of edges

        /*! commented out by PLM 08/11/06  
        OA_ptr<DGraph::DGraphInterface::EdgesIterator> it = 
          node->getOutgoingEdgesIterator(); // Stupid Sun CC 5.4
        */

        OA_ptr<DGraph::EdgesIteratorInterface> it = 
          node->getOutgoingEdgesIterator();  
        edgeIterPtr = it;
      } else {
        
       
        /*! commented out by PLM 08/11/06  
        OA_ptr<DGraph::DGraphInterface::EdgesIterator> it = 
          node->getIncomingEdgesIterator(); // Stupid Sun CC 5.4
        */
          
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

    /*! commented out by PLM 08/11/06
    OA::OA_ptr<DGraph::DGraphInterface::EdgesIterator> edgeIterPtr;
    */

    OA::OA_ptr<DGraph::EdgesIteratorInterface> edgeIterPtr;
    if (alongFlow==DGraph::DEdgeOrg) {  // follows original orientation of edges

      /*! commented out by PLM 08/11/06  
      OA_ptr<DGraph::DGraphInterface::EdgesIterator> it = 
	node->getOutgoingEdgesIterator(); // Stupid Sun CC 5.4
    */
      
        OA::OA_ptr<DGraph::EdgesIteratorInterface> it =
            node->getOutgoingEdgesIterator();  
      edgeIterPtr = it;
    } else {

      /*! commented out by PLM 08/10/06  
      OA_ptr<DGraph::DGraphInterface::EdgesIterator> it = 
	node->getIncomingEdgesIterator(); // Stupid Sun CC 5.4
     */

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


//-----------------------------------------------------------------------
// solver callbacks
//-----------------------------------------------------------------------
/*! commented out by PLM 08/11/06
bool DGraphIterativeDFP::atDGraphNode
    (OA_ptr<DGraph::DGraphInterface::NodeInterface>, DGraph::DGraphEdgeDirection)
{
  return false;
}
*/

bool DGraphIterativeDFP::atDGraphNode
     (OA_ptr<DGraph::NodeInterface>, DGraph::DGraphEdgeDirection)
{
    
    return false;
}



/*! commented out by PLM 08/11/06  
bool DGraphIterativeDFP::atDGraphEdge
    (OA_ptr<DGraph::DGraphInterface::EdgeInterface>, DGraph::DGraphEdgeDirection)
{
  return false;
}
*/

bool DGraphIterativeDFP::atDGraphEdge
     (OA_ptr<DGraph::EdgeInterface>, DGraph::DGraphEdgeDirection)
{
    return false;
}



//-----------------------------------------------------------------------
// finalization callbacks
//-----------------------------------------------------------------------


/*! commented out by PLM 08/11/06
void DGraphIterativeDFP::finalizeEdge(OA_ptr<DGraph::DGraphInterface::EdgeInterface>)
{
}
*/



 void DGraphIterativeDFP::finalizeEdge(OA_ptr<DGraph::EdgeInterface>)
{
}


/*! commented out by PLM 08/11/06
void DGraphIterativeDFP::finalizeNode(OA_ptr<DGraph::DGraphInterface::NodeInterface>)
{
}
*/

 

 void DGraphIterativeDFP::finalizeNode(OA_ptr<DGraph::NodeInterface>)
{
}

  } // end of DataFlow namespace
}  // end of OA namespace


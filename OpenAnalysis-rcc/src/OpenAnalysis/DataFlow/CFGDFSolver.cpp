/*! \file
  
  \brief Definition of a dataflow algorithm for CFGs.

  \authors Michelle Strout (April 2004)
           adapted from John Mellor-Crummey's dsystem CallGraphDFProblem
  \version $Id: CFGDFSolver.cpp,v 1.24 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>

  Added by Priyadarshini Malusare 07/26/06
*/

#include "CFGDFSolver.hpp"
#include <Utils/Util.hpp>

namespace OA {
  namespace DataFlow {

static bool debug = false;


CFGDFSolver::CFGDFSolver(DFDirectionType pDirection, CFGDFProblem& prob)
    : mDirection(pDirection), mDFProb(prob)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_CFGDFSolver:ALL", debug);
}

OA_ptr<DataFlowSet> CFGDFSolver::solve(OA_ptr<CFG::CFGInterface> cfg, 
                                       DFPImplement algorithm)
{
    // remove all mappings from statements to in and out sets
//    mStmtInSetMap.clear();
//    mStmtOutSetMap.clear();

    mNodeInSetMap.clear();
    mNodeOutSetMap.clear();
    mNodeInitTransApp.clear();

    mTop = mDFProb.initializeTop();
    
    //mBottom = initializeBottom();
    
    
    DataFlow::DGraphSolverDFP::solve(cfg, 
            ((mDirection == Forward) ? DGraph::DEdgeOrg : DGraph::DEdgeRev),
            algorithm);


    // if forward then return DataFlowSet for exit
    if (mDirection==Forward) {
        OA_ptr<CFG::NodeInterface> exitnode = cfg->getExit();
        return mNodeInSetMap[exitnode];

    // if backward then return DataFlowSet for entry
    } else {
        OA_ptr<CFG::NodeInterface> entrynode = cfg->getEntry();
        return mNodeOutSetMap[entrynode];
    }
}

  
//========================================================
// implementation of DGraphIterativeDFP callbacks
//========================================================
  
//--------------------------------------------------------
// initialization upcall 
//--------------------------------------------------------
void CFGDFSolver::initialize(OA_ptr<DGraph::DGraphInterface> dg)
{
    
    OA_ptr<CFG::CFGInterface> cfg = dg.convert<CFG::CFGInterface>();


    // iterate over all nodes and call initialization routine
    // that sets up DataFlowSets
    OA_ptr<CFG::NodesIteratorInterface> nodeIterPtr;
    nodeIterPtr = cfg->getCFGNodesIterator();

    

    for ( ;nodeIterPtr->isValid(); ++(*nodeIterPtr) ) {


        /*! added by PLM 08/10/06 */
        OA_ptr<CFG::NodeInterface> node = nodeIterPtr->currentCFGNode();    
        
        mNodeInSetMap[node]
                     = mDFProb.initializeNodeIN(node);
        
        mNodeOutSetMap[node]
                     = mDFProb.initializeNodeOUT(node);
        
        mNodeInitTransApp[node] = false;

    }

}



//--------------------------------------------------------
// solver upcalls
//--------------------------------------------------------

bool CFGDFSolver::atDGraphNode( OA_ptr<DGraph::NodeInterface> pNode, 
                                 DGraph::DGraphEdgeDirection pOrient)
{
    bool changed = false;

    OA_ptr<CFG::NodeInterface> node 
        = pNode.convert<OA::CFG::NodeInterface>();

    if (debug) {
        std::cout << "CFGDFSolver::atDGraphNode: CFG node = ";
        std::cout << node->getId() << std::endl;
    }

    //-----------------------------------------------------
    // do a meet of all out information from nodes that are
    // predecessors based on the flow direction
    //-----------------------------------------------------
    OA_ptr<DataFlowSet> meetPartialResult = mTop->clone();
    // added following for ReachConsts, should not bother other flows
    // because DFProblem has monotonicity
    if (pOrient==DGraph::DEdgeOrg) { // forward
        meetPartialResult =
            mDFProb.meet(meetPartialResult,mNodeInSetMap[node]);
    } else {
        meetPartialResult =
            mDFProb.meet(meetPartialResult,mNodeOutSetMap[node]);
    }


    // set up iterator for predecessor nodes
    OA_ptr<CFG::NodesIteratorInterface> predIterPtr;
    if (pOrient==DGraph::DEdgeOrg) {
      OA_ptr<CFG::NodesIteratorInterface> it =
        node->getCFGPredNodesIterator(); // Stupid Sun CC 5.4
      predIterPtr = it.convert<CFG::NodesIteratorInterface>();
    } else {
      OA_ptr<CFG::NodesIteratorInterface> it =
        node->getCFGSuccNodesIterator(); // Stupid Sun CC 5.4
      predIterPtr = it.convert<CFG::NodesIteratorInterface>();
    }
    // iterate over predecessors and do meet operation
    for (; predIterPtr->isValid(); ++(*predIterPtr)) {
      OA_ptr<CFG::NodeInterface> predNode = predIterPtr->currentCFGNode();
      if (pOrient==DGraph::DEdgeOrg) {
        meetPartialResult = mDFProb.meet(meetPartialResult, mNodeOutSetMap[predNode]);
      } else {
        meetPartialResult = mDFProb.meet(meetPartialResult, mNodeInSetMap[predNode]);
      }
    }


    // update the appropriate set for this node
    if (pOrient==DGraph::DEdgeOrg) { // forward
      if ( mNodeInSetMap[node] != meetPartialResult ) {
        if (debug) {
            std::cout << "%%%%%%%% CFGDFSolver:  There was a change" 
                      << std::endl;
            std::cout << "\tmNodeInSetMap != meetPartialResult" << std::endl;
        }
        mNodeInSetMap[node] = meetPartialResult;
        changed = true;
      }
    } else { // reverse
      if ( mNodeOutSetMap[node] != meetPartialResult ) {
        mNodeOutSetMap[node] = meetPartialResult;
        changed = true;
      }
    }

    // if the data flowing into this node has changed or if the
    // transfer functions have never been applied, then
    // loop through statements in the CFG node/(Basic Block) 
    // calculating the new node out
    if (debug) {
      std::cout << "\tchanged = " << changed << ", mNITA[node]=" 
                << mNodeInitTransApp[node] << std::endl;
    }
    if (changed || !mNodeInitTransApp[node]) {
      changed = false;  // reuse to determine if there is a change based
                        // on the block transfer function
      mNodeInitTransApp[node] = true;

      // Forward direction
      if (pOrient==DGraph::DEdgeOrg) {
        OA_ptr<DataFlowSet> prevOut = mNodeInSetMap[node]->clone();
        // loop through statements in forward order
        if (debug) {
            std::cout << "CFGDFSolver: iterating over statements in node";
            std::cout << std::endl;
        }
        OA_ptr<CFG::NodeStatementsIteratorInterface> stmtIterPtr 
            = node->getNodeStatementsIterator();
        for (; stmtIterPtr->isValid(); ++(*stmtIterPtr)) {
          OA::StmtHandle stmt = stmtIterPtr->current();
          if (debug) {
              std::cout << "\tstmt(hval=" << stmt.hval() << ")" << std::endl;
          }
          prevOut = mDFProb.transfer(prevOut, stmt);
        }
        if (debug) {
            std::cout << "CFGDFSolver: done iterating over statements";
            std::cout << std::endl;
        }

        if (prevOut != mNodeOutSetMap[node] ) {
          changed = true;
          mNodeOutSetMap[node] = prevOut;
          if (debug) {
            std::cout << "%%%%%%%% CFGDFSolver:  There was a change" 
                      << std::endl;
            std::cout << "\tmNodeOutSetMap != prevOut" << std::endl;
          }
        }
      
      // Reverse direction
      } else { 
        OA_ptr<DataFlowSet> prevIn = mNodeOutSetMap[node]->clone();
        // loop through statements in reverse order
        if (debug) {
            std::cout << "CFGDFSolver: iterating backward over statements in node";
            std::cout << std::endl;
        }
        OA_ptr<CFG::NodeStatementsRevIteratorInterface> stmtIterPtr 
            = node->getNodeStatementsRevIterator();
        for (; stmtIterPtr->isValid(); ++(*stmtIterPtr)) {
          OA::StmtHandle stmt = stmtIterPtr->current();
          if (debug) {
              std::cout << "\tstmt(hval=" << stmt.hval() << ")" << std::endl;
          }
          prevIn = mDFProb.transfer(prevIn, stmt);
        }
        if (debug) {
            std::cout << "CFGDFSolver: done iterating over statements";
            std::cout << std::endl;
        }

        if (prevIn != mNodeInSetMap[node] ) {
          changed = true;
          mNodeInSetMap[node] = prevIn;
          if (debug) {
            std::cout << "%%%%%%%% CFGDFSolver:  There was a change" 
                      << std::endl;
            std::cout << "\tmNodeInSetMap != prevIn" << std::endl;
          }
        }
      }

    }
   
    if (debug) {
      std::cout << "CFGDFSolver::atDGraphNode: changed = " << changed << std::endl;
    }
    return changed;
}
  
//--------------------------------------------------------
// finalization upcalls
//--------------------------------------------------------
void 
CFGDFSolver::finalizeNode(OA_ptr<DGraph::NodeInterface> node)
{
}



//DataFlowSet* CFGDFProblem::transfer(DataFlowSet* in, DGraph::Interface::Node* n)
//{
//}

//--------------------------------------------------------
// debugging upcall 
//--------------------------------------------------------
void CFGDFSolver::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    /*! commented out by PLM 08/08/06
    // iterate over all entries in mNodeInSetMap and mNodeOutSetMap
    std::map<OA_ptr<CFG::NodeInterface>,
             OA_ptr<DataFlowSet> >::iterator mapIter;
    for (mapIter=mNodeInSetMap.begin(); mapIter!=mNodeInSetMap.end(); mapIter++)
    {
        OA_ptr<DataFlowSet> dfset = mapIter->second;
        os << "Node (" << mapIter->first << ") In: ";
        dfset->dump(os,ir);
        os << std::endl;
    }
    for (mapIter=mNodeOutSetMap.begin(); mapIter!=mNodeOutSetMap.end(); mapIter++)
    {
        OA_ptr<DataFlowSet> dfset = mapIter->second;
        os << "Node (" << mapIter->first << ") Out: ";
        dfset->dump(os,ir);
        os << std::endl;
    }
    */
}

  } // end of DataFlow namespace
}  // end of OA namespace


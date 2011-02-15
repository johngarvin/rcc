/*! \file
  
  \brief Definition of a dataflow algorithm for ICFGs.

  \authors Michelle Strout, Barbara Kreaseck

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ICFGDFSolver.hpp"
#include <Utils/Util.hpp>

namespace OA {
  namespace DataFlow {

static bool debug = false;

ICFGDFSolver::ICFGDFSolver(DFDirectionType pDirection, ICFGDFProblem& prob)
    : mDirection(pDirection), mDFProb(prob)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ICFGDFSolver", debug);
}

void
ICFGDFSolver::solve(OA_ptr<ICFG::ICFGInterface> icfg, DFPImplement algorithm)
{
    // remove all mappings of nodes to data flow sets 
    mNodeInSetMap.clear();
    mNodeOutSetMap.clear();
    mNodeInitTransApp.clear();

    mTop = mDFProb.initializeTop();

    DataFlow::DGraphSolverDFP::solve(icfg, 
            ((mDirection==Forward) ? DGraph::DEdgeOrg : DGraph::DEdgeRev),
            algorithm);

    /*
    // if forward then return DataFlowSet for exit
    if (mDirection==Forward) {
        OA_ptr<ICFG::ICFGStandard::Node> exitnode = icfg->getExit();
        return mNodeInSetMap[exitnode];

    // if backward then return DataFlowSet for entry
    } else {
        OA_ptr<ICFG::ICFGStandard::Node> entrynode = icfg->getEntry();
        return mNodeOutSetMap[entrynode];
    }
    */
}
  
//========================================================
// implementation of DGraphIterativeDFP callbacks
//========================================================
  
//--------------------------------------------------------
// initialization upcall 
//--------------------------------------------------------


void ICFGDFSolver::initialize(OA_ptr<DGraph::DGraphInterface> dg)
{
    OA_ptr<ICFG::ICFGInterface> cfg = dg.convert<ICFG::ICFGInterface>();

    // iterate over all nodes and call initialization routine
    // that sets up DataFlowSets
    OA_ptr<ICFG::NodesIteratorInterface> nodeIterPtr;
    nodeIterPtr = cfg->getICFGNodesIterator();
    if (debug) {
      std::cout << "initializing:  looping over nodes:\n";
    }
    for ( ;nodeIterPtr->isValid(); ++(*nodeIterPtr) ) {
      if (debug) {
        std::cout << "\tNode " << (nodeIterPtr->current())->getId()
                  << std::endl;
      }
      OA_ptr<ICFG::NodeInterface> iNode = nodeIterPtr->currentICFGNode();
      mNodeInSetMap[iNode]
        = mDFProb.initializeNodeIN(iNode);
      mNodeOutSetMap[iNode]
        = mDFProb.initializeNodeOUT(iNode);
      mNodeInitTransApp[iNode] = false;
    }
}


//--------------------------------------------------------
// solver upcalls
//--------------------------------------------------------
bool ICFGDFSolver::atDGraphNode( OA_ptr<DGraph::NodeInterface> pNode, 
                                 DGraph::DGraphEdgeDirection pOrient)
{
    bool changed = false;
    OA_ptr<ICFG::NodeInterface> node 
        = pNode.convert<OA::ICFG::NodeInterface>();

    if (debug) {
        std::cout << "ICFGDFSolver::atDGraphNode: ICFG node = ";
        std::cout << node->getId() << std::endl;
    }

    //-----------------------------------------------------
    // do a meet of all out information from nodes that are
    // predecessors with current input for this node
    // based on the flow direction, edge type,
    // and node type
    // FIXME: Have to meet with current input for this node because
    // of ReachConsts?
    //-----------------------------------------------------
    OA_ptr<DataFlowSet> meetPartialResult = mTop->clone();
    if (pOrient==DGraph::DEdgeOrg) {
      meetPartialResult = mDFProb.meet(meetPartialResult,mNodeInSetMap[node]);
    } else {
      meetPartialResult = mDFProb.meet(meetPartialResult,mNodeOutSetMap[node]);
    }

    // set up iterator for predecessor edges
    OA_ptr<ICFG::EdgesIteratorInterface> predIterPtr;
    if (pOrient==DGraph::DEdgeOrg) {
      predIterPtr = node->getICFGIncomingEdgesIterator();
    } else {
      predIterPtr = node->getICFGOutgoingEdgesIterator();
    }
    // iterate over predecessors and do meet operation
    for (; predIterPtr->isValid(); ++(*predIterPtr)) {
      OA_ptr<ICFG::EdgeInterface> predEdge = predIterPtr->currentICFGEdge();
      OA_ptr<ICFG::NodeInterface> predNode;
      OA_ptr<DataFlowSet> inSet;
      if (pOrient==DGraph::DEdgeOrg) {
        OA_ptr<ICFG::NodeInterface> predNode = predEdge->getICFGSource();
        switch(predEdge->getType()) {
            case (ICFG::CALL_EDGE):
                // mNodeInSet because only get to use data-flow set before call
                // in caller
                inSet = mDFProb.callerToCallee(predEdge->getSourceProc(),
                            mNodeOutSetMap[predNode], predEdge->getCall(),
                            predEdge->getSinkProc());
                break;
            case (ICFG::RETURN_EDGE):
                inSet = mDFProb.calleeToCaller(predEdge->getSourceProc(),
                            mNodeOutSetMap[predNode], predEdge->getCall(),
                            predEdge->getSinkProc());
                break;
            case (ICFG::CALL_RETURN_EDGE):
              // has same parameter order as callerToCallee() for now
              // but will need to be changed when callToReturn is 
              // revisited.  Currently, only LocDFSet is used.
                inSet = mDFProb.callToReturn(predEdge->getSourceProc(),
                            mNodeOutSetMap[predNode], predEdge->getCall(),
                            predEdge->getSinkProc());
                break;
            case (ICFG::CFLOW_EDGE):
                inSet = mNodeOutSetMap[predNode];
                break;
        }
        if (debug) {
          std::cout << "performing forward meet with pred node " 
                    << predNode->getId() << std::endl;
        }                                              
        meetPartialResult = mDFProb.meet(meetPartialResult, inSet);

      // backward flow
      } else {
        OA_ptr<ICFG::NodeInterface> predNode = predEdge->getICFGSink();
        switch(predEdge->getType()) {
            case (ICFG::CALL_EDGE):
                inSet = mDFProb.calleeToCaller(predEdge->getSinkProc(),
                            mNodeInSetMap[predNode], predEdge->getCall(),
                            predEdge->getSourceProc());
                break;
            case (ICFG::RETURN_EDGE):
                // use outset for RETURN_NODE in caller
                inSet = mDFProb.callerToCallee(predEdge->getSinkProc(),
                            mNodeInSetMap[predNode], predEdge->getCall(),
                            predEdge->getSourceProc());
                break;
            case (ICFG::CALL_RETURN_EDGE):
              // has same parameter order as callerToCallee() for now
              // but will need to be changed when callToReturn is 
              // revisited.  Currently, only LocDFSet is used.
                inSet = mDFProb.callToReturn(predEdge->getSinkProc(),
                            mNodeInSetMap[predNode], predEdge->getCall(),
                            predEdge->getSourceProc());
                break;
            case (ICFG::CFLOW_EDGE):
                inSet = mNodeInSetMap[predNode];
                break;
        }
        if (debug) {
          std::cout << "performing backward meet with succ node " 
                    << predNode->getId() << std::endl;
        }                                              
        meetPartialResult = mDFProb.meet(meetPartialResult, inSet);
      }
    }

    // update the appropriate set for this node
    if (pOrient==DGraph::DEdgeOrg) { // forward
      if ( mNodeInSetMap[node] != meetPartialResult ) {
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
        OA_ptr<DataFlowSet> currOut; 
        OA_ptr<DataFlowSet> inClone = mNodeInSetMap[node]->clone();

        // call transfer methods based on what kind of node 
        if (node->getType()==ICFG::ENTRY_NODE) {
            currOut = mDFProb.entryTransfer(node->getProc(), inClone);

        // otherwise it is a normal node and should apply regular transfer
        // loop through statements in forward order
        } else {

            if(node->getType()==ICFG::CALL_NODE) {
               OA::CallHandle call = node->getCall();
               currOut = mDFProb.transfer(node->getProc(), inClone, call); 
            } else {
               currOut = inClone;
               OA_ptr<CFG::NodeStatementsIteratorInterface> stmtIterPtr 
                   = node->getNodeStatementsIterator();
               for (; stmtIterPtr->isValid(); ++(*stmtIterPtr)) {
                    OA::StmtHandle stmt = stmtIterPtr->current();
                    currOut = mDFProb.transfer(node->getProc(), currOut, stmt);
               }
            }
        }
        // check to see if changed from last time
        if (currOut != mNodeOutSetMap[node] ) {
          changed = true;
          mNodeOutSetMap[node] = currOut;
        }
      
      // Reverse direction
      } else { 
        OA_ptr<DataFlowSet> currIn;
        OA_ptr<DataFlowSet> outClone = mNodeOutSetMap[node]->clone();
        
        // if it is an exit node call transfer method for exit nodes
        if (node->getType()==ICFG::EXIT_NODE) {
            currIn = mDFProb.exitTransfer(node->getProc(), outClone);

        // otherwise it is a normal node and should apply regular transfer
        } else {


            if(node->getType()==ICFG::CALL_NODE) {
                OA::CallHandle call = node->getCall();
                currIn = mDFProb.transfer(node->getProc(), outClone, call);
            } else {
                currIn = outClone;
                // loop through statements in reverse order
                OA_ptr<CFG::NodeStatementsRevIteratorInterface> stmtIterPtr 
                  = node->getNodeStatementsRevIterator();
                if (debug) {
                  std::cout << "looping through statements in reverse order:\n";
                }
                for (; stmtIterPtr->isValid(); ++(*stmtIterPtr)) {
                    OA::StmtHandle stmt = stmtIterPtr->current();
                    if (debug) {
                      std::cout << "\tstmt.hval(" << stmt.hval() << ")\n";
                    }
                    currIn = mDFProb.transfer(node->getProc(), currIn, stmt);
                }
            }
        }
        // check to see if changed from last time
        if (currIn != mNodeInSetMap[node] ) {
          changed = true;
          mNodeInSetMap[node] = currIn;
        }
      }

    }
   
    if (debug) {
      std::cout << "ICFGDFSolver::atDGraphNode: changed = " << changed 
                << std::endl;
    }
    return changed;
}
  
//--------------------------------------------------------
// finalization upcalls
//--------------------------------------------------------
void 
ICFGDFSolver::finalizeNode(OA_ptr<DGraph::NodeInterface> node)
{
}



//--------------------------------------------------------
// debugging upcall 
//--------------------------------------------------------
void ICFGDFSolver::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    // iterate over all entries in mNodeInSetMap and mNodeOutSetMap
    std::map<OA_ptr<ICFG::NodeInterface>,
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
}

  } // end of DataFlow namespace
}  // end of OA namespace


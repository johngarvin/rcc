/*! \file
  
  \brief The AnnotationManager that generates ICFG

  \authors Michelle Strout
  \version $Id: ManagerICFGStandard.cpp,v 1.2 2005/06/10 02:32:04 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <OpenAnalysis/ICFG/ManagerICFG.hpp>
#include <OpenAnalysis/Utils/Util.hpp>
#include <OpenAnalysis/Utils/OutputBuilderDOT.hpp>

static bool debug = false;

namespace OA {
  namespace ICFG {


/*!
*/
ManagerICFGStandard::ManagerICFGStandard(OA_ptr<ICFGIRInterface> _ir) 
    : mIR(_ir)
/*!
 */
{ 
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerICFGStandard:ALL", debug);
}

bool ManagerICFGStandard::stmt_has_call(StmtHandle stmt)
{
    bool callflag = false;
    OA_ptr<IRCallsiteIterator> callsiteItPtr = mIR->getCallsites(stmt);
    for ( ; callsiteItPtr->isValid(); ++(*callsiteItPtr)) {
        CallHandle call = callsiteItPtr->current();
        OA_ptr<ProcHandleIterator> procIter;
        procIter = mCallGraph->getCalleeProcIter(call);
        for ( ; procIter->isValid(); ++(*procIter)) {
          ProcHandle proc = procIter->current();
          if (debug) {
            std::cout << "called proc = " 
                      << mIR->toString(proc) << std::endl;
          }
          if (proc != ProcHandle(0)) {
            callflag = true;
          }
        }
    }
    return callflag;
}

StmtHandle 
ManagerICFGStandard::get_call_stmt(OA_ptr<CFG::Node> cfgNode)
{
  // assumes that there is at most one call stmt within a cfgNode
    OA_ptr<StmtHandleIterator> stmtIter;
    stmtIter = cfgNode->getNodeStatementsIterator();
    for ( ; stmtIter->isValid(); (*stmtIter)++ ) {
        StmtHandle stmt = stmtIter->current();
        if ( stmt_has_call(stmt) ) {
            return stmt; 
        }
    }
    return StmtHandle(0);
}

OA_ptr<Node> ManagerICFGStandard::create_icfg_node(
        OA_ptr<ICFG> icfg, ProcHandle proc, NodeType pType, 
        OA_ptr<CFG::Node> cfgNode)
{
  OA_ptr<Node> icfgNode; 
  icfgNode = new Node(icfg, proc, pType, cfgNode);
  icfg->addNode(icfgNode);
  mCFGNodeToICFGNode[cfgNode] = icfgNode;
  return icfgNode;
}

/*! The returned prevICFGNode (last one in sequence) is returned.
    firstICFGNode is set to first node in sequence.
*/
OA_ptr<Node>
ManagerICFGStandard::handle_call_node(OA_ptr<ICFG> icfg, 
        ProcHandle proc, OA_ptr<CFG::Node> cfgNodeNew,
        OA_ptr<Node> prevICFGNode, 
        OA_ptr<Node>& firstICFGNode,
        std::list<ProcHandle>& worklist)
{
    OA_ptr<Node> callICFGNode;
    OA_ptr<Edge> icfgEdge;
    StmtHandle stmt = get_call_stmt(cfgNodeNew);

    // Call node and matched return node
//    firstICFGNode = callICFGNode 
//        = create_icfg_node(icfg,proc,CALL_NODE,cfgNodeNew);


//    firstICFGNode = create_icfg_node(icfg,proc,CALL_NODE,cfgNodeNew);


    CallHandle callH;
    OA_ptr<IRCallsiteIterator>
        It = mIR->getCallsites(stmt);
    for ( ; It->isValid(); ++(*It)) {
        callH = It->current();
    }
    firstICFGNode = callICFGNode = new Node(icfg, proc, CALL_NODE, callH);


    // edge between prevICFGNode and callICFGNode 
    if (!prevICFGNode.ptrEqual(0)) {
        icfgEdge = new Edge(icfg, prevICFGNode, callICFGNode, CFLOW_EDGE, 
                            CallHandle(0), CFG::NO_EDGE, ExprHandle(0));
        icfg->addEdge(icfgEdge);
    }

    // return node
    OA_ptr<CFG::Node> emptyNode;
    emptyNode = new CFG::Node(stmt);
    prevICFGNode = create_icfg_node(icfg,proc,RETURN_NODE,emptyNode);

    // edge between callnode and return node
    //icfgEdge = new Edge(icfg, 
    //           callICFGNode, prevICFGNode, CALL_RETURN_EDGE, CallHandle(0));
    //icfg->addEdge(icfgEdge);

    // CALL_RETURN_EDGE should have valid CallHandle.
    // Passing CallHandle(0) in order to create Calll_RETURN_EDGE is Invalid.
    OA_ptr<IRCallsiteIterator>
        Iter = mIR->getCallsites(stmt);
    CallHandle call;
    for ( ; Iter->isValid(); ++(*Iter)) {
       call = Iter->current();
    }
    icfgEdge = new Edge(icfg, callICFGNode, prevICFGNode, CALL_RETURN_EDGE,
                        call, CFG::NO_EDGE, ExprHandle(0));
    icfg->addEdge(icfgEdge);

    // for each call, add a call Edge and return edge to each
    // may-called defined-procedure
    OA_ptr<IRCallsiteIterator> 
        callsiteItPtr = mIR->getCallsites(stmt);
    for ( ; callsiteItPtr->isValid(); ++(*callsiteItPtr)) {
        CallHandle call = callsiteItPtr->current();
        OA_ptr<ProcHandleIterator> procIter;
        procIter = mCallGraph->getCalleeProcIter(call);
        for ( ; procIter->isValid(); ++(*procIter)) {
          ProcHandle callee = procIter->current();
          if (callee==ProcHandle(0)) { continue; }
          worklist.push_back(callee);
          // get CFG
          OA_ptr<CFG::CFGInterface> calleeCFGInterface 
            = mEachCFG->getCFGResults(callee);
          OA_ptr<CFG::CFG> calleeCFG 
            = calleeCFGInterface.convert<CFG::CFG>();
          // get entry and exit node
          OA_ptr<CFG::NodeInterface> temp
            = calleeCFG->getEntry();
          OA_ptr<CFG::Node> entryNode 
            = temp.convert<CFG::Node>();
          temp = calleeCFG->getExit();
          OA_ptr<CFG::Node> exitNode 
            = temp.convert<CFG::Node>();
          // make an ICFG node for these if necessary
          if (mCFGNodeToICFGNode[entryNode].ptrEqual(0)) {
            mCFGNodeToICFGNode[entryNode] 
              = new Node(icfg, callee, ENTRY_NODE, entryNode);
            icfg->addNode(mCFGNodeToICFGNode[entryNode]);
          }
          if (mCFGNodeToICFGNode[exitNode].ptrEqual(0)) {
            mCFGNodeToICFGNode[exitNode] 
              = new Node(icfg, callee, EXIT_NODE, exitNode);
            icfg->addNode(mCFGNodeToICFGNode[exitNode]);
          }
          
          // edge to link callICFGNode to entryNode
          icfgEdge = new Edge(icfg, callICFGNode, 
                              mCFGNodeToICFGNode[entryNode], 
                              CALL_EDGE, call,
                              CFG::NO_EDGE, ExprHandle(0));
          icfg->addEdge(icfgEdge);
          
          // edge to link exitNode to prevICFGNode
          icfgEdge = new Edge(icfg, mCFGNodeToICFGNode[exitNode], 
                              prevICFGNode, RETURN_EDGE, call,
                              CFG::NO_EDGE, ExprHandle(0));
          icfg->addEdge(icfgEdge);

        } // end of loop for each may-called ProcHandle for this CallHandle

    } // end of loop for each CallHandle within this call StmtHandle
        
    return prevICFGNode;
}

OA_ptr<std::list<OA_ptr<CFG::Node> > > 
ManagerICFGStandard::break_out_calls(OA_ptr<CFG::Node> cfgNode)
{
    OA_ptr<std::list<OA_ptr<CFG::Node> > > retval;
    retval = new std::list<OA_ptr<CFG::Node> >;

    OA_ptr<CFG::Node> prevNode;
    prevNode = new CFG::Node;

    // loop through statements in original cfgNode
    // inserting them into prevNode until hit a call stmt
    // when hit a call statement make a new node for the call stmt
    // and then a new prevNode and loop
    OA_ptr<StmtHandleIterator> stmtIter;
    stmtIter = cfgNode->getNodeStatementsIterator();
    for ( ; stmtIter->isValid(); (*stmtIter)++ ) {
        StmtHandle stmt = stmtIter->current();
        if ( stmt_has_call(stmt) ) {
            if (prevNode->size() > 0) {
                retval->push_back(prevNode);
                prevNode = new CFG::Node;
            }
            prevNode->add(stmt);
            retval->push_back(prevNode);
            prevNode = new CFG::Node;
        } else {
            prevNode->add(stmt);
        }
    }
    retval->push_back(prevNode);

    return retval;
}

/*!
    Creates an ICFG based upon the CFGs for the program
*/
OA_ptr<ICFG> ManagerICFGStandard::performAnalysis( 
    OA_ptr<IRProcIterator> procIter,
    OA_ptr<CFG::EachCFGInterface> eachCFG,
    OA_ptr<CallGraph::CallGraphInterface> cGraph)
{
  OA_ptr<Edge> icfgEdge;
  OA_ptr<ICFG> icfg; icfg = new ICFG();

  // for use within helper functions
  mEachCFG = eachCFG;
  mCallGraph = cGraph;

  /* ---------------------------
  // list of edges that are needed, the ICFGStandard::Node is the
  // sink of the edge and the CFGNodes are predecessors
  // have to keep these until the end and all CFG nodes are mapped
  // to ICFG nodes

     std::map<OA_ptr<Node>,
              std::set<OA_ptr<CFG::Node> > > edgeMap;
  // BK: 08/14/08 change
  // Need to actually hang onto the CFG::Edges (incoming edges) rather
  // than just the Incoming source nodes. (to decorate ICFG edges with
  // CFG edge information)
  */ //-------------------------
  
  std::map<OA_ptr<Node>,
    std::set<OA_ptr<CFG::Edge> > > edgeMap;
 
 
  /*
  // get the CFG for the root procedure
  OA_ptr<CFG::Interface> rootCFGInterface = eachCFG->getCFGResults(rootProc);
  OA_ptr<CFG::CFGStandard> rootCFG 
      = rootCFGInterface.convert<CFG::CFGStandard>();
  if (debug) {
      std::cout << "--- rootCFG" << std::endl;
      rootCFG->dumpdot(std::cout, mIR);
  }

  // get entry and exit node for cfg
  OA_ptr<CFG::Interface::Node> temp = rootCFG->getEntry();
  OA_ptr<CFG::CFGStandard::Node> cfgNode 
      = temp.convert<CFG::CFGStandard::Node>();
  OA_ptr<ICFGStandard::Node> icfgNode 
      = create_icfg_node(icfg, rootProc, ENTRY_NODE, cfgNode);

  temp = rootCFG->getExit();
  cfgNode = temp.convert<CFG::CFGStandard::Node>();
  icfgNode = create_icfg_node(icfg, rootProc, EXIT_NODE, cfgNode);
  */

  // worklist and set of procs that have already been visited
  std::list<ProcHandle> worklist;
  std::set<ProcHandle> alreadyVisited;
  for ( procIter->reset(); procIter->isValid(); (*procIter)++ ) {
    worklist.push_back(procIter->current());
  }

  // while there is a proc in the worklist
  while ( ! worklist.empty() ) {
    
    // get next proc and make sure it hasn't already been visited
    ProcHandle proc = worklist.front();
    worklist.pop_front();
    if (alreadyVisited.find(proc)!=alreadyVisited.end()) {
        continue;
    } else {
        alreadyVisited.insert(proc);
    }

    // get cfg 
    OA_ptr<CFG::CFGInterface> cfgInterface = eachCFG->getCFGResults(proc);
    //cfgInterface->output(*mIR);
 
    OA_ptr<CFG::CFG> cfg = cfgInterface.convert<CFG::CFG>();
    if (debug) {
      std::cout << "\n\n\n--- cfg" << std::endl;
      cfg->output(*mIR);
      // dot output
      OA::OA_ptr<OA::OutputBuilder> outBuild;
      outBuild = new OA::OutputBuilderDOT;
      cfg->configOutput(outBuild);
      cfg->output(*mIR);
      std::cout << "\n\n\n";
      
      //cfg->dumpdot(std::cout, mIR);
      //cfg->dumpdot(std::cout, mIR);
    }

    // for each node in cfg create an icfg node
    OA_ptr<DGraph::NodesIteratorInterface> cfgNodeIter
        = cfg->getNodesIterator();
    for ( ; cfgNodeIter->isValid(); (*cfgNodeIter)++ ) {
        OA_ptr<DGraph::NodeInterface> dNode = cfgNodeIter->current();
        OA_ptr<CFG::Node> cfgNode = dNode.convert<CFG::Node>();

        if (debug) {
            std::cout << "cfgNode = ";
            //cfgNode->dumpdot(*cfg,std::cout,mIR);
        }

        // determine if any of the statements contain a call
        OA_ptr<StmtHandleIterator> stmtIter;
        stmtIter = cfgNode->getNodeStatementsIterator();
        bool callflag = false;
        for ( ; stmtIter->isValid(); (*stmtIter)++ ) {
            StmtHandle stmt = stmtIter->current();
            if (debug) { std::cout << "stmt = " 
                                   << mIR->toString(stmt) << std::endl;}
            if ( stmt_has_call(stmt) ) {
                callflag = true;
                if (debug) { std::cout << "\tcallflag = true" << std::endl;}
            }
        }

        // if there were any calls in any of the statements
        if (callflag) {
            // get list of new CFG nodes, will NOT have empty ones for
            // return nodes
            OA_ptr<std::list<OA_ptr<CFG::Node> > > newCFGList
                = break_out_calls(cfgNode);
            std::list<OA_ptr<CFG::Node> >::iterator newCFGNodeIter;
            newCFGNodeIter = newCFGList->begin();

            // for first one, create associated ICFG node and link that
            // associated ICFG with all predecessors for original CFG node
            OA_ptr<CFG::Node> firstCFGNode = *newCFGNodeIter;
            if (debug) {
                std::cout << "firstCFGNode = ";
                //firstCFGNode->dumpdot(*cfg,std::cout,mIR);
            }
            OA_ptr<Node> firstICFGNode, prevICFGNode;
            // Call node and matched return node
            if ( get_call_stmt(firstCFGNode) != StmtHandle(0) ) {
                prevICFGNode = handle_call_node(icfg, proc, firstCFGNode,
                                                prevICFGNode, firstICFGNode,
                                                worklist);
            // regular cfg node
            } else if (firstCFGNode->size() > 0) {
                firstICFGNode = create_icfg_node(icfg,proc,CFLOW_NODE, 
                                                 firstCFGNode);
                                                 
                prevICFGNode = firstICFGNode;
            }

            if (debug) {
                std::cout << "firstICFGNode = ";
                //firstICFGNode->dumpdot(*icfg,std::cout,mIR);
            }

            // get predecessors for original CFG node
            // now storing incoming edges rather than source nodes
            OA_ptr<CFG::EdgesIteratorInterface> ieIter
                = cfgNode->getCFGIncomingEdgesIterator();
            for ( ; ieIter->isValid(); (*ieIter)++ ) {
                OA_ptr<CFG::Edge> inCFGEdge;
                inCFGEdge = (ieIter->current()).convert<CFG::Edge>();
                // temporarily record
                // edge between predecessor ICFG node and firstICFGNode
                edgeMap[firstICFGNode].insert(inCFGEdge);
            }

            // for middle ones create appropriate ICFG and link to previous
            newCFGNodeIter++;
            for ( ; newCFGNodeIter!=newCFGList->end(); newCFGNodeIter++ )
            {
                OA_ptr<CFG::Node> cfgNodeNew = *newCFGNodeIter;

                // Call node and matched return node
                if ( get_call_stmt(cfgNodeNew) != StmtHandle(0) ) {
                    prevICFGNode = handle_call_node(icfg, proc, cfgNodeNew,
                                                  prevICFGNode, firstICFGNode,
                                                  worklist);

                // regular cfg node
                } else if (cfgNodeNew->size() > 0) {

                    OA_ptr<Node> currICFGNode;
                    currICFGNode  
                        = create_icfg_node(icfg,proc,CFLOW_NODE,cfgNodeNew);

                    // edge between prevICFGNode and this one
                    icfgEdge = new Edge(icfg, prevICFGNode, currICFGNode,
                                        CFLOW_EDGE, CallHandle(0),
                                        CFG::NO_EDGE, ExprHandle(0));
                    icfg->addEdge(icfgEdge);

                    prevICFGNode = currICFGNode;
                }

            } 
            
            // for last one, associate with original CFG node
            // so correct linkages will occur later when successors
            // are linking with predecessors
            mCFGNodeToICFGNode[cfgNode] = prevICFGNode; 

        // CFG node that doesn't have a call stmt
        } else {
            OA_ptr<Node> icfgNode; 
            if (mCFGNodeToICFGNode[cfgNode].ptrEqual(0)) {
                if (cfg->getEntry() == cfgNode) {
                    icfgNode = create_icfg_node(icfg,proc,ENTRY_NODE,cfgNode);
                } else if (cfg->getExit() == cfgNode) {
                    icfgNode = create_icfg_node(icfg,proc,EXIT_NODE,cfgNode);
                } else {
                    icfgNode = create_icfg_node(icfg,proc,CFLOW_NODE,cfgNode);
                }
            } else {
                icfgNode = mCFGNodeToICFGNode[cfgNode];
            }

            // now storing in-coming edges rather than source nodes
            OA_ptr<CFG::EdgesIteratorInterface> ieIter
                = cfgNode->getCFGIncomingEdgesIterator();
            for ( ; ieIter->isValid(); (*ieIter)++ ) {
                OA_ptr<CFG::Edge> inCFGEdge;
                inCFGEdge = (ieIter->current()).convert<CFG::Edge>();;
                // temporarily record
                // edge between predecessor ICFG node and firstICFGNode
                edgeMap[icfgNode].insert(inCFGEdge);
            }

        }

    } // end of loop over CFG nodes 


  } // worklist loop for procedures          
  
  // loop over edges and generate ICFG edges
  std::map<OA_ptr<Node>,
           std::set<OA_ptr<CFG::Edge> > >::iterator mapIter;
  for (mapIter=edgeMap.begin(); mapIter!=edgeMap.end(); mapIter++ ) {
      OA_ptr<Node> icfgSink = mapIter->first;
      std::set<OA_ptr<CFG::Edge> >& edgeSet = mapIter->second;
      std::set<OA_ptr<CFG::Edge> >::iterator edgeIter;
      for (edgeIter=edgeSet.begin(); edgeIter!=edgeSet.end(); edgeIter++) {
          OA_ptr<CFG::Edge> inEdge = *edgeIter;
          CFG::EdgeType cfgEdgeType = inEdge->getType();
          ExprHandle cfgEdgeExpr = inEdge->getExpr(); 
          OA_ptr<CFG::Node> cfgSource;
          cfgSource = (inEdge->getCFGSource()).convert<CFG::Node>();
          OA_ptr<Node> icfgSource = mCFGNodeToICFGNode[cfgSource];

          icfgEdge = new Edge(icfg, icfgSource, icfgSink, 
                              CFLOW_EDGE, CallHandle(0),
                              cfgEdgeType, cfgEdgeExpr);
          icfg->addEdge(icfgEdge);
      }
  }

  return icfg;
}


  } // end of namespace MPICFG
} // end of namespace OA

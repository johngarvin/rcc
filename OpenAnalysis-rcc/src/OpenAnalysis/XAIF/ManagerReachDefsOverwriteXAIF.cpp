/*! \file
  
  \brief The AnnotationManager that generates ReachDefsOverwriteXAIF

  \authors Jean Utke

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerReachDefsOverwriteXAIF.hpp"
#include "OpenAnalysis/ReachDefs/ReachDefsStandard.hpp"


namespace OA {
  namespace XAIF {

static bool debug = false;


    ManagerReachDefsOverwriteXAIF::ManagerReachDefsOverwriteXAIF(OA_ptr<XAIFIRInterface> _ir) : mIR(_ir) {
      OA_DEBUG_CTRL_MACRO("DEBUG_ManagerReachDefsOverwriteXAIF:ALL", debug);
    }

    OA_ptr<ReachDefsOverwriteXAIF> ManagerReachDefsOverwriteXAIF::performAnalysis(OA_ptr<CFG::CFGInterface> cfg,
									 OA_ptr<ReachDefsOverwrite::Interface> reachDefs) {
      if (debug) {
	std::cout << "In OA::XAIF::ManagerReachDefsOverwriteXAIF::performAnalysis" << std::endl;
      }
      OA_ptr<ReachDefsOverwriteXAIF> aReachDefsOverwriteXAIF;
      aReachDefsOverwriteXAIF = new ReachDefsOverwriteXAIF();
      OA_ptr<DGraph::NodesIteratorInterface> nodeIterPtr;
      nodeIterPtr = cfg->getNodesIterator();
      // looping over basic blocks
      for ( ;nodeIterPtr->isValid(); ++(*nodeIterPtr) ) {
	OA_ptr<DGraph::NodeInterface> dnode = nodeIterPtr->current();
	OA_ptr<CFG::NodeInterface> node = dnode.convert<CFG::NodeInterface>();
	OA_ptr<CFG::NodeStatementsIteratorInterface> stmtIterPtr 
	  = node->getNodeStatementsIterator();
	// looping over statements in basic blocks
	for (; stmtIterPtr->isValid(); ++(*stmtIterPtr)) {
	  OA::StmtHandle stmt = stmtIterPtr->current();
	  if (debug) {
	    std::cout << "checking overwrites of " 
		      << mIR->toString(stmt) << std::endl; 
	  }
	  StmtSet subSet;
	  OA_ptr<ReachDefs::Interface::ReachDefsIterator> overwritingStmtIterPtr;
	  overwritingStmtIterPtr = reachDefs->getOverwritingStmts(stmt);
	  for (; overwritingStmtIterPtr->isValid(); ++(*overwritingStmtIterPtr)) {
	    StmtHandle overwritingStmt = overwritingStmtIterPtr->current();
	    if (debug) {
	      std::cout << "overwritingStmt (" << overwritingStmt.hval() << ") = " 
			<< mIR->toString(overwritingStmt) << std::endl; 
	    }
	    subSet.insert(overwritingStmt);
	  }
	  int chainId = aReachDefsOverwriteXAIF->findChain(subSet);
	  if (chainId == ChainsXAIF::CHAIN_ID_NONE) {
	    // if it isn't then make a new chain
	    chainId=getNextChainId();
	    // and add the subset of statements into that chain
	    aReachDefsOverwriteXAIF->addStmtSet(subSet,chainId);
	  }
	  // assiciate the stmt with the chainId
	  aReachDefsOverwriteXAIF->insert(stmt, chainId);
	}
      }
      // insert an empty chain as chain 0 because that is the default chain in XAIF
      // don't insert it until the end because while building, we don't want to find
      // empty chains at 0, want to find it at 1
      StmtSet emptySet;
      aReachDefsOverwriteXAIF->addStmtSet(emptySet,0);
      return aReachDefsOverwriteXAIF;
    }
    
    int ManagerReachDefsOverwriteXAIF::ourCurrentStartId=3;

    int ManagerReachDefsOverwriteXAIF::getNextChainId() {
      return ourCurrentStartId++;
    }
      
  }
} 

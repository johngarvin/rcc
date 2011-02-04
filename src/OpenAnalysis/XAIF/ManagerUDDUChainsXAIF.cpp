/*! \file
  
  \brief The AnnotationManager that filters UDDUChains for XAIF.

  \authors Michelle Strout
  \version $Id: ManagerUDDUChainsXAIF.cpp,v 1.23 2005/01/18 21:13:16 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerUDDUChainsXAIF.hpp"


namespace OA {
  namespace XAIF {

    static bool debug = false;

    /*!
     */
    ManagerUDDUChainsXAIF::ManagerUDDUChainsXAIF(OA_ptr<XAIFIRInterface> _ir) : mIR(_ir)
    {
      OA_DEBUG_CTRL_MACRO("DEBUG_ManagerUDDUChainsXAIF:ALL", debug);
    }

    /*!
      \brief    Used to perform analysis when not using AQM
    */
    OA_ptr<UDDUChainsXAIF> ManagerUDDUChainsXAIF::performAnalysis(OA_ptr<CFG::CFGInterface> cfg, 
								  OA_ptr<UDDUChains::Interface> udChains,
								  bool donotfilterBB) {
      if (debug) {
	std::cout << "In ReachDefs::ManagerUDDUChainsXAIF::performAnalysis" << std::endl;
      }
      OA_ptr<UDDUChainsXAIF> aUDDUChainsXAIF;
      aUDDUChainsXAIF = new UDDUChainsXAIF();
      // making a mapping of Stmt's to basic block id's
      // FIXME: add functionality to CFG?
      // also mapping memory references to stmts
      // first assign StmtHandle(0) to entry node, which should have no
      // other statements in it
      mStmtToBBMap[StmtHandle(0)] = cfg->getEntry();
      if (debug) { std::cout << "mStmtToBBMap[StmtHandle(0)] = "
			     << mStmtToBBMap[StmtHandle(0)] << std::endl; }
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
	  // mapping of statement to CFG nodes
	  mStmtToBBMap[stmt] = node;
	  if (debug && !donotfilterBB) { 
            std::cout << "mapping stmt: "
		      << mIR->toString(stmt)
		      << " to CFG node >"; 
	    node->output(*mIR);
	    std::cout << "< end of CFG node and the following memrefs are mapped to this statement: " << std::endl;
	  }
	  // for all memory references in the statement
	  OA_ptr<MemRefHandleIterator> mrItPtr = mIR->getAllMemRefs(stmt);
	  for ( ; mrItPtr->isValid(); (*mrItPtr)++) {
	    if (debug) {
	      std::cout << "memref (" << mrItPtr->current().hval() 
			<< ") = " << mIR->toString(mrItPtr->current()) << std::endl;
	    }
	    mMemRefToStmt[mrItPtr->current()] = stmt;
	  }
	}
      }
      // for each UDChain start from a use memory reference:
      OA_ptr<UDDUChains::Interface::MemRefsWithUDChainIterator> useIterPtr; 
      useIterPtr = udChains->getMemRefsWithUDChainIterator();
      for (; useIterPtr->isValid(); ++(*useIterPtr)) {
	bool haveDefinitionBeforeUse=false; 
	MemRefHandle use = useIterPtr->current();
	if (debug) {
	  std::cout << "use memref (" << use.hval() << ") = " << mIR->toString(use);
	  std::cout << std::endl;
	}
	// get the BB in which the use occurs
	OA_ptr<CFG::NodeInterface> useBB = mStmtToBBMap[mMemRefToStmt[use]];
	if (debug) {
	  std::cout << " occurs in useBB = " << useBB << std::endl;
	}
	StmtSet subSet;
	OA_ptr<UDDUChains::Interface::ChainStmtIterator> defStmtIterPtr;
	defStmtIterPtr = udChains->getUDChainStmtIterator(use);
	// iterate through all possible definitions: 
	for (; defStmtIterPtr->isValid(); (*defStmtIterPtr)++) {
	  StmtHandle defStmt = defStmtIterPtr->current();
	  if (debug) {
	    std::cout << "  def stmt (" << defStmt.hval() << ") = " 
		      << mIR->toString(defStmt) << std::endl;
	    std::cout << "    mStmtToBBMap[defStmt] = " << mStmtToBBMap[defStmt] 
		      << std::endl;
	  }
	  if (donotfilterBB) {
	    subSet.insert(defStmt);
	    if (debug) { std::cout << "    Filter off - inserting def stmt" << std::endl; }
	  }
	  else { // do filter 
	    // determine the subset of the def statements that are
	    // within the same basic block, include the StmtHandle(0) if
	    // a def statement is not within the same basic block
	    // or comes after the use statement
	    if (mStmtToBBMap[defStmt]==useBB) {
	      if (debug) { std::cout << "  Filter on - stmts are in same basic block" << std::endl; }
	      // iterate over statements in this basic block, if hit def
	      // first then insert it, if hit use first then insert StmtHandle(0)
	      OA_ptr<CFG::NodeStatementsIteratorInterface> stmtIterPtr 
		= useBB->getNodeStatementsIterator();
	      // looping over statements in basic blocks
	      for (; stmtIterPtr->isValid(); ++(*stmtIterPtr)) {
		OA::StmtHandle stmt = stmtIterPtr->current();
		if (!haveDefinitionBeforeUse && stmt == mMemRefToStmt[use]) {
		  if (debug) { std::cout << "Filter on - out of order - inserting StmtHandle(0)" << std::endl; }
		  subSet.insert(StmtHandle(0)); // outside of block definition
		  haveDefinitionBeforeUse=true;
		  break;
		} 
		if (stmt == defStmt) {
		  if (debug) { std::cout << "    Filter on - inserting def stmt" << std::endl; }
		  subSet.insert(defStmt);
		  haveDefinitionBeforeUse=true;
		  break;
		} 
	      }
	      // not in same basic block
	    } else {
	      if (debug) { std::cout << "    Filter on - outside def - inserting StmtHandle(0)" << std::endl; }
	      subSet.insert(StmtHandle(0));
	      haveDefinitionBeforeUse=true;
	    }
	  }
	} // end for def stmts
	// see if the subset is already in a UDDUChainsXAIF data structure
	int chainId = aUDDUChainsXAIF->findChain(subSet);
	if (chainId != UDDUChainsXAIF::CHAIN_ID_NONE) {
	  // if it is then insert the use memory reference into that chain
	  aUDDUChainsXAIF->insertInto(use, chainId);
	  if (debug) { std::cout << "  found as existing chain id: " << chainId << std::endl; }
	} else {
	  // if it isn't then make a new chain in the UDDUChainsXAIF 
	  // and add the subset of statements into that chain
	  int newChainId = getNextChainId();
	  aUDDUChainsXAIF->insertInto(use,newChainId);
	  if (debug) { std::cout << "  make new chain id: " << newChainId << std::endl; }
	  aUDDUChainsXAIF->addStmtSet(subSet,newChainId);
	}
      } // end for use stmts
      // for each DUChain starting from a def memory reference
      OA_ptr<UDDUChains::Interface::MemRefsWithDUChainIterator> defIterPtr; 
      defIterPtr = udChains->getMemRefsWithDUChainIterator();
      for (; defIterPtr->isValid(); ++(*defIterPtr)) {
	bool haveUseAfterDefinition=false; 
	MemRefHandle def = defIterPtr->current();
	if (debug) {
	  std::cout << "def memref (" << def.hval() << ") = " << mIR->toString(def);
	  std::cout << std::endl;
	}
	// BB for def
	OA_ptr<CFG::NodeInterface> defBB 
	  = mStmtToBBMap[mMemRefToStmt[def]];
	if (debug) {
	  std::cout << " occurs in defBB = " << defBB << std::endl;
	}
	// determine the subset of the use statements that are
	// within the same basic block, include the StmtHandle(0) if
	// a statement is not within the same basic block
	StmtSet subSet;
	OA_ptr<UDDUChains::Interface::ChainStmtIterator> useStmtIterPtr;
	useStmtIterPtr = udChains->getDUChainStmtIterator(def);
	for (; useStmtIterPtr->isValid(); (*useStmtIterPtr)++) {
	  StmtHandle useStmt = useStmtIterPtr->current();
	  if (debug) {
	    std::cout << "  useStmt (" << useStmt.hval() << ") = " 
		      << mIR->toString(useStmt) << std::endl; 
	    std::cout << "    mStmtToBBMap[useStmt] = " << mStmtToBBMap[useStmt];
	    std::cout << std::endl;
	  }
	  if (donotfilterBB) { 
	    subSet.insert(useStmt);
	    if (debug) { std::cout << "    Filter off - inserting use stmt" << std::endl; }
	  } 
	  else { // filtered
	    // determine the subset of use statements that are
	    // within the same basic block, include the StmtHandle(0) if
	    // a use statement is not within the same basic block
	    // or comes before the def statement
	    if (mStmtToBBMap[useStmt]==defBB) { // in same basic block
	      if (debug) { std::cout << "    Filter on - stmts are in same basic block" << std::endl; }
	      // iterate over statements in this basic block, if hit def
	      // first then insert use, if hit use first then insert StmtHandle(0)
	      OA_ptr<CFG::NodeStatementsIteratorInterface> stmtIterPtr 
		= defBB->getNodeStatementsIterator();
	      // looping over statements in basic blocks
	      for (; stmtIterPtr->isValid(); ++(*stmtIterPtr)) {
		OA::StmtHandle stmt = stmtIterPtr->current();
		if (!haveUseAfterDefinition && stmt == useStmt) {
		  if (debug) { std::cout << "    Filter on - out of order - inserting StmtHandle(0)" << std::endl; }
		  subSet.insert(StmtHandle(0));
		  haveUseAfterDefinition=true;
		  break;
		} 
		if (stmt == mMemRefToStmt[def]) {
		  if (debug) { std::cout << "    Filter on - inserting use stmt" << std::endl; }
		  subSet.insert(useStmt);
		  haveUseAfterDefinition=true;
		  break;
		} 
	      }
	    } 
	    else { //  not in same basic block
	      if (debug) { std::cout << "    Filter on - outside use - inserting StmtHandle(0)" << std::endl; }
	      subSet.insert(StmtHandle(0));
	      haveUseAfterDefinition=true;
	    }
	  }
	} // use stmts
	// see if the subset is already in a UDDUChainsXAIF data structure
	int chainId = aUDDUChainsXAIF->findChain(subSet);
	if (chainId != UDDUChainsXAIF::CHAIN_ID_NONE) {
	  // if it is then insert the use memory reference into that chain
	  aUDDUChainsXAIF->insertInto(def, chainId);
          if (debug) { std::cout << "  found as existing chain id: " << chainId << std::endl; }
	} 
	else {
	  // if it isn't then make a new chain in the UDDUChainsXAIF 
	  // and add the subset of statements into that chain
	  int newChainId = getNextChainId();
	  aUDDUChainsXAIF->insertInto(def,newChainId);
	  aUDDUChainsXAIF->addStmtSet(subSet,newChainId);
          if (debug) { std::cout << "  make new chain id: " << newChainId << std::endl; }
	}
      } // def stmts
      // insert an empty chain as chain 0 because that is the default chain in XAIF
      // don't insert it until the end because while building, we don't want to find
      // empty chains at 0, want to find it at 1
      StmtSet emptySet;
      aUDDUChainsXAIF->addStmtSet(emptySet,0);
      return aUDDUChainsXAIF;
    }

    int ManagerUDDUChainsXAIF::ourCurrentStartId=3;

    int ManagerUDDUChainsXAIF::getNextChainId() {
      return ourCurrentStartId++;
    }
      
  } 

} 

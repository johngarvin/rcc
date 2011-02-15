/*! \file
  
  \brief Implementation of SSAStandard.

  \authors Arun Chauhan (2001 as part of Mint), Nathan Tallent, Michelle Strout
  \version $Id: SSAStandard.cpp,v 1.5 2005/06/10 02:32:05 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/


// standard headers

#ifdef NO_STD_CHEADERS
# include <stdlib.h>
# include <string.h>
# include <assert.h>
#else
# include <cstdlib>
# include <cstring>
# include <cassert>
using namespace std; // For compatibility with non-std C headers
#endif

#include <iostream>
using std::ostream;
using std::endl;
using std::cout;
using std::cerr;

#include "SSAStandard.hpp"

namespace OA {
  namespace SSA {

static bool debug = false;

//--------------------------------------------------------------------
    
    
//-----------------------------------------------------------------------------
/*! SSA constructor computes the dominator tree, and dominance
    frontiers from it, for the given CFG.  Using the dominance
    frontiers, it then inserts phi functions using the following
    algorithm (from the book "Engineering a Compiler", by Keith
    D. Cooper and Linda Torczon, chapter "Data-flow Analysis").  The
    set NonLocals is the set of all upwardly exposed uses in each node
    of the CFG and is computed using the CFG method
    "compute_uses_sets".  The map Blocks maps a variable name i to the
    set of CFG nodes (basic blocks) where it is defined.

    <pre>
    for each name i in NonLocals
        WorkList <-- Blocks(i)
        for each block b in WorkList
            for each block d in Dominance_Frontier(b)
                insert a phi function for i in d
                WorkList <-- WorkList + d
    </pre>
*/
SSAStandard::SSAStandard(const SymHandle name_,
			 OA_ptr<SSAIRInterface> ir_,
			 OA_ptr<CFG::CFGInterface> cfg_)
  : name(name_), cfg(cfg_), mIR(ir_)
{
  OA_DEBUG_CTRL_MACRO("DEBUG_SSAStandard:ALL", debug);

  DomTree dt(cfg, cfg->getEntry());
  compute_uses_sets();
  dt.compute_dominance_frontiers();
  if (debug) {
    cout << "====================" << endl 
	 << "Dominator tree " << endl 
	 << "--------------------" << endl;
    dt.dump(cout);
    cout << "====================" << endl;
  }

  // insert Phi functions where needed
  OA_ptr<NonLocalsIterator> i_it = getNonLocalsIterator();
  std::set<OA_ptr<CFG::NodeInterface> > work_list;
  std::set<OA_ptr<CFG::NodeInterface> > blks_with_phi;
  for ( ; i_it->isValid(); ++(*i_it)) {
    work_list.clear();
    blks_with_phi.clear();
    SymHandle var_name = mIR->getSymHandle(i_it->current()); // FIXME
    OA_ptr<DefBlocksIterator> block_it = getDefBlocksIterator(var_name);
    for ( ; block_it->isValid(); ++(*block_it)) {
      work_list.insert(block_it->current());
    }
    std::set<OA_ptr<CFG::NodeInterface> >::iterator b_it = work_list.begin();
    for ( ; b_it != work_list.end(); ++b_it) {
      OA_ptr<CFG::NodeInterface> cfgnode = (*b_it);
      OA_ptr<DGraph::NodeInterface> n1 = 
        cfgnode.convert<DGraph::NodeInterface>();
      OA_ptr<DomTree::Node> n2 = dt.domtree_node(n1);
      
      OA_ptr<DomTree::DomFrontIterator> d_it = n2->getDomFrontIterator();
      for ( ; d_it->isValid(); ++(*d_it)) {
        OA_ptr<DomTree::Node> n = d_it->current();
        OA_ptr<DGraph::NodeInterface> dnode = n->getGraphNode();
        OA_ptr<CFG::NodeInterface> blk = 
          dnode.convert<CFG::NodeInterface>();
	if (blks_with_phi.find(blk) == blks_with_phi.end()) {
	  OA_ptr<SSA::Phi> p; p = new SSA::Phi(var_name, cfg);
	  phi_node_sets[blk].insert(p);
	  if (debug) {
	    cout << "inserted Phi node ";
	    p->dump(cout);
	    cout << " in node ";
	    //blk->dumpbase(cout);
	    cout << endl;
	  }
	  if (work_list.find(blk) == work_list.end()) {
	    work_list.insert(blk);
	  }
	}
      }
    }
  }
}
//-----------------------------------------------------------------------------

SSAStandard::~SSAStandard()
{
  // FIXME: left empty for now
}

//-----------------------------------------------------------------------------

/** Compute "uses" set for each basic block (one CFG node corresponds
    to one basic block).  The set of uses is computed by traversing
    the statements in each node and eliminating the uses that get
    killed.  In other words, only upwardly exposed uses are computed.
    Simultaneously, for each variable, the set of blocks that defines
    that variable, is also built (def_blocks_set).
*/
void
SSAStandard::compute_uses_sets()
{
  std::set<SymHandle> kill_set;
  
  OA_ptr<DGraph::NodesIteratorInterface> nodes_iter = cfg->getNodesIterator();
  for ( ; nodes_iter->isValid(); ++(*nodes_iter)) {
    kill_set.clear();
    OA_ptr<DGraph::NodeInterface> dn = nodes_iter->current();
    OA_ptr<CFG::Node> n = dn.convert<CFG::Node>();
    
    OA_ptr<CFG::NodeStatementsIteratorInterface> s_iter = 
      n->getNodeStatementsIterator();
    for ( ; s_iter->isValid(); ++(*s_iter)) {
      OA_ptr<IRUseDefIterator> use_it = mIR->getUses(s_iter->current());
      for ( ; use_it->isValid(); ++(*use_it)) {
	LeafHandle s = use_it->current();
	if (kill_set.find(mIR->getSymHandle(s)) == kill_set.end()) {
	  // the variable does not exist in the kill set
	  if (non_locals.find(s) == non_locals.end()) {
	    // insert only if not already in the set, otherwise, it
	    // can get duplicated
	    non_locals.insert(s);
	  }
	}
      }

      OA_ptr<IRUseDefIterator> def_it = mIR->getDefs(s_iter->current());
      for ( ; def_it->isValid(); ++(*def_it)) {
	LeafHandle s = def_it->current();
        SymHandle s_name = mIR->getSymHandle(s);
	kill_set.insert(s_name);
	if (def_blocks_set[s_name].find(n) == def_blocks_set[s_name].end()) {
	  def_blocks_set[s_name].insert(n);
	}
      }
    }
  }
}

//-----------------------------------------------------------------------------
void
SSAStandard::dump(ostream& os)
{
  os << "===== SSAStandard dump =====" << endl
     << mIR->toString(name) << endl
     << "--------------------" << endl;
  
  // dump each individual CFG node and the phi nodes associated with each node
  OA_ptr<DGraph::NodesIteratorInterface> cfgnode_it = cfg->getNodesIterator();
  for ( ; cfgnode_it->isValid(); ++(*cfgnode_it)) {
    OA_ptr<DGraph::NodeInterface> dn = cfgnode_it->current();
    OA_ptr<CFG::Node> n = dn.convert<CFG::Node>();
    //n->longdump(cfg, os); //FIXME

    OA_ptr<PhiNodesIterator> phi_it = getPhiNodesIterator(n);
    if (phi_it->isValid()) {
      os << "Phi Nodes:" << endl;
      for ( ; phi_it->isValid(); ++(*phi_it)) {
	os << "    ";
	phi_it->current()->dump(os);
	os << endl;
      }
    }
    os << endl;
  }
  os << endl;
}

//-----------------------------------------------------------------------------


  } // end of SSA namespace
} // end of OA namespace

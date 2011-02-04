/*! \file
  
  \brief Tarjan's algorithm to determine strongly connected components
         of a directed graph. This was the first algorithm that solved
         the problem in linear time.

  \authors Original DSystem code by John Mellor-Crummey, Lei Zhou;
           Ported to OpenAnalysis by Nathan Tallent
  \version $Id: SCC.cpp,v 1.9 2005/06/08 20:31:50 eraxxon Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  The algorithm uses a push-down STACK. With every node in the graph,
  three variables are associated: visited, dfn, link.  Furthermore, the
  algorithm uses counters df-count and r.
*/


//************************** System Include Files ***************************

#include <cassert>

#include <map>
#include <set>
#include <stack>

//*************************** User Include Files ****************************

#include <OpenAnalysis/Utils/SCC.hpp>
#include <OpenAnalysis/Utils/Util.hpp>

//*************************** Forward Declarations **************************

//------------------------------------------------------------------------
// LowLinkState is a private data structure used for finding SCC
// components.
//------------------------------------------------------------------------

namespace OA {

class LowLinkState {
public:
  LowLinkState(OA::OA_ptr<OA::RIFG> rifg_) : rifg(rifg_), mCount(1) { 
    unsigned int sz = rifg->getHighWaterMarkNodeId() + 1;
    mNodeStatus = new SCCNodeStatus[sz];
  }
  
  ~LowLinkState() { 
    delete[] mNodeStatus;
  }
  
  // Mark methods
  void Mark(OA::RIFG::NodeId nid) 
    { mOld.insert(nid); }
  unsigned int IsMarked(OA::RIFG::NodeId nid) { 
    MyNodeIdSet::const_iterator it = mOld.find(nid);
    return (it != mOld.end()); 
  }

  // Lowlink and depth-first methods
  unsigned int& LOWLINK(OA::RIFG::NodeId nid) 
    { return mNodeStatus[nid].lowlink; }

  unsigned int& DFNUMBER(OA::RIFG::NodeId nid) 
    { return mNodeStatus[nid].dfnumber; }
  
  unsigned int& Count() 
    { return mCount; }
  
  // Stack methods
  void Push(OA_ptr<DGraph::Interface::Node> node, OA::RIFG::NodeId nid) {
    mNodeStatus[nid].inStack = true;
    mNodeStack.push(node);
  }
  
  OA_ptr<DGraph::Interface::Node> Top()
    { return mNodeStack.top(); }

  OA_ptr<DGraph::Interface::Node> Pop() {
    OA_ptr<DGraph::Interface::Node> node = mNodeStack.top();
    OA::RIFG::NodeId nid = rifg->getNodeId(node);
    mNodeStack.pop();
    mNodeStatus[nid].inStack = false;
    return node; 
  }
  
  bool IsOnStack(OA::RIFG::NodeId nid) 
    { return mNodeStatus[nid].inStack; }

private:
  
  // SCCNodeStatus is a data structure to describe the current status
  // of a SCC node.
  class SCCNodeStatus {
  public:
    SCCNodeStatus(unsigned int l=0, unsigned int d=0, bool i=false)
      : lowlink(l), dfnumber(d), inStack(i) { }
    
    unsigned int lowlink;
    unsigned int dfnumber;
    bool inStack;
  };

  typedef std::set<OA::RIFG::NodeId> MyNodeIdSet;

  typedef std::stack<OA_ptr<DGraph::Interface::Node> > MyNodeStack;
  
private:
  OA::OA_ptr<OA::RIFG> rifg;

  unsigned int mCount;
  SCCNodeStatus* mNodeStatus; // indexed by OA::RIFG::NodeId
  MyNodeIdSet mOld;
  MyNodeStack mNodeStack;
};

} // end of namespace OA


//*************************** Forward Declarations **************************

static void 
CreateHelper(OA::SCCSet* sccSet, 
	     OA::OA_ptr<OA::RIFG> rifg,
	     OA::OA_ptr<OA::DGraph::Interface::Node> v, 
	     OA::RIFG::NodeId vid,
	     OA::LowLinkState* state);


//***************************************************************************


//***************************************************************************
// SCCSet
//***************************************************************************

namespace OA {

SCCSet::SCCSet() 
{
}


SCCSet::SCCSet(OA_ptr<DGraph::Interface> graph,
	       OA_ptr<RIFG> rifg) 
{
  if (rifg.ptrEqual(0)) {
    rifg = new OA::RIFG(graph, RIFG::getSourceNode(graph), 
			RIFG::getSinkNode(graph));
  }
  // assert(rifg->getGraph() == graph);
  Create(graph, rifg);
}


void
SCCSet::Create(OA_ptr<DGraph::Interface> graph,
	       OA_ptr<RIFG> rifg)
{
  // Note: We use a RIFG in order to get dense node ids between 1 and n.
  LowLinkState *state = new LowLinkState(rifg);
  //LowLinkState *state = &ST;
  
  // if there exists a node not marked yet, visit it.
  OA_ptr<DGraph::Interface::NodesIterator> it = graph->getNodesIterator();
  for (; it->isValid(); ++(*it)) {
    OA_ptr<DGraph::Interface::Node> node = it->current();
    OA::RIFG::NodeId nid = rifg->getNodeId(node);
    if ( !state->IsMarked(nid) ) {
      CreateHelper(this, rifg, node, nid, state);
    }
  }
}


SCCSet::~SCCSet()
{
  Destroy();
}


void
SCCSet::Destroy()
{
  this->clear();
}


OA_ptr<SCCNodeSet> 
SCCSet::NodeToSCC(const OA_ptr<DGraph::Interface::Node> node)
{
  // N.B.: The implementation is a linear search and direct
  // translation from the DSystem code.  However, if this is going to
  // be used a lot, we should probably create a map.
  for (self_t::const_iterator it = this->begin(); it != this->end(); ++it) {
    OA_ptr<SCCNodeSet> scc = *it;
    if (scc->find(node) != scc->end()) {
      return scc;
    }
  }
  assert(false);               // every node should be in a set
  return OA_ptr<SCCNodeSet>(); // never reached!
}


void 
SCCSet::dump(std::ostream& os)
{
  using std::endl;

  os << "{ SCCSet: num SCC sets: " << this->size() << endl;
  for (self_t::iterator it = this->begin(); it != this->end(); ++it) {
    OA_ptr<SCCNodeSet> scc = *it;
    
    os << "  { SCC: ";
    SCCNodeSet::iterator sccIt = scc->begin();
    for ( ; sccIt != scc->end(); ++sccIt) {
      OA_ptr<DGraph::Interface::Node> node = *sccIt;
      os << node->getId() << " ";
    }
    os << "}" << endl;
  } 
}


} // end of namespace OA


//***************************************************************************
// SCCSet helpers
//***************************************************************************

static void 
CreateHelper(OA::SCCSet* sccSet, 
	     OA::OA_ptr<OA::RIFG> rifg,
	     OA::OA_ptr<OA::DGraph::Interface::Node> v, 
	     OA::RIFG::NodeId vid,
	     OA::LowLinkState* state)
{
  using namespace OA;
  
  state->Mark(vid);                           // mark v
  state->DFNUMBER(vid) = state->Count();      // set dfnumber
  state->Count()++;                           // increment count by 1
  state->LOWLINK(vid) = state->DFNUMBER(vid); // set lowlink number
  state->Push(v, vid);                        // push v into stack
  
  // for each w which is on the adjacency list of v, there is an edge v->w
  OA::OA_ptr<OA::DGraph::Interface::NodesIterator> it = 
    v->getSinkNodesIterator();
  for (; it->isValid(); ++(*it)) {
    OA_ptr<DGraph::Interface::Node> w = it->current();
    OA::RIFG::NodeId wid = rifg->getNodeId(w);
    
    // if w is not marked yet
    if (!state->IsMarked(wid)) { 
      CreateHelper(sccSet, rifg, w, wid, state); 
      state->LOWLINK(vid) = OA_MIN(state->LOWLINK(vid), state->LOWLINK(wid));
    } 
    // w is already marked
    else {
      if ( state->DFNUMBER(wid) < state->DFNUMBER(vid) && 
	   state->IsOnStack(wid) ) {
     	state->LOWLINK(vid) = OA_MIN(state->DFNUMBER(wid), 
				     state->LOWLINK(vid));
      }
    }
  }

  // if lowlink[v] == DFNumber[v], the current Node in stack above
  // v form a SCC. 
  if ( state->LOWLINK(vid) == state->DFNUMBER(vid) ) {
    OA_ptr<SCCNodeSet> scc; scc = new SCCNodeSet;
    
    // pop x from top of stack, until x == v
    OA_ptr<DGraph::Interface::Node> x;
    do {
      x = state->Pop();
      scc->insert(x);
    } while (x != v);

    // the SCC is complete now, add it to sccSet.
    sccSet->insert(scc);
  }
}


/*! \file
  
  \brief This file contains code to determine the Tarjan intervals
         (nested strongly connected regions) of a flow graph.
         Tarjan's original algorithm is described in the following
         article:
           Tarjan, R.E. "Testing flow graph reducibility"
           J. of Computer & System Sciences 9, 355-365, 1974. 
  
         Extensions to Tarjan's algorithm were made here to handle
         reducible SCRs surrounded by or containing irreducible ones.
         The extensions are described in the following article:
           Havlak, P. "Nesting of reducible and irreducible loops"
           ACM TOPLAS, July, 1997.
  
         The result is returned as a tree of SCRs, where a parent SCR
         contains its children.  A leaf SCR is a single node, which is
         not really a loop unless the isCyclic flag is set.
  
         Preconditions:
         1. The underlying directed graph has a unique start node,
            dominating all others in the graph and a unique exit node,
            postdominating all others.
        
  \authors Original DSystem code by phh (Paul Havlak?)
           Port to 'Classic OpenAnalysis' by John Mellor-Crummey,
             Jason Eckhardt
           Port to OpenAnalysis by Nathan Tallent (renamed NestedSCR
	     from TarjanIntervals)
  \version $Id: NestedSCR.cpp,v 1.2 2005/03/14 23:49:26 ntallent Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

//************************** System Include Files ***************************

#include <iostream>
using std::cout;
#include <algorithm>     // for max
#include <map>
#include <list>

#ifdef NO_STD_CHEADERS
# include <string.h>
# include <stdio.h>
# include <limits.h>
#else
# include <cstring>
# include <cstdio>
# include <climits>
using namespace std; // For compatibility with non-std C headers
#endif

#include <unistd.h>

//*************************** User Include Files ****************************

#include <OpenAnalysis/Utils/NestedSCR.hpp>

//*************************** Forward Declarations **************************

const OA::NestedSCR::DFNUM_t DFNUM_ROOT = 0;
const OA::NestedSCR::DFNUM_t DFNUM_NIL  = UINT_MAX;

//*************************** Forward Declarations **************************

namespace OA {

//------------------------------------------------------------------------
// TarjWork: There will be an array of TarjWorks, one for each node.
// These are indexed by the DFS number of the node.  This way, there
// is no dependence on the node numbering of the underlying graph.
//------------------------------------------------------------------------

class TarjWork {
public:
  TarjWork();
  ~TarjWork();

  RIFG::NodeId wk_vertex;	// map from DFS# of vertex to RIFGNodeId
  OA::NestedSCR::DFNUM_t wk_last;   // DFS # of vertex's last descendant
  OA::NestedSCR::DFNUM_t wk_header; // header of the vertex's interval - HIGHPT
  OA::NestedSCR::DFNUM_t wk_nextP;  // next member of P set == reachunder
  OA::NestedSCR::DFNUM_t wk_nextQ;  // next member of Q set == worklist
  bool wk_inP;			// test for membership in P == reachunder
  bool wk_isCyclic;		// has backedges -- to id singles
  bool wk_reducible;		// true if cyclic scr is reducible
  std::list<int> backPreds;	// List of preds that are backedges.
  std::list<int> nonBackPreds;  // List of preds that are non-backedges.
};


TarjWork::TarjWork() 
{
  wk_vertex = RIFG::NIL;
  wk_last   = DFNUM_NIL;
  wk_header = DFNUM_ROOT;
  wk_nextP  = DFNUM_NIL;
  wk_nextQ  = DFNUM_NIL;
  wk_inP = false;
  wk_isCyclic = false;
  wk_reducible = true;
}


TarjWork::~TarjWork() 
{
  backPreds.clear();
  nonBackPreds.clear();
}


//------------------------------------------------------------------------
// Interval tree will consist of an array of TarjTreeNodes. These
// are indexed (internally) by the DFS number of the node.  This way,
// there is no dependence on the node numbering of the underlying graph. 
//
// There is also a map from RIFGNodeIds to DFS numbers, so
// that tree nodes can be accessed (by the client) through their
// node ids. 
//------------------------------------------------------------------------

class TarjTreeNode {
public:
  TarjTreeNode();
  RIFG::NodeId nodeid;	// Associated RIFG::NodeId.
  short level;		// nesting depth -- outermost loop is 1 
  NestedSCR::Node_t type; // acyclic, interval or irreducible 
  OA::NestedSCR::DFNUM_t outer;	 // DFS number of header of containing interval
  OA::NestedSCR::DFNUM_t inners; // DFS number of header of first nested interval
  OA::NestedSCR::DFNUM_t next;	// DFS number of next nested header
  int prenum;			// preorder number
  OA::NestedSCR::DFNUM_t last;	// number of last descendent
  RIFG::NodeId last_id;		// id of last descendent
  short loopIndex;		// unique id for intervals
};


TarjTreeNode::TarjTreeNode()
{
  nodeid = RIFG::NIL;
  level  = 0;
  type   = NestedSCR::NODE_ACYCLIC;
  outer  = DFNUM_NIL;
  inners = DFNUM_NIL;
  next   = DFNUM_NIL;
  prenum = -1;
  last   = DFNUM_NIL;	
  last_id = RIFG::NIL;
  loopIndex = -1;
}

} // end of namespace OA



//***************************************************************************
// NestedSCR
//***************************************************************************

namespace OA {
  
  // Refers to members in TarjTreeNode
#define TARJ_nodeid(name)	(tarj[name].nodeid)
#define TARJ_outer(name)	(tarj[name].outer)
#define TARJ_inners(name)	(tarj[name].inners)
#define TARJ_next(name)		(tarj[name].next)
#define TARJ_level(name)	(tarj[name].level)
#define TARJ_type(name)		(tarj[name].type)
#define TARJ_last(name)		(tarj[name].last)
#define TARJ_last_id(name)	(tarj[name].last_id)
#define TARJ_loopIndex(name)	(tarj[name].loopIndex)
#define TARJ_contains(a,b)	\
    ( ( tarj[a].prenum <= tarj[b].prenum ) && \
      ( tarj[b].prenum <= tarj[tarj[a].last].prenum ) \
    )

  // Refers to members in TarjWork
#define vertex(x)	(wk[x].wk_vertex)
#define TLast(x)	(wk[x].wk_last)
#define header(x)	(wk[x].wk_header)
#define nextP(x)	(wk[x].wk_nextP)
#define nextQ(x)	(wk[x].wk_nextQ)
#define inP(x)		(wk[x].wk_inP)
#define isCyclic(x)	(wk[x].wk_isCyclic)
#define reducible(x)	(wk[x].wk_reducible)
#define backPreds(x)	(wk[x].backPreds)
#define nonBackPreds(x)	(wk[x].nonBackPreds)

static int n;		// next DFS preorder number
static int last_id;	// RIFG::NodeId whose DFS preorder number is n

//
// is_backedge(a,b) returns true if a is a descendant of b in DFST
// (a and b are the DFS numbers of the corresponding vertices). 
//
// Note that this is in the local structures; corresponds somewhat
// to Contains(tarj,b,a) for the public structures.
//
//
#define is_backedge(a,b) ((b <= a) & (a <= TLast(b)))

#define dfnum(v)		(nodeid_to_dfnum_map[v])


NestedSCR::NestedSCR(OA::OA_ptr<OA::RIFG> rifg_)
  : rifg(rifg_)
{
  Create();
}


NestedSCR::~NestedSCR()
{
  if (tarj)
    delete[] tarj;
  if (uf)
    delete uf;
  if (wk)
    delete[] wk;
  rev_top_list.clear();
  nodeid_to_dfnum_map.clear();
}


// Construct tree of Tarjan intervals for the graph
void 
NestedSCR::Create()
{
  RIFG::NodeId root = rifg->getSource();
  
  Init();
  DFS(root);
  FillPredLists();
  GetTarjans();
  Build();
  Sort();
  ComputeIntervalIndex();
  FreeWork();
}


//
// Sort the Tarjan Tree in topological ordering
//
// Feb. 2002 (jle) - Removed the call to GetTopologicalMap which computes
// a (breadth-first) topological ordering of the CFG. This is completely
// unnecessary (and wasteful) since by virtue of performing a DFS as part of
// interval building, we have a topological ordering available as a by-product.
// Further, the breadth-first version in GetTopologicalMap is clumsy (in this
// context) since it has to make call-backs to the tarjan code to determine
// if the current edge being examined is a back-edge (to avoid cycling).
//
void 
NestedSCR::Sort()
{
  RIFG::NodeId gId;
  int parent;		// Tarj parent (loop header)

  // Now disconnect all "next" fields for all tarj nodes

  OA_ptr<RIFG::NodesIterator> ni = rifg->getNodesIterator();
  for ( ; (ni->isValid()); ++(*ni)) {
    gId = ni->current();
    int gdfn = dfnum(gId);
    // gdfn will be invalid if the node gId was unreachable.
    if (gdfn != DFNUM_NIL) {
      TARJ_inners(gdfn) = DFNUM_NIL;
      TARJ_next(gdfn) = DFNUM_NIL;
    }
  }

  // The list is sorted in reverse topological order since each child
  // in the loop below is prepended to the list of children.
  std::list<RIFG::NodeId>::iterator i;
  for (i = rev_top_list.begin(); i != rev_top_list.end(); i++) {
    gId = *i;
    if (gId != RIFG::NIL) {
      // Add new kid
      int gdfn = dfnum(gId);
      parent = TARJ_outer(gdfn);
      if (parent != DFNUM_NIL) {
        TARJ_next(gdfn) = TARJ_inners(parent);
        TARJ_inners(parent) = gdfn;
      }
    }
  }

  Renumber();
} // end of tarj_sort()


TarjTreeNode* 
NestedSCR::getTree()
{
  return tarj;
}



//
// Allocate and initialize structures for algorithm, and UNION-FIND
//
void 
NestedSCR::Init()
{
  unsigned int g_size = rifg->getHighWaterMarkNodeId() + 1;

  n = DFNUM_ROOT;

  //
  // Local work space
  //
  wk = new TarjWork[g_size];
  uf = new UnionFindUniverse(g_size);
  tarj = new TarjTreeNode[g_size];
  InitArrays();
}


//
// Initialize only nodes in current instance g
//
void 
NestedSCR::InitArrays()
{
  OA_ptr<RIFG::NodesIterator> ni = rifg->getNodesIterator();
  for ( ; ni->isValid(); ++(*ni)) {
    int nid = ni->current();
    dfnum(nid) = DFNUM_NIL;
  }
}


//
// Do depth first search on control flow graph to 
// initialize vertex[], dfnum[], last[]
//
void 
NestedSCR::DFS(RIFG::NodeId v)
{
  vertex(n) = v;
  dfnum(v)  = n++;
 
  RIFG::EdgeId succ;
  OA_ptr<RIFG::OutgoingEdgesIterator> ei = rifg->getOutgoingEdgesIterator(v);
  for (; (ei->isValid()); ++(*ei)) {
    succ = ei->current();
    int son = rifg->getEdgeSink(succ);
    if (dfnum(son) == DFNUM_NIL)
      DFS(son);
  }

  //
  // Equivalent to # of descendants -- number of last descendant
  //
  TLast(dfnum(v)) = n-1;
  rev_top_list.push_back(v);
}


// Fill in the backPreds and nonBackPreds lists for each node.
void 
NestedSCR::FillPredLists()
{
  for (int i = DFNUM_ROOT; i < n; i++) {
    OA_ptr<RIFG::IncomingEdgesIterator> ei = 
      rifg->getIncomingEdgesIterator(vertex(i)  );
    for ( ; (ei->isValid()); ++(*ei)) {
      int prednum = dfnum(rifg->getEdgeSrc(ei->current()));
      if (is_backedge(prednum, i)) {
        backPreds(i).push_back(prednum); 
      } else {
        nonBackPreds(i).push_back(prednum); 
      }
    } // for preds
  } // for nodes
}


//
// In bottom-up traversal of depth-first spanning tree, 
// determine nested strongly connected regions and flag irreducible regions.
//                                                    phh, 4 Mar 91
//
void 
NestedSCR::GetTarjans()
{
  int w;			// DFS number of current vertex
  DFNUM_t firstP, firstQ;	// set and worklist

  //
  // Following loop should skip root (prenumbered as 0)
  //
  for (w = n - 1; w != DFNUM_ROOT; w--) // loop c
    //
    // skip any nodes freed or not reachable
    //
    if (w != DFNUM_NIL && rifg->isValid(vertex(w))) {
      firstP = firstQ = DFNUM_NIL;
      //
      // Add sources of cycle arcs to P -- and to worklist Q
      //
      std::list<int>::iterator prednum; 
      for (prednum = backPreds(w).begin(); prednum != backPreds(w).end();
           prednum++) { // loop d
        int u,v;			// vertex names
        v = *prednum;
        // ignore predecessors not reachable
        if (v != DFNUM_NIL)
        {
          if (v == w) {
            //
            // Don't add w to its own P and Q sets
            //
            isCyclic(w) = true;
          } else {
            //
            // Add FIND(v) to P and Q
            //
            u = FIND(v);
            if (!inP(u)) {
              nextP(u) = nextQ(u) = firstP;
              firstP   = firstQ   = u;
              inP(u)   = true;
            } // if (!inP(u))
          } // if (v == w)
        } // if 
      } // for preds

      //
      // P nonempty -> w is header of a loop
      //
      if (firstP != DFNUM_NIL)
        isCyclic(w) = true;

      while (firstQ != DFNUM_NIL) {
        int x, y, yy;		// DFS nums of vertices

        x = firstQ;
        firstQ = nextQ(x);	// remove x from worklist

        //
        // Now look at non-cycle arcs
        //
        std::list<int>::iterator prednum;
        for (prednum = nonBackPreds(x).begin();
             prednum != nonBackPreds(x).end();
             prednum++) { // loop d
          y = *prednum;

          //
          // ignore predecessors not reachable
          //
          if (y != DFNUM_NIL)
          {
            //
            // Add FIND(y) to P and Q
            //
            yy = FIND(y);

            if (is_backedge(yy, w)) {
              if ((!inP(yy)) & (yy != w)) {
                nextP(yy) = firstP;
                nextQ(yy) = firstQ;
                firstP = firstQ = yy;
                inP(yy) = true;
              }
              //
              // Slight change to published alg'm...
              // moved setting of header (HIGHPT)
              // from here to after the union.
              //
            } else {
              //
              // Irreducible region!
              //
              reducible(w) = false;
#if 1
              // FIXME: The DSystem version of the code did not have the
              // following line.  However, this line IS in the 1997 article,
              // and I believe it is necessary (jle, 03-02-2002).
              nonBackPreds(w).push_back(yy);
#endif
            }
          }
        }
      }
      //
      // now P = P(w) as in Tarjan's paper
      //
      while (firstP != DFNUM_NIL) {
        //
        // First line is a change to published algorithm;
        // Want sources of cycle edges to have header w
        // and w itself not to have header w.
        //
        if ((header(firstP) == DFNUM_ROOT) & (firstP != w))
          header(firstP) = w;
        UNION(firstP, w, w);
        inP(firstP) = false;
        firstP = nextP(firstP);
      } // while
    } // if
}


//
// Traverse df spanning tree, building tree of Tarjan intervals
//
void 
NestedSCR::Build()
{
  int w;		// DFS number of current vertex
  int outer;		// DFS number header of surrounding loop

  TARJ_nodeid(DFNUM_ROOT) = rifg->getSource();
  //
  // Let the root of the tree be the root of the instance...
  // Following loop can skip the root (prenumbered 0)
  //
  for (w = DFNUM_ROOT + 1; w < n; w++) {
    RIFG::NodeId wnode = vertex(w);
    //
    // skip any nodes not in current instance g
    //
    if (wnode != RIFG::NIL) {
      outer = header(w);
      TARJ_outer(w) = outer;
      TARJ_nodeid(w) = wnode;
      //
      // tarj[w].inners = DFNUM_NIL;  % done in InitArrays
      //
      if (isCyclic(w)) {
        //
        // Level is deeper than outer if this is a loop
        //
        if (reducible(w)) {
          TARJ_type(w) = NODE_INTERVAL;
          TARJ_level(w) = TARJ_level(outer) + 1;
        } else {
          TARJ_type(w) = NODE_IRREDUCIBLE;
          TARJ_level(w) = TARJ_level(outer);
        }
      } else {
        //
        // tarj[w].type  = RI_TARJ_ACYCLIC;	% done in InitArrays
        //
        TARJ_level(w) = TARJ_level(outer);
      }
      TARJ_next(w) = TARJ_inners(outer);
      TARJ_inners(outer) = w;
    }
  }
  n = 0;
  Prenumber(DFNUM_ROOT);
}


void 
NestedSCR::Prenumber(int v)
{
  int inner;

  tarj[v].prenum = ++n;
  last_id = TARJ_nodeid(v);
    
  for (inner = TARJ_inners(v); inner != DFNUM_NIL; 
       inner = TARJ_next(inner)) {
    Prenumber(inner);
  }

  /* tarj[v].last = n;	// 3/18/93 RvH: switch to RIFG::NodeId last_id */
  tarj[v].last_id = last_id;
  tarj[v].last = dfnum(last_id);
}


void 
NestedSCR::Renumber()
{
  tarj = tarj;
  n = 0;
  Prenumber(DFNUM_ROOT);
}


//  Free all space used in computation of 
//  Tarjan interval tree (but not tree itself)
void 
NestedSCR::FreeWork()
{
  delete[] wk;
  wk = 0;

  delete uf;
  uf = 0;
}


static int loopIndex;
void 
NestedSCR::ComputeIntervalIndexSubTree(int node, int value)
{
  int kid, valKid;
    
  TARJ_loopIndex(node) = value;
  if (TARJ_inners(node) != DFNUM_NIL)
    valKid = ++loopIndex;

  for (kid = TARJ_inners(node); kid != DFNUM_NIL; kid = TARJ_next(kid))
    ComputeIntervalIndexSubTree(kid, valKid);
}


void 
NestedSCR::ComputeIntervalIndex()
{
  loopIndex = 0;
  ComputeIntervalIndexSubTree(DFNUM_ROOT, loopIndex);
}


void 
NestedSCR::dump(std::ostream& os)
{
  printf("Tarjan interval tree: <node id>(level,type)::IntervalIndex\n");
  DumpSubTree(os, DFNUM_ROOT, 0 /* Indent */);
}


//
// Feb. 2002 (jle): Got rid of all the silly concatenating of blank
// strings. The %*s format specifier is far simpler for creating indentations.
//
void 
NestedSCR::DumpSubTree(std::ostream& os, int node, int indent)
{
  static const char *NodeType[] = {"NOTHING", "Acyclic",
				   "Interval", "Irreducible"};
  //
  // Indent by three
  //
  if (indent < 72)
    indent += 3;
  
  printf("%*s%d(%d,%s)::%d\n", indent, " ",
         TARJ_nodeid(node), TARJ_level(node),
         NodeType[(int) (TARJ_type(node))], TARJ_loopIndex(node));
  
  for (int kid = TARJ_inners(node); kid != DFNUM_NIL; 
       kid = TARJ_next(kid)) {
    DumpSubTree(os, kid, indent);
  }
  
  //
  // Unindent
  //
  if (indent >= 3)
    indent -= 3;
}


int 
NestedSCR::FIND(int v)
{
  return uf->Find(v);
}


void 
NestedSCR::UNION(int i, int j, int k)
{
  uf->Union(i,j,k);
}


//
// Return number of loops exited in traversing g edge from src to sink
// (0 is normal).
//
int 
NestedSCR::getExits(RIFG::NodeId src, RIFG::NodeId sink)
{
  RIFG::NodeId lca = LCA(src, sink);
  if (lca == RIFG::NIL)
    return 0;

  int dfn_src = dfnum(src);
  int dfn_lca = dfnum(lca);
  return std::max(0, TARJ_level(dfn_src) - TARJ_level(dfn_lca));
}


//
// Return outermost loop exited in traversing g edge from src to sink
// (RIFG::NIL if no loops exited).
//
RIFG::NodeId 
NestedSCR::getLoopExited(RIFG::NodeId src, RIFG::NodeId sink)
{
  RIFG::NodeId lca = LCA(src, sink);
  if (lca == RIFG::NIL)
    return RIFG::NIL;

  if (lca == src)
    return RIFG::NIL;

  int dfn_src = dfnum(src);
  while (!Contains(TARJ_nodeid(TARJ_outer(dfn_src)), sink))
    dfn_src = TARJ_outer(dfn_src);
    
  if (TARJ_type(dfn_src) == NODE_INTERVAL
      || TARJ_type(dfn_src) == NODE_IRREDUCIBLE)
    return TARJ_nodeid(dfn_src);

  return RIFG::NIL;
}


//
// Return type of g edge from src to sink, one of
//   RI_TARJ_NORMAL
//   RI_TARJ_LOOP_ENTRY
//   RI_TARJ_IRRED_ENTRY
//   RI_TARJ_ITERATE (back edge)
//
NestedSCR::Edge_t 
NestedSCR::getEdgeType(RIFG::NodeId src, RIFG::NodeId sink)
{
  RIFG::NodeId anc = LCA(src, sink);
  int dfn_sink = dfnum(sink);
  int dfn_anc = dfnum(anc);

  if (dfn_anc == dfn_sink) {
    return EDGE_ITERATE;
  } 
  else if (TARJ_outer(dfn_sink) != DFNUM_NIL
           && (TARJ_type(TARJ_outer(dfn_sink)) == NODE_IRREDUCIBLE)
           && (dfn_anc != TARJ_outer(dfn_sink))) {
    //
    // Entering irreducible region not through the "header"
    //
    return EDGE_IRRED_ENTRY;
  } 
  else {
    switch (TARJ_type(dfn_sink)) {
    case NODE_INTERVAL:
      return EDGE_LOOP_ENTRY;
    case NODE_IRREDUCIBLE:
      //
      // Entering irreducible region through the "header"
      //
      return EDGE_IRRED_ENTRY;
    case NODE_ACYCLIC:
    case NODE_NOTHING:
    default:
      return EDGE_NORMAL;
    }
  }
}


//
// LCA = least common ancestor
//
RIFG::NodeId 
NestedSCR::LCA(RIFG::NodeId a, RIFG::NodeId b)
{
  if ((a == RIFG::NIL) || (b == RIFG::NIL))
    return RIFG::NIL;

  if (Contains(a,b))
    return a;

  int dfn_b = dfnum(b);
  while ((dfn_b != DFNUM_NIL) && !Contains(b,a)) {
    dfn_b = TARJ_outer(dfn_b);
    b = TARJ_nodeid(dfn_b);
  }

  return b;
}


//
// Return true if the CFG edge passed in is an backedge.
//
//   Precondition: Tarjan tree must be built already.
//
bool 
NestedSCR::isBackEdge(RIFG::EdgeId edgeId)
{
  RIFG::NodeId src, dest;
  src = rifg->getEdgeSrc(edgeId);
  dest = rifg->getEdgeSink(edgeId);
  return is_backedge(dfnum(src), dfnum(dest));
} 


RIFG::NodeId 
NestedSCR::getInners(RIFG::NodeId id)
{
  int dfn_inners = TARJ_inners(dfnum(id));
  return dfn_inners == DFNUM_NIL ? RIFG::NIL : TARJ_nodeid(dfn_inners);
}


RIFG::NodeId 
NestedSCR::getInnersLast(RIFG::NodeId id)
{
  int dfn_last_id = TARJ_last_id(dfnum(id));
  return TARJ_nodeid(dfn_last_id);
}


RIFG::NodeId
NestedSCR::getOuter(RIFG::NodeId id)
{
  int dfn_outer = TARJ_outer(dfnum(id));
  return dfn_outer == DFNUM_NIL ? RIFG::NIL : TARJ_nodeid(dfn_outer);
}


RIFG::NodeId 
NestedSCR::getNext(RIFG::NodeId id)
{
  int dfn_next = TARJ_next(dfnum(id));
  return dfn_next == DFNUM_NIL ? RIFG::NIL : TARJ_nodeid(dfn_next);
}



bool 
NestedSCR::isHeader(RIFG::NodeId id)
{
  return (getInners(dfnum(id)) != DFNUM_NIL);
}


bool 
NestedSCR::isFirst(RIFG::NodeId id)
{
  int dfn_id = dfnum(id);

  if (getOuter(dfn_id) == DFNUM_NIL)
    return true;

  return (getInners(getOuter(dfn_id)) == dfn_id);
}


bool 
NestedSCR::isLast(RIFG::NodeId id)
{
  return (getNext(dfnum(id)) == DFNUM_NIL);
}


int 
NestedSCR::getLevel(RIFG::NodeId id)
{
  return TARJ_level(dfnum(id));
}


NestedSCR::Node_t 
NestedSCR::getNodeType(RIFG::NodeId id)
{
  return TARJ_type(dfnum(id));
}


bool 
NestedSCR::Contains(RIFG::NodeId a, RIFG::NodeId b)
{
  return (TARJ_contains(dfnum(a), dfnum(b)));
}


int 
NestedSCR::getLoopIndex(RIFG::NodeId id)
{
  return TARJ_loopIndex(dfnum(id));
}


} // end of namespace OA


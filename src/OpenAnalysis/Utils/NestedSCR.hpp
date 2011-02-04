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
           Port to OpenAnalysis by Nathan Tallent (and renamed NestedSCR
	     from TarjanIntervals)
  \version $Id: NestedSCR.hpp,v 1.2 2005/03/14 23:49:26 ntallent Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

/* 
   NOTES: 
     - A clean re-write in C++ would greatly enhance the style and
       clarity of the code.
     - Nathan Tallent: Added the DGraph-node -> SCRId map to get
       around the unmet assumption that DGraph-node-ids go from 0-n.
*/

#ifndef NestedStronglyConnectedRegions_H
#define NestedStronglyConnectedRegions_H

#include <iostream>

#include <OpenAnalysis/Utils/RIFG.hpp>
#include <OpenAnalysis/Utils/UnionFindUniverse.hpp>

//***************************************************************************

namespace OA {

//***************************************************************************
// Class: NestedSCR
//
// Description:
//    A Tree of strongly connected regions
//
//***************************************************************************

class TarjWork;
class TarjTreeNode;

  
class NestedSCR {
public:
  
  enum Node_t { NODE_NOTHING, 
		NODE_ACYCLIC, 
		NODE_INTERVAL, 
		NODE_IRREDUCIBLE };

  enum Edge_t { EDGE_NORMAL, 
		EDGE_LOOP_ENTRY, 
		EDGE_IRRED_ENTRY, 
		EDGE_ITERATE };

  typedef unsigned int DFNUM_t;

public:
  // Construct the Tarjan interval tree for the graph.
  NestedSCR(OA::OA_ptr<OA::RIFG> rifg);
  ~NestedSCR();
  
  OA::OA_ptr<OA::RIFG> getRIFG() { return rifg; }


  // Obtain a pointer to tree. [FIXME: Do we want to allow this?]
  TarjTreeNode* getTree();


  bool isFirst(RIFG::NodeId id);
  bool isLast(RIFG::NodeId id);
  bool isHeader(RIFG::NodeId id);

  
  // Given an interval header, return its first nested interval.
  RIFG::NodeId getInners(RIFG::NodeId id);

  // Given an interval header, return its last nested interval.
  RIFG::NodeId getInnersLast(RIFG::NodeId id);

  // Given an interval, return its parent (header) interval.
  RIFG::NodeId getOuter(RIFG::NodeId id);

  // Given an interval, return its next sibling interval. 
  RIFG::NodeId getNext(RIFG::NodeId id);


  // Given an interval, return its nesting level.
  int getLevel(RIFG::NodeId id);

  // Given an interval, return its unique index.
  int getLoopIndex(RIFG::NodeId id);

  // Given a node, return its type (acyclic, interval, irreducible).
  Node_t getNodeType(RIFG::NodeId id);
  Edge_t getEdgeType(RIFG::NodeId src, RIFG::NodeId sink);

  // Return true if the edge is a backedge, false otherwise.
  bool isBackEdge(RIFG::EdgeId e);

  // Return number of loops exited in traversing g edge from src to sink.
  int getExits(RIFG::NodeId src, RIFG::NodeId sink);

  // Return outermost loop exited in traversing g edge from src to sink
  // (RIFG::NIL if no loops exited).
  RIFG::NodeId getLoopExited(RIFG::NodeId src, RIFG::NodeId sink);
  
  
  bool Contains(RIFG::NodeId a, RIFG::NodeId b);

  // Return the least-common-ancestor of two nodes in the tree.
  RIFG::NodeId LCA(RIFG::NodeId a, RIFG::NodeId b);

  
  // Updates the prenumbering of the tree.  Useful if some change
  // has been made to the cfg where one knows how to update the
  // interval tree (as when adding pre-header nodes).
  void Renumber();
  void Prenumber(int n);


  // Pretty-print the interval tree. [N.B.: ignores 'os' at the moment]
  void dump(std::ostream& os);
  
private: // methods
  void Create();

  void Init();
  void InitArrays();
  void DFS(RIFG::NodeId n);
  void FillPredLists();
  void GetTarjans();
  void Build();
  void Sort();
  void ComputeIntervalIndex();
  void ComputeIntervalIndexSubTree(int node, int value);
  void FreeWork();

  void DumpSubTree(std::ostream& os, int node, int indent);

  int FIND(int v);
  void UNION(int i, int j, int k);

private: // data
  // The graph to analyze.
  OA::OA_ptr<OA::RIFG> rifg;

  // Work data
  UnionFindUniverse *uf;
  TarjWork *wk;       // maps RIFG::NodeId to TarjWork
  TarjTreeNode *tarj; // maps RIFG::NodeId to TarjTreeNode
  
  // List of nodes in reverse topological order.
  std::list<RIFG::NodeId> rev_top_list;

  // A map from RIFG::NodeId to DFS number.
  std::map<RIFG::NodeId, DFNUM_t> nodeid_to_dfnum_map;
};


  
} // end of namespace OA

#endif

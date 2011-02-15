/*! \file
  
  \brief Declaration for DomTree

  \authors Arun Chauhan (2001 as part of Mint)
           Port from 'Classic OpenAnalysis' to OpenAnalysis by Nathan Tallent
  \version $Id: DomTree.hpp,v 1.4 2005/04/14 20:04:22 eraxxon Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

//***************************************************************************

//---------------------------------------------------------------------------

#ifndef OA_DomTree_H
#define OA_DomTree_H

//---------------------------------------------------------------------------

#include <iostream>

// STL headers
#include <set>
#include <map>

// OpenAnalysis headers
#include <OpenAnalysis/Utils/Tree.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphInterface.hpp>

//---------------------------------------------------------------------------

namespace OA {

//---------------------------------------------------------------------------
class DomTree : public Tree 
{
public:
  class DomFrontIterator;

  //-------------------------------------------------------------------------
  class Node : public Tree::Node {
  public:
    Node(OA_ptr<DGraph::NodeInterface> n) : Tree::Node() 
      { graph_node_ptr = n; }
    ~Node() {}
    OA_ptr<DGraph::NodeInterface> getGraphNode() { return graph_node_ptr; }

    OA_ptr<DomFrontIterator> getDomFrontIterator() { 
      OA_ptr<DomFrontIterator> it; it = new DomFrontIterator(*this);
      return it;
    }
    
    // included here to return subclass node type, avoids convert
    // calls in code that uses this
    OA_ptr<Node>  parent () const { 
      OA_ptr<Tree::Node> n = Tree::Node::parent();
      if (n.ptrEqual(0)) {
	OA_ptr<Node> n; // convert() dies on NULL pointers!
	return n;
      }
      else {
	return n.convert<Node>();
      }
    }

    void dump(std::ostream& os);

  private:
    typedef std::set<OA_ptr<Node> > DomFrontSet;

    OA_ptr<DGraph::NodeInterface> graph_node_ptr;
    DomFrontSet dom_front;
    friend class DomTree;
    friend class DomTree::DomFrontIterator;
  };
  
  //-------------------------------------------------------------------------
  class DomFrontIterator : public Iterator {
  public:
    DomFrontIterator(Node& n) 
      : Iterator(), mDomFrontSet(n.dom_front), mIter(mDomFrontSet.begin()) { }
    ~DomFrontIterator() { }
    
    OA_ptr<Node> current() const { return *mIter; }
    void operator++() { ++mIter; }
    bool isValid() const { return (mIter != mDomFrontSet.end()); }
    void reset() { mIter = mDomFrontSet.begin(); }

  private:
    std::set<OA_ptr<Node> >& mDomFrontSet;
    std::set<OA_ptr<Node> >::iterator mIter;
  };
  //-------------------------------------------------------------------------

public:
  DomTree(OA_ptr<DGraph::DGraphInterface> graph,
	  OA_ptr<DGraph::NodeInterface> root);
  ~DomTree() {}

  OA_ptr<Node> domtree_node(OA_ptr<DGraph::NodeInterface> n) 
    { return dom_tree_node[n]; }
  void compute_dominance_frontiers();
  void dump(std::ostream& os);

private:
  OA_ptr<DGraph::DGraphInterface> graph;
  std::map<OA_ptr<DGraph::NodeInterface>, OA_ptr<Node> > dom_tree_node;
};
//---------------------------------------------------------------------------

} // end of namespace OA

#endif

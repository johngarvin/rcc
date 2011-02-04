/*! \file
  
  \brief Declaration of Tree class

  \authors Arun Chauhan (2001 was part of Mint)
  \version $Id: Tree.cpp,v 1.10 2005/03/08 18:55:56 ntallent Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  Tree is the base class of a general tree where each node can have a
  variable number of child nodes.  All algorithms that operate on
  abstract trees should use this base class for maximum portability.
*/


// Mint headers
#include "Tree.hpp"

namespace OA {

template <class T> void deque_erase (std::deque<T> d, T elt);

//--------------------------------------------------------------------
/*! OutEdgesIterator iterates over all the non-null outgoing edges of
    a node.  The constructor initialized the
    iterator to point to the first non-null edge. 
*/
Tree::OutEdgesIterator::OutEdgesIterator (const Node&  n)
{
  mQueue = n.outgoing;
  mIter = mQueue->begin();
  OA_ptr<Edge>  e; e = 0;
  if (mIter != mQueue->end()) {
    e = *mIter;
  }
  while ((e.ptrEqual(0)) && (mIter != mQueue->end())) {
    ++mIter;
    e = *mIter;
  }
}
//--------------------------------------------------------------------

//--------------------------------------------------------------------
/*! The ++ operator for OutEdgesIterator skips all null edges. */
void
Tree::OutEdgesIterator::operator ++ ()
{
  ++mIter;
  OA_ptr<Edge>  e; e = 0;
  if (mIter != mQueue->end())
    e = *mIter;
  while ((e.ptrEqual(0)) && (mIter != mQueue->end())) {
    ++mIter;
    e = *mIter;
  }
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
/*! PreOrderIterator creates an iterator to enumerate the tree nodes
    in pre-order.  If this is the first time a pre-order traversal has
    been demanded for this tree, or the tree has changed since the last
    pre-order traversal, a pre-order walk is carried out using the
    recursive method create_preorder_links.  This links up the nodes in
    pre-order and subsequent pre-order traversals simply follow the links.
*/
Tree::PreOrderIterator::PreOrderIterator (Tree& t)
{
  if (! t.root_node.ptrEqual(0)) {
    if (t.preorder_needed) {
      // reset all the preoder_next links
      std::set<OA_ptr<Node> >::iterator ni = t.node_set->begin();
      while (ni != t.node_set->end()) {
        OA_ptr<Node>  n = *ni;
        n->next_preorder = 0;
        ++ni;
      }
      t.create_preorder_links(t.root_node);
      t.preorder_needed = false;
    }
  }
  p = t.root_node;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
/*! This method must be called before a pre-order traversal can be
    carried out in a new, or altered, graph.  The
    method recursively traverses the graph in pre-order and updates
    the nodes' next_preorder links. 
*/
OA_ptr<Tree::Node> 
Tree::create_preorder_links (OA_ptr<Tree::Node>  n)
{
  OA_ptr<Node>  last;

  ChildNodesIterator iter(*n);
  if (!iter.isValid()) {
    n->next_preorder = 0;
    last = n;
  }
  else {
    last = n;
    do {
      last->next_preorder = iter.current();
      last = create_preorder_links(iter.current());
      ++iter;
    } while (iter.isValid());
  }
  return last;
}
//--------------------------------------------------------------------

//--------------------------------------------------------------------
/*! PostOrderIterator creates an iterator to enumerate the tree nodes
    in post-order.  If this is the first time a post-order traversal has
    been demanded for this tree, or the tree has changed since the last
    post-order traversal, a post-order walk is carried out using the
    recursive method create_postorder_links.  This links up the nodes in
    post-order and subsequent post-order traversals simply follow the links.
*/
Tree::PostOrderIterator::PostOrderIterator (Tree& t)
{
  if (! t.root_node.ptrEqual(0)) {
    if (t.postorder_needed) {
      // reset all the preoder_next links
      std::set<OA_ptr<Node> >::iterator ni = t.node_set->begin();
      while (ni != t.node_set->end()) {
	OA_ptr<Node>  n = *ni;
	n->next_postorder = NULL;
	n->prev_postorder = NULL;
	++ni;
      }
      OA_ptr<Node> nullNode;  nullNode = NULL;
      t.create_postorder_links(t.root_node, nullNode);
      t.postorder_needed = false;
      t.rpostorder_needed = false;
    }
  }
  p = t.mPostOrderStart;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
/*! This method must be called before a post-order traversal can be
    carried out in a new, or altered, graph.  The
    method recursively traverses the graph in post-order and appends
    nodes to a list.
*/
OA_ptr<Tree::Node> 
Tree::create_postorder_links (OA_ptr<Tree::Node>  n, OA_ptr<Tree::Node> last)
{
  OA_ptr<Node> retLast = last;

  ChildNodesIterator iter(*n);
  for (; iter.isValid(); ++iter) {
    retLast = create_postorder_links(iter.current(),retLast);
  }
 
  // links for this node
  if (retLast.ptrEqual(NULL)) { // this is the first node in postorder
    mPostOrderStart = n;
  } else {
    retLast->next_postorder = n;
  }
  n->prev_postorder = retLast;
  retLast = n;

  return retLast;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
/*! An edge to be added to the tree must not belong to any tree 
    (including this one). */
void
Tree::addEdge(OA_ptr<Tree::Edge>  e)
  throw (Tree::DuplicateEdge, Tree::EdgeInUse, Tree::EmptyEdge, Tree::SecondParent, Tree::EmptyEndPoint)
{
  if (e.ptrEqual(0)) {
    throw EmptyEdge();
  }
  if (edge_set->find(e) != edge_set->end()) {
    throw DuplicateEdge(e);
  }
  if (e->in_use) {
    throw EdgeInUse(e);
  }
  if ((e->child_node.ptrEqual(0)) || (e->parent_node.ptrEqual(0))) {
    throw EmptyEndPoint(e);
  }
  if (!e->child_node->incoming.ptrEqual(0)) {
      throw SecondParent(e);
  }

  // insert the nodes if they don't already exist in the graph
  if (node_set->find(e->parent_node) == node_set->end()) {
    addNode(e->parent_node);
  }
  if (node_set->find(e->child_node) == node_set->end()) {
    addNode(e->child_node);
  }

  // insert the edge in the corresponding lists of the two nodes
  e->parent_node->outgoing->push_back(e);
  e->child_node->incoming = e;

  // finally, insert the edge itself in the tree
  e->in_use = true;
  edge_set->insert(e);
  preorder_needed = postorder_needed = rpostorder_needed = true;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
/*! A node to be added to the tree must not belong to any tree (including this one). */
void
Tree::addNode(OA_ptr<Tree::Node>  n)
  throw (Tree::DuplicateNode, Tree::NodeInUse, Tree::EmptyNode)
{
  if (n.ptrEqual(0)) {
    throw EmptyNode();
  }
  if (node_set->find(n) != node_set->end()) {
    throw DuplicateNode(n);
  }
  if (n->in_use) {
    throw NodeInUse(n);
  }
  if (root_node.ptrEqual(0)) {
    root_node = n;
  }
  n->in_use = true;
  node_set->insert(n);
  preorder_needed = postorder_needed = rpostorder_needed = true;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
/*! It is often useful to be able to have an empty outgoing edge to
    indicate the absence of a sub-tree.  This method
    adds an empty edge to the given node. */
void
Tree::add_empty_edge (OA_ptr<Tree::Node> n)
  throw (Tree::EmptyNode)
{
  if (n.ptrEqual(0)) {
    throw EmptyNode();
  }
  OA_ptr<Edge> nullEdge; nullEdge = 0;
  n->outgoing->push_back(nullEdge);
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
void
Tree::removeEdge (OA_ptr<Tree::Edge>  e)
  throw (Tree::NonexistentEdge, Tree::EmptyEdge)
{
  if (e.ptrEqual(0)) {
    throw EmptyEdge();
  }
  if (edge_set->erase(e) == 0) {
    throw NonexistentEdge(e);
  }
  preorder_needed = postorder_needed = rpostorder_needed = true;
  e->in_use = false;
  deque_erase<OA_ptr<Edge> >(*(e->parent_node->outgoing), e);
  e->child_node->incoming = 0;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
void
Tree::removeNode (OA_ptr<Tree::Node>  n)
  throw (Tree::NonexistentNode, Tree::DeletingRootOfNonSingletonTree, Tree::EmptyNode)
{
  if (n.ptrEqual(0)) {
    throw EmptyNode();
  }
  if ((n.ptrEqual(root_node)) && (!edge_set->empty() || (node_set->size() > 1)))
    throw DeletingRootOfNonSingletonTree(n);
  if (node_set->erase(n) == 0)
    throw NonexistentNode(n);
  preorder_needed = postorder_needed = rpostorder_needed = true;
  n->in_use = false;
  if (n == root_node)
    root_node = 0;

  if (! n->incoming.ptrEqual(0)) {
    edge_set->erase(n->incoming);
    deque_erase<OA_ptr<Edge> >(*(n->incoming->parent_node->outgoing), 
                               n->incoming);
  }

  OutEdgesIterator out(*n);
  while (out.isValid()) {
    OA_ptr<Edge>  e = out.current();
    e->child_node->incoming = 0;
    edge_set->erase(e);
    ++out;
  }
}
//--------------------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------
/*! Since std::deque does not support random deletion of elements, we 
    must search for an element and then erase it. */
template <class T>
void
deque_erase (std::deque<T> d, T elt)
{
  typename std::deque<T>::iterator iter = d.begin();
  while (iter != d.end()) {
    T k = *iter;
    if (k == elt) {
      d.erase(iter);
      break;
    }
  }
}
//--------------------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------------------
/** Clone (as the name suggests) clones an entire subtree.  The parent of the new subtree is left undefined, but, all
    of it is added to the Tree.  It returns a pointer to the root of the newly created subtree.  Cloning proceeds by
    first creating a new copy of the root of the subtree -- this uses the new_copy virtual function for Node.  Next,
    the children of this root node are recursively cloned and clones of the existing edges are added to link the root
    with all the newly created children -- this time, using the new_copy virtual function for Edge.  */
/*
OA_ptr<Tree::Node>
Tree::clone (OA_ptr<Tree::Node>  subtree)
{
  OA_ptr<Node>  new_root = subtree->new_copy();
  OutEdgesIterator edge_iter(*subtree);
  while (edge_iter.isValid()) {
    OA_ptr<Edge>  out_edge = edge_iter.current();
    if (! out_edge.ptrEqual(0)) {
      OA_ptr<Node>  new_child = clone(out_edge->sink());
      addEdge(out_edge->new_copy(new_root, new_child));
    }
    else {
      OA_ptr<Edge> nullEdge; nullEdge = 0;
      new_root->outgoing->push_back(nullEdge);
    }
    ++edge_iter;
  }
  return new_root;
}
*/
//--------------------------------------------------------------------------------------------------------------------

} // end of OA namespace

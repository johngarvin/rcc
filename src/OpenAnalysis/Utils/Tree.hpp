/*! \file
  
  \brief Declaration for Tree class

  \authors Arun Chauhan (2001 was part of Mint)
  \version $Id: Tree.hpp,v 1.15 2005/06/10 02:32:05 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef Tree_H
#define Tree_H

// STL headers
#include <deque>
#include <set>

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

// Mint headers
#include "Iterator.hpp"
#include "Exception.hpp"

#include <OpenAnalysis/Utils/OutputBuilder.hpp>
namespace OA {

//--------------------------------------------------------------------
/*! 
   Tree is the base class of a general tree where each node can have a
   variable number of child nodes.  All algorithms that operate on
   abstract trees should use this base class for maximum portability.

   Nodes and edges must be unique, meaning, a node or object cannot be
   added to the tree more than once.  The root node cannot be deleted
   if there are other nodes or edges remaining in the tree.  Finally,
   no node can have more than one incoming (parent) edge.  Edges and
   nodes are identified by their pointer values (OA_ptr<Tree::Edge>  and
   OA_ptr<Tree::Node> ).  An node is allowed to point to an empty edge since
   this comes handy in many situations where the order of outgoing
   edges matters.  Such an edge must be inserted using add_empty_edge
   method.

   Following exceptions are thrown by the class (all are subclasses of
   Exception):

     -#  Tree::EmptyEdge                      -- attempt to add, or remove, an empty edge (null pointer)
     -#  Tree::DuplicateEdge                  -- attempt to add an edge more than once
     -#  Tree::NonexistentEdge                -- attempt to remove an edge that does not belong to the tree
     -#  Tree::EdgeInUse                      -- attempt to add an edge that is already a part of another tree
     -#  Tree::SecondParent                   -- attempt to add a second incoming edge to a node
     -#  Tree::EmptyNode                      -- attempt to add, or remove, an empty node (null pointer)
     -#  Tree::DuplicateNode                  -- attempt to add a node more than once
     -#  Tree::NonexistentNode                -- attempt to remove a node that does not belong to the tree
     -#  Tree::NodeInUse                      -- attempt to add a node that is already a part of another tree
     -# Tree::DeletingRootOfNonSingletonTree -- attempt to delete the root node when tree has more nodes & edges

   NOTE ON friend CLASSES: Many classes (especially Tree::Node  and
   Tree::Edge) have many friend classes.  This is *not* a kludge.  It
   is simulating "package" visiblity in Java.  We want a limited
   public interface to OA_ptr<Node>  and Edge and yet give more permissions to
   methods within the Graph class.  
*/
//--------------------------------------------------------------------
class Tree : public virtual Annotation {
 public:
  class Node ;
  class Edge;
  class NodesIterator;
  class EdgesIterator;
  class PreOrderIterator;
  class PostOrderIterator;
  class ReversePostOrderIterator;
  class OutEdgesIterator;
  class ChildNodesIterator;
  friend class NodesIterator;
  friend class EdgesIterator;
  friend class PreOrderIterator;
  friend class PostOrderIterator;
  friend class ReversePostOrderIterator;
  //------------------------------------------------------------------
  /*! EmptyEdge exception is thrown if an edge being added is null (0) */
  class EmptyEdge : public Exception {
  public:
    EmptyEdge () {}
    ~EmptyEdge () {}
    void report (std::ostream& o) const { o << "E!  Adding a null edge to a tree." << std::endl; }
  };
  //------------------------------------------------------------------
  /*! DuplicateEdge exception is thrown if an edge being added is 
      already a part of the tree. 
  */
  class DuplicateEdge : public Exception {
  public:
    DuplicateEdge (OA_ptr<Tree::Edge>  e) { offending_edge = e; }
    ~DuplicateEdge () {}
    void report (std::ostream& o) const { o << "E!  Adding a duplicate edge to a tree." << std::endl; }
  private:
    OA_ptr<Tree::Edge> offending_edge;
  };
  //------------------------------------------------------------------
  /*! NonexistentEdge exception is thrown if an edge being deleted is 
      not a part of the tree. */
  class NonexistentEdge : public Exception {
  public:
    NonexistentEdge (OA_ptr<Tree::Edge>  e) { offending_edge = e; }
    ~NonexistentEdge () {}
    void report (std::ostream& o) const { o << "E!  Removing a non-existent edge from a tree." << std::endl; }
  private:
    OA_ptr<Tree::Edge>  offending_edge;
  };
  //------------------------------------------------------------------
  /*! EdgeInUse exception is thrown if an edge being added is already 
      a part of another tree. 
  */
  class EdgeInUse : public Exception {
  public:
    EdgeInUse (OA_ptr<Tree::Edge>  e) { offending_edge = e; }
    ~EdgeInUse () {}
    void report (std::ostream& o) const { o << "E!  Adding an edge that is already a part of another tree." << std::endl; }
  private:
    OA_ptr<Tree::Edge>  offending_edge;
  };
  //------------------------------------------------------------------
  /*! SecondParent exception is thrown if the sink (child) of the edge 
      being added is already present in the tree,
      meaning that it already has a parent. 
  */
  class SecondParent : public Exception {
  public:
    SecondParent (OA_ptr<Tree::Edge>  e) { offending_edge = e; }
    ~SecondParent () {}
    void report (std::ostream& o) const { o << "E!  Adding an edge that points to an existing node in the tree." << std::endl; }
  private:
    OA_ptr<Tree::Edge>  offending_edge;
  };
  //------------------------------------------------------------------
  /*! EmptyEndPoint exception is thrown if the one, or both, of the 
      end points of an edge being added is null. 
  */
  class EmptyEndPoint : public Exception {
  public:
    EmptyEndPoint (OA_ptr<Tree::Edge>  e) { offending_edge = e; }
    ~EmptyEndPoint () {}
    void report (std::ostream& o) const { o << "E!  Adding an edge that points to an empty node." << std::endl; }
  private:
    OA_ptr<Tree::Edge>  offending_edge;
  };
  //------------------------------------------------------------------
  /*! EmptyNode exception is thrown if a node being added is null (0) */
  class EmptyNode : public Exception {
  public:
    EmptyNode () {}
    ~EmptyNode () {}
    void report (std::ostream& o) const { o << "E!  Adding a null node to a tree." << std::endl; }
  };
  //------------------------------------------------------------------
  /*! DuplicateNode exception is thrown if a node being added is 
      already a part of the tree. 
  */
  class DuplicateNode : public Exception {
  public:
    DuplicateNode (OA_ptr<Tree::Node>  n) { offending_node = n; }
    ~DuplicateNode () {}
    void report (std::ostream& o) const { o << "E!  Adding a duplicate node to a tree." << std::endl; }
  private:
    OA_ptr<Tree::Node>  offending_node;
  };
  //------------------------------------------------------------------
  /*! NonexistentNode exception is thrown if a node being deleted is 
      not a part of the tree. 
  */
  class NonexistentNode : public Exception {
  public:
    NonexistentNode (OA_ptr<Tree::Node>  n) { offending_node = n; }
    ~NonexistentNode () {}
    void report (std::ostream& o) const { o << "E!  Removing a non-existent node from the tree." << std::endl; }
  private:
    OA_ptr<Tree::Node>  offending_node;
  };
  //------------------------------------------------------------------
  /*! NodeInUse exception is thrown if a node being added is already 
      a part of another tree. 
  */
  class NodeInUse : public Exception {
  public:
    NodeInUse (OA_ptr<Tree::Node>  n) { offending_node = n; }
    ~NodeInUse () {}
    void report (std::ostream& o) const { o << "E!  Adding a node that is already a part of another tree." << std::endl; }
  private:
    OA_ptr<Tree::Node>  offending_node;
  };
  //------------------------------------------------------------------
  /*! DeletingRootOfNonSingletonTree is thrown when an attempt is made 
      to remove the root of a tree that has other
      nodes and edges. 
  */
  class DeletingRootOfNonSingletonTree : public Exception {
  public:
    DeletingRootOfNonSingletonTree (OA_ptr<Tree::Node>  n) { offending_node = n; }
    ~DeletingRootOfNonSingletonTree () {}
    void report (std::ostream& o) const { o << "E!  Deleting the root node of a non-singleton tree." << std::endl; }
  private:
    OA_ptr<Tree::Node>  offending_node;
  };
  //------------------------------------------------------------------
  /*! A tree node has a pointer for the incoming (parent) node, a list 
      of outgoing (children) nodes and links for
      different traversal orders. 
  */
  class Node : public virtual Annotation {
  public:
    Node() 
        { in_use = false; 
          next_preorder = next_postorder = prev_postorder = 0;  
          incoming = 0; 
          outgoing = new std::deque<OA_ptr<Edge> >;
        }
    //virtual OA_ptr<Node> new_copy () 
    //    { OA_ptr<Node> n; n = new Node(); copy(n); return n; }
    virtual ~Node () {}


    OA_ptr<Node> parent () const 
        { OA_ptr<Node> retval; retval=0;
          if (!incoming.ptrEqual(0)) {
            retval = incoming->source(); 
          } 
          return retval;
        }
    OA_ptr<Node> child (int i) const 
        { OA_ptr<Node> retval; retval = 0;
          if (! (*outgoing)[i].ptrEqual(0)) {
            retval = (*outgoing)[i]->child(); 
          } 
          return retval;
        }

    OA_ptr<Edge>  in_edge () const { return incoming; }
    OA_ptr<Edge>  out_edge (int i) const { return (*outgoing)[i]; }
    int num_children () const { return outgoing->size(); }
    virtual bool operator==(Node& other) { return &other==this; }
    virtual bool operator<(Node& other) { return this<&other; }
    virtual void dump (std::ostream& os) { os << this; }
    
    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    virtual void output(IRHandlesIRInterface& ir) const {
      std::ostringstream label;
      label << this; // what does this print, anyway?
      sOutBuild->outputString( label.str() );
    }

    // Iterators
    OA_ptr<OutEdgesIterator> 
    getOutEdgesIterator() const {
      OA_ptr<OutEdgesIterator> it; it = new OutEdgesIterator(*this);
      return it;
    }
    
    OA_ptr<ChildNodesIterator> 
    getChildNodesIterator() const {
      OA_ptr<ChildNodesIterator> it; it = new ChildNodesIterator(*this);
      return it;
    }

  protected:
    //virtual void copy (OA_ptr<Node>  n) {}
    OA_ptr<Edge>  incoming;
    OA_ptr<std::deque<OA_ptr<Edge> > > outgoing;
    OA_ptr<Node>  next_preorder;
    OA_ptr<Node>  next_postorder;
    OA_ptr<Node>  prev_postorder;
    bool in_use;
    friend class Tree;
    friend class Tree::PreOrderIterator;
    friend class Tree::PostOrderIterator;
    friend class Tree::ReversePostOrderIterator;
    friend class Tree::OutEdgesIterator;
    friend class Tree::ChildNodesIterator;
  };
  //------------------------------------------------------------------
  /*! A tree edge has pointers to the nodes it links (parent and child). */
  class Edge {
  public:
    Edge (OA_ptr<Node>  _source, OA_ptr<Node>  _sink) 
        { in_use = false; parent_node = _source; child_node = _sink; }
    //virtual OA_ptr<Edge> new_copy (OA_ptr<Node>  _src, OA_ptr<Node>  _snk) 
    //    { OA_ptr<Edge>  e; e = new Edge(_src, _snk); copy(e); return e; }
    virtual ~Edge () {}
    virtual bool operator==(Edge& other) 
        {
            if (other.parent_node == parent_node 
                && other.child_node == child_node )
            { return true; }
            else { return false; }
        }
    virtual bool operator<(Edge& other) 
        {
            if (*this==other) { return false; }
            if (parent_node < other.parent_node) { return true; }
            if (child_node < other.child_node) { return true; }
            return false; 
        }
    OA_ptr<Node>  parent () const { return parent_node; }
    OA_ptr<Node>  source () const { return parent(); }
    OA_ptr<Node>  tail () const { return parent(); }
    OA_ptr<Node>  child () const { return child_node; }
    OA_ptr<Node>  sink () const { return child(); }
    OA_ptr<Node>  head () const { return child(); }
    virtual void dump (std::ostream& os) { os << this; }
  protected:
    //void copy (OA_ptr<Edge>  e) {}
    OA_ptr<Node>  parent_node;
    OA_ptr<Node>  child_node;
    bool in_use;
    friend class Tree;
    friend class Tree::Node;
  };
  //------------------------------------------------------------------
  /*! The node iterator iterates over all the nodes in the tree in no 
      particular order. 
  */
  class NodesIterator : public Iterator {
  public:
    NodesIterator (const Tree& t) { mSet = t.node_set;  
        mIter = mSet->begin(); }
    virtual ~NodesIterator () {}
    void operator++ () { ++mIter; }
    void operator++(int) { ++*this; }  // postfix
    bool isValid() const { return (mIter != mSet->end()); }
    OA_ptr<Node>  current() const { return *mIter; }

  protected:
    OA_ptr<std::set<OA_ptr<Node> > > mSet;
    std::set<OA_ptr<Node> >::iterator mIter;
  };
  //------------------------------------------------------------------
  /*! The edge iterator iterates over all the edges in the tree in no 
      particular order. 
  */
  class EdgesIterator : public Iterator {
  public:
    EdgesIterator (const Tree& t) { mSet = t.edge_set; 
        mIter = mSet->begin(); }
    virtual ~EdgesIterator () {}
    void operator++ () { ++mIter; }
    void operator++(int) { ++*this; }  // postfix
    bool isValid() const { return (mIter != mSet->end()); }
    OA_ptr<Edge>  current() const { return *mIter; }

  protected:
    OA_ptr<std::set<OA_ptr<Edge> > > mSet;
    std::set<OA_ptr<Edge> >::iterator mIter;
  };
  //------------------------------------------------------------------
  /*! OutEdgesIterator iterates over all the non-null outgoing nodes 
      of a node. 
  */
  class OutEdgesIterator : public Iterator {
  public:
    OutEdgesIterator (const Node& n);
    virtual ~OutEdgesIterator () {}
    void operator++ ();
    void operator++(int) { ++*this; }  // postfix
    bool isValid() const { return (mIter != mQueue->end()); }
    OA_ptr<Edge>  current() const { return *mIter; }

  private:
    OA_ptr<std::deque<OA_ptr<Edge> > > mQueue;
    std::deque<OA_ptr<Edge> >::iterator mIter;
  };
  //------------------------------------------------------------------
  /*! ChildNodesIterator iterates over all the child nodes of a node.  
   *  Thus, it skips the null edges. 
   */
  class ChildNodesIterator : private OutEdgesIterator {
  public:
    ChildNodesIterator(const Node& n) : OutEdgesIterator(n) {}
    ~ChildNodesIterator() {}
    void operator++() { OutEdgesIterator::operator++(); }
    void operator++(int) { ++*this; }  // postfix
    bool isValid() const { return OutEdgesIterator::isValid(); }
    OA_ptr<Node>  current() const
    { OA_ptr<Edge>  e = OutEdgesIterator::current();  return e->sink(); }

  };
  //------------------------------------------------------------------
  /*! Pre-order iterator enumerates the nodes in pre-order (a node is 
      visited before all its sub-trees). 
  */
  class PreOrderIterator : public Iterator {
  public:
    PreOrderIterator(Tree& t);
    virtual ~PreOrderIterator() {}
    void operator++() { if (!p.ptrEqual(0)) p = p->next_preorder; }
    void operator++(int) { ++*this; }  // postfix
    bool isValid() const { return (!p.ptrEqual(0)); }
    OA_ptr<Node>  current() const { return p; }

  private:
    OA_ptr<Node>  p; // pointer to the last visited node
  };
  //------------------------------------------------------------------
  /*! Post-order iterator enumerates the nodes in post-order 
      (a node's sub-trees are visited before the node). 

      If Tree is changed during iteration of nodes then the
      list contained within this Iterator may no longer be accurate.
      This iterator could also keep references to Node's no longer
      in the Tree.

      OLD DESIGN notes (decided to use links):
      We don't want the Iterator to have an OA_ptr<> to the Tree
      itself because the Tree might have a getPostOrderIterator
      method and then we would have to pass this to an OA_ptr.
      If we did have an OA_ptr to the Tree then we could keep 
      the current list with the Tree and it could be shared amongst
      various iterators.  We could also do a better job of keeping
      the iterators up-to-date.
      We could also keep iterators better up-to-date if we figured
      out how to do the post-order links
  */
  class PostOrderIterator : public Iterator {
  public:
    PostOrderIterator(Tree& t);
    virtual ~PostOrderIterator() {}
    void operator++() { if (isValid()) p = p->next_postorder; }
    void operator++(int) { ++*this; }  // postfix
    bool isValid() const { return (!p.ptrEqual(0)); }
    OA_ptr<Node>  current() const { return p; }

  private:
    OA_ptr<Node>  p; // pointer to the last visited node
  };
  //------------------------------------------------------------------
  /*! Reverse post-order iterator, as the name suggests, enumerates 
      the nodes in the order that is reverse of post-order. 
  */
  class ReversePostOrderIterator : public Iterator {
  public:
    ReversePostOrderIterator (const Tree& t);
    virtual ~ReversePostOrderIterator () {}
    void operator++ () { if (!p.ptrEqual(0)) p = p->prev_postorder; }
    void operator++(int) { ++*this; }  // postfix
    bool isValid() const { return (!p.ptrEqual(0)); }
    OA_ptr<Node>  current() const { return p; }

  private:
    OA_ptr<Node>  p; // pointer to the last visited node
  };

  //------------------------------------------------------------------
public:
  Tree() 
      { root_node = 0;
        edge_set = new std::set<OA_ptr<Edge> >;
        node_set = new std::set<OA_ptr<Node> >; }
  Tree(OA_ptr<Node>  root) { addNode(root); }
  virtual ~Tree() {}
  
  OA_ptr<Node>  getRoot() const { return root_node; }

  void addEdge(OA_ptr<Edge> ) 
    throw (DuplicateEdge, EdgeInUse, EmptyEdge, SecondParent, EmptyEndPoint);
  void addNode(OA_ptr<Node> ) throw (DuplicateNode, NodeInUse, EmptyNode);
  void add_empty_edge(OA_ptr<Node> ) throw (EmptyNode);
  void removeEdge(OA_ptr<Edge> ) throw (NonexistentEdge, EmptyEdge);
  void removeNode(OA_ptr<Node> ) 
      throw (NonexistentNode, DeletingRootOfNonSingletonTree, EmptyNode);
  //OA_ptr<Node>  clone (OA_ptr<Node> );

  //*****************************************************************
  // Annotation Interface
  //*****************************************************************
  virtual void output(IRHandlesIRInterface& ir) const { }


  //------------------------------------------------------------------
  // Iterators
  //------------------------------------------------------------------
  
  OA_ptr<NodesIterator> getNodesIterator() const { 
    OA_ptr<NodesIterator> it; it = new NodesIterator(*this);
    return it;
  }

  OA_ptr<EdgesIterator> getEdgesIterator() const {
    OA_ptr<EdgesIterator> it; it = new EdgesIterator(*this);
    return it;
  }

  OA_ptr<PreOrderIterator> getPreOrderIterator() {
    OA_ptr<PreOrderIterator> it; it = new PreOrderIterator(*this);
    return it;
  }

  OA_ptr<PostOrderIterator> getPostOrderIterator() {
    OA_ptr<PostOrderIterator> it; it = new PostOrderIterator(*this);
    return it;
  }
  
  OA_ptr<ReversePostOrderIterator> getReversePostOrderIterator() const {
    OA_ptr<ReversePostOrderIterator> it; 
    it = new ReversePostOrderIterator(*this);
    return it;
  }


protected:
  OA_ptr<Node>  root_node;
  OA_ptr<Node>  mPostOrderStart;
  OA_ptr<std::set<OA_ptr<Edge> > > edge_set;
  OA_ptr<std::set<OA_ptr<Node> > > node_set;
  bool preorder_needed;
  bool postorder_needed;
  bool rpostorder_needed;

private:
  OA_ptr<Node> create_preorder_links ( OA_ptr<Node> );
  OA_ptr<Node> create_postorder_links ( OA_ptr<Node>, OA_ptr<Node> );
  //OA_ptr<Node>  create_rpostrder_links ( OA_ptr<Node> );
};
//--------------------------------------------------------------------

} // end of OA namespace

#endif

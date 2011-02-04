/*! \file
  
  \brief Expression tree declarations
  
  \authors Nathan Tallent, Michelle Strout, Andy Stone (ordering ops)
  \version $Id: ExprTree.hpp,v 1.17 2005/03/17 22:35:29 ntallent Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ExprTree_H
#define ExprTree_H
#include <set>
#include <map>

// OpenAnalysis headers
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Utils/Tree.hpp>
#include <OpenAnalysis/IRInterface/ConstValBasicInterface.hpp>
#include <OpenAnalysis/IRInterface/OpBasicInterface.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>

#include <OpenAnalysis/ExprTree/ExprTree.hpp>

namespace OA {
using namespace std;

class ExprTree;

// to avoid circular reference in header files
class ExprTreeVisitor;



//--------------------------------------------------------------------
/*! ExprTree is a Tree with enhanced nodes.  
    It represents expressions involving various IRHandles.
*/
  class ExprTree : public Tree {
public:
  class Node;
  class Edge;

public:
  ExprTree();
  virtual ~ExprTree();
  
  OA_ptr<Node>  getRoot() const { 
    OA_ptr<Tree::Node> n = Tree::getRoot();
    return n.convert<Node>(); 
  }


  //! visit root and then accept visitor on tree
  void acceptVisitor(ExprTreeVisitor& pVisitor) const;

  //-------------------------------------
  // construction
  //-------------------------------------
  void connect(OA_ptr<Node> src, OA_ptr<Node> dst) 
      { OA_ptr<Edge> e; e = new Edge(src,dst); addEdge(e); }
  void disconnect(OA_ptr<Edge> e) { removeEdge(e.convert<Tree::Edge>()); }
  void copyAndConnectSubTree(OA_ptr<Node> src, OA_ptr<ExprTree> subtree);
  

  //*****************************************************************
  // Ordering Operations
  //*****************************************************************
  // an ordering for expression trees, needed for use within STL containers
  bool operator<(ExprTree &rhs);
  bool operator==(ExprTree &rhs);
private:
  bool compareTreesRootedAt(OA_ptr<Node> rootLHS, OA_ptr<Node> rootRHS);
public:

  //--------------------------------------------------------
  class Node : public Tree::Node {
  public:
    Node()
      : Tree::Node() { }
    Node(const std::string& x)
      : Tree::Node(), mName(x) { }
    virtual ~Node() { }
    
    // A name for this node (useful for debugging)
    std::string& getName() { return mName; }

    //! return a copy of self
    virtual OA_ptr<Node> copy() = 0;

    //! isa methods, default for all is false
    virtual bool isaOpNode() { return false; }
    virtual bool isaCallNode() { return false; }
    virtual bool isaMemRefNode() { return false; }
    virtual bool isaConstSymNode() { return false; }
    virtual bool isaConstValNode() { return false; }

    //! pass visitor to nodes in postorder and then accept visitor on tree
    // NOTE: could give this a default implementation if wanted to
    // have ExprTree visitors have default implementation for nodes
    // they don't know about
    virtual void acceptVisitor(ExprTreeVisitor&) = 0;
    virtual void dump(std::ostream& os) { Tree::Node::dump(os); }
    virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    virtual void output(IRHandlesIRInterface& ir) const {
        Tree::Node::output(ir);
    }
   
    //*****************************************************************
    // Ordering Operations
    //*****************************************************************
    virtual bool operator<(Node& rhs) = 0;
    virtual int getOrder() const = 0;

  private:
    std::string mName;
    //std::string mAttr;
  }; // end of Node

  class OpNode : public Node {
  public:
    OpNode() : Node("OpNode") { }
    OpNode(OA_ptr<OpBasicInterface> op) : Node("OpNode"), mOpInterface(op) {}
    virtual ~OpNode() { }

    //! return a copy of self
    OA_ptr<Node> copy() 
      { OA_ptr<Node> ret; ret = new OpNode(mOpInterface); return ret; }

    bool isaOpNode() { return true; }
    OA_ptr<OpBasicInterface> getOpInterface() const { return mOpInterface; }
    void dump(std::ostream& os) { Node::dump(os); }
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

    void acceptVisitor(ExprTreeVisitor& pVisitor);

    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir) const;

    //*****************************************************************
    // Ordering Operations
    //*****************************************************************
    bool operator<(Node& rhs);
    bool operator<(OpNode& rhs);
    virtual int getOrder() const { return sOrder; }

  private:
    static const int sOrder = 100;
    OA_ptr<OpBasicInterface> mOpInterface;
  };

  class CallNode : public Node {
  public:
    CallNode() : Node("CallNode") { }
    CallNode(CallHandle h) : Node("CallNode"), mHandle(h) {}
    virtual ~CallNode() { }

    //! return a copy of self
    OA_ptr<Node> copy() 
      { OA_ptr<Node> ret; ret = new CallNode(mHandle); return ret; }

    bool isaCallNode() { return true; }
    CallHandle getHandle() const { return mHandle; }
    void dump(std::ostream& os) { Node::dump(os); }
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

    void acceptVisitor(ExprTreeVisitor& pVisitor);

    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir) const;

    //*****************************************************************
    // Ordering Operations
    //*****************************************************************
    bool operator<(Node& rhs);
    bool operator<(CallNode& rhs);
    virtual int getOrder() const { return sOrder; }

  private:
    //Changed from ExprHandle to CallHandle by LMR. 6.8.06
     static const int sOrder = 500;
     CallHandle mHandle;
  };

  class MemRefNode : public Node {
  public:
    MemRefNode() : Node("MemRefNode") { }
    MemRefNode(OA_ptr<MemRefExpr> mre) :
        Node("MemRefNode"),
        mMRE(mre)
    {}
    virtual ~MemRefNode() { }

    //! return a copy of self
    OA_ptr<Node> copy() 
      { OA_ptr<Node> ret; ret = new MemRefNode(mMRE); return ret; }

    bool isaMemRefNode() { return true; }
    OA_ptr<MemRefExpr> getMRE() const { return mMRE; }
    void dump(std::ostream& os) { Node::dump(os); }
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

    void acceptVisitor(ExprTreeVisitor& pVisitor);

    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir) const;

    //*****************************************************************
    // Ordering Operations
    //*****************************************************************
    bool operator<(Node& rhs);
    bool operator<(MemRefNode& rhs);
    virtual int getOrder() const { return sOrder; }

  private:
    static const int sOrder = 200;
    OA_ptr<MemRefExpr> mMRE;
  };

  class ConstSymNode : public Node {
  public:
    ConstSymNode() : Node("ConstSymNode") { }
    ConstSymNode(ConstSymHandle h) : Node("ConstSymNode"), mHandle(h) {}
    virtual ~ConstSymNode() { }

    //! return a copy of self
    OA_ptr<Node> copy() 
      { OA_ptr<Node> ret; ret = new ConstSymNode(mHandle); return ret; }

    bool isaConstSymNode() { return true; }
    ConstSymHandle getHandle() const { return mHandle; }
    void dump(std::ostream& os) { Node::dump(os); }
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

    void acceptVisitor(ExprTreeVisitor& pVisitor);

    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir) const;

    //*****************************************************************
    // Ordering Operations
    //*****************************************************************
    bool operator<(Node& rhs);
    bool operator<(ConstSymNode& rhs);
    virtual int getOrder() const { return sOrder; }

  private:
    static const int sOrder = 300;
    ConstSymHandle mHandle;
  };

  class ConstValNode : public Node {
  public:
    ConstValNode() : Node("ConstValNode") { }
    ConstValNode(OA_ptr<ConstValBasicInterface> constInter) :
        Node("ConstValNode"),
        mConstValInter(constInter)
    { }
    virtual ~ConstValNode() { }

    //! return a copy of self
    OA_ptr<Node> copy() 
      { OA_ptr<Node> ret; ret = new ConstValNode(mConstValInter); return ret; }

    bool isaConstValNode() { return true; }
    OA_ptr<ConstValBasicInterface> getConstValInterface() const {
        return mConstValInter;
    }
    void dump(std::ostream& os) { Node::dump(os); }
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

    void acceptVisitor(ExprTreeVisitor& pVisitor);

    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir) const;

    //*****************************************************************
    // Ordering Operations
    //*****************************************************************
    bool operator<(Node& rhs);
    bool operator<(ConstValNode& rhs);
    virtual int getOrder() const { return sOrder; }

  private:
    static const int sOrder = 400;
    OA_ptr<ConstValBasicInterface> mConstValInter;
  };
  
  //--------------------------------------------------------
  class Edge : public Tree::Edge {
  public:
    Edge(OA_ptr<Node> n1, OA_ptr<Node> n2)
      : Tree::Edge(n1, n2) { }
    virtual ~Edge() { }
    
    // included here to return subclass node type
    OA_ptr<Node>  parent () const { 
      OA_ptr<Tree::Node> n = Tree::Edge::parent();
      return n.convert<Node>();
    }
    OA_ptr<Node>  source () const { return parent(); }
    OA_ptr<Node>  tail () const { return parent(); }
    OA_ptr<Node>  child () const { 
      OA_ptr<Tree::Node> n = Tree::Edge::child();
      return n.convert<Node>(); 
    }
    OA_ptr<Node>  sink () const { return child(); }
    OA_ptr<Node>  head () const { return child(); }

    void dump(std::ostream& os);
    void dump() { dump(std::cout); }

  private:
  };  
  //--------------------------------------------------------
  
  //------------------------------------------------------------------
  /*! The node iterator iterates over all the nodes in the tree in no 
      particular order.   
      Need one in this class as well as
      super class so that return value of current method is subclass
      specific.
  */
  class NodesIterator : public Tree::NodesIterator {
  public:
    NodesIterator (const Tree& t) : Tree::NodesIterator(t) {}
    virtual ~NodesIterator () {}
    OA_ptr<Node>  current() const { 
      OA_ptr<Tree::Node> n = Tree::NodesIterator::current();
      return n.convert<Node>(); 
    }
  };
  
  //------------------------------------------------------------------
  /*! The edge iterator iterates over all the edges in the tree in no 
      particular order. 
      Need one in this class as well as
      super class so that return value of current method is subclass
      specific.
  */
  class EdgesIterator : public Tree::EdgesIterator {
  public:
    EdgesIterator (Tree& t) : Tree::EdgesIterator(t) {}
    virtual ~EdgesIterator () {}
    OA_ptr<Edge>  current() const { 
      OA_ptr<Tree::Edge> e = Tree::EdgesIterator::current();
      return e.convert<Edge>(); 
    }
  };

  //------------------------------------------------------------------
  /*! OutEdgesIterator iterates over all the non-null outgoing nodes 
      of a node. 
  */
  class OutEdgesIterator : public Tree::OutEdgesIterator {
  public:
    OutEdgesIterator (Node&  n) : Tree::OutEdgesIterator(n) {}
    virtual ~OutEdgesIterator () {}
    OA_ptr<Edge>  current() const { 
      OA_ptr<Tree::Edge> e = Tree::OutEdgesIterator::current();
      return e.convert<Edge>(); 
    }
  };
  
  //------------------------------------------------------------------
  /*! ChildNodesIterator iterates over all the children nodes of a node.  
   *  It skips the null edges.  Included here so that datatype information
   *  is not lost
   */
  class ChildNodesIterator : public Tree::ChildNodesIterator {
  public:
    ChildNodesIterator (const Node&  n) : Tree::ChildNodesIterator(n) {}
    ~ChildNodesIterator () {}
    OA_ptr<Node>  current() const { 
      OA_ptr<Tree::Node> n = Tree::ChildNodesIterator::current();
      return n.convert<Node>();
    }
  };
 
  void str(std::ostream& os);
  
  void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
  //void dump() { dump(std::cout); }

  //*****************************************************************
  // Annotation Interface
  //*****************************************************************
  void output(IRHandlesIRInterface& ir) const;

  //------------------------------------------------------------------
  /*! Pre-order iterator enumerates the nodes in pre-order (a node is 
      visited before all its sub-trees). 
  */
  class PreOrderIterator : public Tree::PreOrderIterator {
  public:
    PreOrderIterator (Tree& t) : Tree::PreOrderIterator(t) {}
    virtual ~PreOrderIterator () {}
    OA_ptr<Node>  current() const { 
      OA_ptr<Tree::Node> n = PreOrderIterator::current();
      return n.convert<Node>();
    }
  };
  //------------------------------------------------------------------
  /*! Post-order iterator enumerates the nodes in post-order 
      (a node's sub-trees are visited before the node). 
  */
  class PostOrderIterator : public Tree::PostOrderIterator {
  public:
    PostOrderIterator (Tree& t) : Tree::PostOrderIterator(t) {}
    virtual ~PostOrderIterator () {}
    OA_ptr<Node>  current() const { 
      OA_ptr<Tree::Node> n = PostOrderIterator::current();
      return n.convert<Node>();
    }
  };
  //------------------------------------------------------------------
  /*! Reverse post-order iterator, as the name suggests, enumerates 
      the nodes in the order that is reverse of post-order. 
  */
  class ReversePostOrderIterator : public Tree::ReversePostOrderIterator {
  public:
    ReversePostOrderIterator (Tree& t) : Tree::ReversePostOrderIterator(t) {}
    virtual ~ReversePostOrderIterator () {}
    OA_ptr<Node>  current() const { 
      OA_ptr<Tree::Node> n = Tree::ReversePostOrderIterator::current();
      return n.convert<Node>(); 
    }
  };
 //--------------------------------------------------------------------
};


} // end of OA namespace

#endif

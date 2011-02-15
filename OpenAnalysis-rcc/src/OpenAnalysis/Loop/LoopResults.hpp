/*! \file
  
  \brief Loop results class that implements the results interface.

  \authors Andy Stone (aistone@gmail.com)

  Copyright (c) 2007, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef LoopResults_H
#define LoopResults_H

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include "Interface.hpp"
#include <vector>
#include <list>
#include <map>
using namespace std;

namespace OA {
  namespace Loop {

/*! Class to represent loop abstractions in a tree structure */
class LoopTreeNode {
  public:
    LoopTreeNode(OA_ptr<LoopAbstraction> loop) :
        mLoop(loop)
    {
        mChildren = new vector<OA_ptr<LoopTreeNode> >();
    }

    /*! Get the loop abstraction represented by this node */
    OA_ptr<LoopAbstraction> getLoopAbstraction() { return mLoop; }

    /*! Add a child node to the tree. Return a pointer to the node */
    OA_ptr<LoopTreeNode> addChild(OA_ptr<LoopAbstraction> loop) {
        OA_ptr<LoopTreeNode> node;
        node = new LoopTreeNode(loop);
        mChildren->push_back(node);
        return node;
    }

    /*! Return the number of children this node has */
    int getNumChildren() { return mChildren->size(); }

    /*! Find the node for a specified loop in this subtree.  This function
        is static and is passed the node to search because otherwise the
        find function would set an OA_ptr to this upon finding the
        corresponding node, this is an error. */
    static OA_ptr<LoopTreeNode> find(
        OA_ptr<LoopAbstraction> loop,
        OA_ptr<LoopTreeNode> node);

    /* Return a specified child */
    OA_ptr<LoopTreeNode> getChild(int num) {
        return (*mChildren)[num];
    }

    // print self and recurse on childrens
    void print(IRHandlesIRInterface &ir, int indentation = 0);

  private:

    OA_ptr<LoopAbstraction> mLoop;
    OA_ptr<vector<OA_ptr<LoopTreeNode> > > mChildren;
};

/* iterators for common STL-container objects */
typedef ListIterator<OA_ptr<LoopTreeNode> > LoopTreeNodeIterator;

/*! The results class implements the results interface (in Interface.hpp).
    It's a simple implementation, no attempt is made for optimization.  It
    merely stores all loops in an STL list. */
class LoopResults :  public Interface {
  public:
  LoopResults(IRHandlesIRInterface &irHndlsInter) :
    mIRHndlsInter(irHndlsInter)
  { 
      mLoopTreeRoots = new list<OA_ptr<LoopTreeNode> >();
  }

  //[Result queries]
    // derived from Interface
    virtual LoopIterator getLoopIterator();
    virtual LoopIterator postOrderLoopIterator();

  //[Result builder]
    // derived from Interface
    virtual void addLoop(OA_ptr<LoopAbstraction> l);

  //[Results queries]
    // derived from Interface
    virtual OA_ptr<LoopAbstraction> getLoop(StmtHandle stmt);

    // derived from Interface
    // NOTE: it would probably be a good idea to use memiozation on this
    //       sort of thing.
    virtual OA_ptr<list<OA_ptr<LoopIndex> > >
        getIndexVars(OA_ptr<LoopAbstraction> loop);

  //[Output]
    void printLoopTree();

  private:
    void postOrderTraversal(
        OA_ptr<LoopTreeNode> node,
        OA_ptr<list<OA_ptr<LoopAbstraction> > > loopList);

  // member variables
    list<OA_ptr<LoopAbstraction> > mLoops;
    map<StmtHandle, OA_ptr<LoopAbstraction> > mStmt2LoopMap;
    OA_ptr<list<OA_ptr<LoopTreeNode> > > mLoopTreeRoots;
    IRHandlesIRInterface &mIRHndlsInter;
};

} } // namespaces

#endif


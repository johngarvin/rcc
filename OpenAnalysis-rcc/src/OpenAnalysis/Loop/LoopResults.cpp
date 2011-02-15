/*! \file
  
  \brief Implementation of LoopResults class

  \authors Andy Stone (aistone@gmail.com)

  Copyright (c) 2007, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "LoopResults.hpp"

namespace OA {
  namespace Loop {

OA_ptr<LoopTreeNode> LoopTreeNode::find(
    OA_ptr<LoopAbstraction> loop,
    OA_ptr<LoopTreeNode> node)
{
    // search the tree in a pre-order traversal

    // look at this node
    if(node->getLoopAbstraction().ptrEqual(loop)) {
        OA_ptr<LoopTreeNode> ptr;
        ptr = node;
        return ptr;
    }

    // search the children one-by-one
    OA_ptr<LoopTreeNode> found;
    for(int i = 0; i < node->getNumChildren(); i++) {
        OA_ptr<LoopTreeNode> child;
        child = node->getChild(i);
        found = LoopTreeNode::find(loop, child);
        if(!found.ptrEqual(NULL)) { return found; }
    }

    OA_ptr<LoopTreeNode> nullPtr;
    return nullPtr;
}

void LoopTreeNode::print(IRHandlesIRInterface &ir, int indentation) {
    // print self
    for(int i = 0; i < indentation; i++) { cout << "    "; }
    cout << ir.toString(mLoop->getLoopStatement()) << endl;

    // print the children recursively, increase the indentation on them
    for(vector<OA_ptr<LoopTreeNode> >::iterator i = mChildren->begin();
        i != mChildren->end(); i++)
    {
        (*i)->print(ir, indentation + 1);
    }
}

LoopIterator LoopResults::getLoopIterator() {
    OA_ptr<list<OA_ptr<LoopAbstraction> > > loopList;
    loopList = &mLoops;
    LoopIterator ret(loopList);
    return ret;
}

LoopIterator LoopResults::postOrderLoopIterator() {
    // TODO: Implement
    assert(false);
}

void LoopResults::postOrderTraversal(
    OA_ptr<LoopTreeNode> node,
    OA_ptr<list<OA_ptr<LoopAbstraction> > > loopList)
{
    // iterate through the children
    for(int i = 0; i < node->getNumChildren(); i++) {
        postOrderTraversal(node->getChild(i), loopList);
    }

    // add this node (if its not the NULL, top, node)
    OA_ptr<LoopAbstraction> ptr = node->getLoopAbstraction();
    if(!ptr.ptrEqual(NULL)) {
        loopList->push_back(ptr);
    }
}

void LoopResults::addLoop(OA_ptr<LoopAbstraction> l) {
    // simply add the passed loop to our list of loops
    mLoops.push_back(l);
    mStmt2LoopMap[l->getLoopStatement()] = l;

    // if we have no roots add this loop as one
    if(mLoopTreeRoots->size() == 0) {
        OA_ptr<LoopTreeNode> newRoot;
        newRoot = new LoopTreeNode(l);
        mLoopTreeRoots->push_back(newRoot);
        return;
    // if there are existing roots iterate through them
    } else {   
        for(LoopTreeNodeIterator i(mLoopTreeRoots);
            i.isValid(); i++)
        {
            OA_ptr<LoopTreeNode> node = i.current();

            // try to find the parent in this subtree
            OA_ptr<LoopTreeNode> parentNode = 
                LoopTreeNode::find(l->getParent(), node);
            // if a parent can be found connect it to the child
            if(!parentNode.ptrEqual(NULL)) {
                parentNode->addChild(l);
                return;
            }
        }
    }

    // if no parent was found add this asscociate this loop with a new
    // root node in the tree
    OA_ptr<LoopTreeNode> newRoot;
    newRoot= new LoopTreeNode(l);
    mLoopTreeRoots->push_back(newRoot);
}

OA_ptr<LoopAbstraction> LoopResults::getLoop(StmtHandle stmt) {
    OA_ptr<LoopAbstraction> loop;

    map<StmtHandle, OA_ptr<LoopAbstraction> >::iterator i;
    i = mStmt2LoopMap.find(stmt);
    if(i == mStmt2LoopMap.end()) { return loop; }
    loop = i->second;
}

OA_ptr<list<OA_ptr<LoopIndex> > >
LoopResults::getIndexVars(OA_ptr<LoopAbstraction> loop)
{
    OA_ptr<list<OA_ptr<LoopIndex> > > vars;
    vars = new list<OA_ptr<LoopIndex> >();

    // treat the list as if it were a stack, pushing named locations for
    // index variables on as we traverse up through the loop hierachy
    while(!loop.ptrEqual(NULL)) {
        vars->push_front(loop->getLoopIndex());

        loop = loop->getParent();
    }

    return vars;
}

void LoopResults::printLoopTree() {
    // iterate across all roots, print each tree separately
    for(LoopTreeNodeIterator i(mLoopTreeRoots);
            i.isValid(); i++)
    {
        i.current()->print(mIRHndlsInter);
    }
}

} } // end namespaces

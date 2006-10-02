/* $Id: NonUniformDegreeTree.cc,v 1.2 2006/10/02 01:55:39 garvin Exp $ */
/****************************************************************************/
//                 Copyright (c) 1990-1999 Rice University
//                          All Rights Reserved
/****************************************************************************/

//***************************************************************************
//
// NonUniformDegreeTree.C
//
//   a general purpose abstraction for non-uniform degree trees. 
//   children of a node are represented by a circularly linked-list
//   of siblings.
//
//   two iterators are included here. one for enumerating the children
//   of a node, and a second for enumerating a tree rooted at a node.
//
//   these abstractions are useless in their own right since the tree
//   contains only structural information. to make use of the abstraction,
//   derive a tree node class that contains some useful data. all of the
//   structural manipulation can be performed using the functions provided
//   in the base classes defined here.
//
// Author: John Mellor-Crummey
//
// Creation Date: September 1991
//
// Modification History:
//  see NonUniformDegreeTree.h
//
//***************************************************************************


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <support/trees/NonUniformDegreeTree.h>



//***************************************************************************
// class NonUniformDegreeTreeNode interface operations
//***************************************************************************

//-----------------------------------------------
// constructor initializes empty node then links
// it to its parent and siblings (if any)
//-----------------------------------------------
NonUniformDegreeTreeNode::NonUniformDegreeTreeNode
(NonUniformDegreeTreeNode *_parent)
{
  // no parent
  this->parent = 0;
  
  // no children
  child_count = 0; children = 0;
  
  // initial circular list of siblings includes only self
  next_sibling = this; prev_sibling = this;
  
  Link(_parent); // link to parent and siblings if any
}


//-----------------------------------------------
// links a node to a parent and at the end of the 
// circular doubly-linked list of its siblings 
// (if any)
//-----------------------------------------------
void NonUniformDegreeTreeNode::Link(NonUniformDegreeTreeNode *newParent)
{
  assert(this->parent == 0); // can only have one parent
  if (newParent != 0) {
    // children maintained as a doubly linked ring. 
    // a new node is linked at the end of the ring (as a predecessor 
    // of "parent->children") which points to first child in the ring
    
    NonUniformDegreeTreeNode *first_sibling = newParent->children;
    if (first_sibling) LinkAfter(first_sibling->prev_sibling);
    else {
      newParent->children = this; // solitary child
      newParent->child_count++;
      parent = newParent;
    }
  }
}


//-----------------------------------------------
void NonUniformDegreeTreeNode::LinkAfter(NonUniformDegreeTreeNode *sibling)
{
  assert(sibling);
  assert(this->parent == 0); // can only have one parent
  
  this->parent = sibling->parent;
  if (parent) parent->child_count++;
    
  // children maintained as a doubly linked ring. 
    
  // link forward chain
  next_sibling = sibling->next_sibling;
  sibling->next_sibling = this;
      
  // link backward chain
  prev_sibling = sibling;
  next_sibling->prev_sibling = this;
}


//-----------------------------------------------
void NonUniformDegreeTreeNode::LinkBefore(NonUniformDegreeTreeNode *sibling)
{
  assert(sibling);
  LinkAfter(sibling->prev_sibling);
  if (parent && sibling == parent->children) {
    parent->children = this;
  }
}

//-----------------------------------------------
// unlinks a node from a parent and siblings
//-----------------------------------------------
void NonUniformDegreeTreeNode::Unlink()
{
  if (parent != 0) {
    // children maintained as a doubly linked ring. 
    // excise this node from from the ring 
    if (--(parent->child_count) == 0) {
      // current node is removed as only child of parent
      // leaving it linked in a circularly linked list with
      // itself
      parent->children = 0;
    } else {
      // fix link from parent to the ring if necessary
      if (parent->children == this)
	parent->children = next_sibling;
	
      // relink predecessor's forward link
      prev_sibling->next_sibling = next_sibling;
      
      // relink successor's backward link
      next_sibling->prev_sibling = prev_sibling;
      
      // relink own pointers into self-circular configuration
      prev_sibling = this;
      next_sibling = this;
    }
  }
  this->parent = 0;
}


//-----------------------------------------------
// virtual destructor that frees all of its 
// children before freeing itself 
//-----------------------------------------------
NonUniformDegreeTreeNode::~NonUniformDegreeTreeNode()
{
  if (child_count > 0) {
    NonUniformDegreeTreeNode *next, *start = children;
    for (int i = child_count; i-- > 0; ) {
      next = start->next_sibling;
      delete start;
      start = next;
    }
  }
}

unsigned int
NonUniformDegreeTreeNode::AncestorCount() const
{
  unsigned int ancestorCount = 0;
  for (NonUniformDegreeTreeNode* ancestor = parent;
       ancestor;
       ancestor = ancestor->parent) {
    ancestorCount++;
  }
  return ancestorCount;
}


//****************************************************************************
// class NonUniformDegreeTreeNodeChildIterator interface operations
//****************************************************************************

CLASS_NAME_IMPL(NonUniformDegreeTreeNodeChildIterator)


NonUniformDegreeTreeNodeChildIterator::NonUniformDegreeTreeNodeChildIterator
(const NonUniformDegreeTreeNode* _parent, bool firstToLast) : 
parent(_parent), currentChild(0), forward(firstToLast)
{
  Reset();
}


NonUniformDegreeTreeNodeChildIterator::~NonUniformDegreeTreeNodeChildIterator()
{
}

NonUniformDegreeTreeNode *NonUniformDegreeTreeNodeChildIterator::Current() const
{
  return currentChild;
}


void *NonUniformDegreeTreeNodeChildIterator::CurrentUpCall() const
{
  return Current();
}


void NonUniformDegreeTreeNodeChildIterator::Reset()
{
  currentChild = forward ? parent->FirstChild() : parent->LastChild();
}


void NonUniformDegreeTreeNodeChildIterator::operator++()
{
  if (currentChild) {
    currentChild = forward
                     ? currentChild->NextSibling()
                     : currentChild->PrevSibling();
    const NonUniformDegreeTreeNode* pastEnd = forward
                                                  ? parent->FirstChild()
                                                  : parent->LastChild();
    if (currentChild == pastEnd) 
      currentChild = 0;
  }
}


void NonUniformDegreeTreeNodeChildIterator::operator++(int)
{
  operator++();
}



//****************************************************************************
// class NonUniformDegreeTreeIterator interface operations
//****************************************************************************

CLASS_NAME_IMPL(NonUniformDegreeTreeIterator)


NonUniformDegreeTreeIterator::NonUniformDegreeTreeIterator(
                                  const NonUniformDegreeTreeNode* root,
				  TraversalOrder torder,
				  NonUniformDegreeTreeEnumType how) :
  IteratorStack(torder, (how == NON_UNIFORM_DEGREE_TREE_ENUM_LEAVES_ONLY)
                            ? ITER_STACK_ENUM_LEAVES_ONLY
		            : ITER_STACK_ENUM_ALL_NODES)
{
  StackableIterator* top = 0;
  if (how == NON_UNIFORM_DEGREE_TREE_ENUM_ALL_NON_ROOTS)
  {// make an iterator for the next (non-root) level
    top = IteratorToPushIfAny((void*) root);
  }
  else
  {// make a singleton iterator for the root
    top = new SingletonIterator(root,
                               (torder == PostOrder) ? PostVisit : PreVisit);
  }
  if (top)
  {// there is something to push on the stack
    Push(top);
  }
}


NonUniformDegreeTreeIterator::~NonUniformDegreeTreeIterator()
{
}


NonUniformDegreeTreeNode *NonUniformDegreeTreeIterator::Current() const
{
  return (NonUniformDegreeTreeNode *) IteratorStack::CurrentUpCall();
}


void *NonUniformDegreeTreeIterator::CurrentUpCall() const
{
  return Current();
}


StackableIterator *NonUniformDegreeTreeIterator::IteratorToPushIfAny
(void *current)
{
  NonUniformDegreeTreeNode *node = (NonUniformDegreeTreeNode *) current;

  if (GetTraversalOrder() == PreAndPostOrder) {
    StackableIterator *top = (StackableIterator *) Top();
    if (top->ClassName() != CLASS_NAME(SingletonIterator)) {
      Push(new SingletonIterator(node, PostVisit));
    } else {
      if (((SingletonIterator *) top)->VisitType() == PreVisit) { 
	Push(new SingletonIterator(node, PostVisit));
      } else return 0;
    }
  }

  return (node->ChildCount() > 0)
       ? new NonUniformDegreeTreeNodeChildIterator(node, IterationIsForward())
       : 0;
}

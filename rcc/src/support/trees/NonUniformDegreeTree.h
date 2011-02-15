/* $Id: NonUniformDegreeTree.h,v 1.1 2005/08/31 05:15:38 johnmc Exp $ */
/****************************************************************************/
//                 Copyright (c) 1990-1999 Rice University
//                          All Rights Reserved
/****************************************************************************/
#ifndef NonUniformDegreeTree_h
#define NonUniformDegreeTree_h

//***************************************************************************
//
// NonUniformDegreeTree.h
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
//  November 1994 -- John Mellor-Crummey
//     -- reimplemented iterators in terms of StackableIterator and
//        IteratorStack, increasing functionality while reducing code
//        volume.
//
//***************************************************************************


#ifndef IteratorStack_h
#include <support/iterators/IteratorStack.h>
#endif



//***************************************************************************
// class NonUniformDegreeTreeNode
//***************************************************************************

class NonUniformDegreeTreeNode {
public:
  NonUniformDegreeTreeNode(NonUniformDegreeTreeNode *_parent = 0);
  
  // destructor for destroying derived class instances
  virtual ~NonUniformDegreeTreeNode();
  
  // link/unlink a node to a parent and siblings 
  void Link(NonUniformDegreeTreeNode *_parent);
  void LinkBefore(NonUniformDegreeTreeNode *sibling);
  void LinkAfter(NonUniformDegreeTreeNode *sibling);
  void Unlink();

  // returns the number of ancestors walking up the tree
  unsigned int AncestorCount() const;
  
  // functions for inspecting links to other nodes
  unsigned int ChildCount() const { return child_count; };

public:
  NonUniformDegreeTreeNode *Parent() const { return parent; };
  NonUniformDegreeTreeNode *NextSibling() const { return next_sibling; };
  NonUniformDegreeTreeNode *PrevSibling() const { return prev_sibling; };
  NonUniformDegreeTreeNode *FirstChild() const { return children; };
  NonUniformDegreeTreeNode *LastChild() const
    { return children ? children->prev_sibling : 0; };

private:
  NonUniformDegreeTreeNode *parent, *children, *next_sibling, *prev_sibling;
  unsigned int child_count;

friend class NonUniformDegreeTreeNodeChildIterator;
friend class NonUniformDegreeTreeIterator;
};



//***************************************************************************
// class NonUniformDegreeTreeNodeChildIterator
//***************************************************************************

class NonUniformDegreeTreeNodeChildIterator : public StackableIterator {
public:
  NonUniformDegreeTreeNodeChildIterator(
                                     const NonUniformDegreeTreeNode* _parent,
				     bool firstToLast = true);
  ~NonUniformDegreeTreeNodeChildIterator();

  void Reset(void);
  void operator++();    // prefix increment
  void operator++(int); // postfix increment

  CLASS_NAME_FDEF(NonUniformDegreeTreeNodeChildIterator);

// protected is commented out because NonUniformDegreeTreeIterators are used
// directly by deprecated (but still necessary) code in the Fortran D compiler.
// it should be uncommented when the offending code is rewritten.
// --JMC 4 Nov 1994
// protected:
  NonUniformDegreeTreeNode *Current() const;

private:
  void *CurrentUpCall() const; // interface for StackableIterator
  const NonUniformDegreeTreeNode *parent;
  NonUniformDegreeTreeNode *currentChild;
  bool forward;
};

CLASS_NAME_EDEF(NonUniformDegreeTreeNodeChildIterator);


//***************************************************************************
// class NonUniformDegreeTreeIterator
//***************************************************************************

enum NonUniformDegreeTreeEnumType {
  NON_UNIFORM_DEGREE_TREE_ENUM_LEAVES_ONLY = ITER_STACK_ENUM_LEAVES_ONLY,
  NON_UNIFORM_DEGREE_TREE_ENUM_ALL_NODES = ITER_STACK_ENUM_ALL_NODES,
  NON_UNIFORM_DEGREE_TREE_ENUM_ALL_NON_ROOTS
  };

// Note:  Reverse traversal orders are OK.

class NonUniformDegreeTreeIterator : public IteratorStack {
public:
  NonUniformDegreeTreeIterator
    (const NonUniformDegreeTreeNode *root, TraversalOrder torder = PreOrder,
     NonUniformDegreeTreeEnumType how= NON_UNIFORM_DEGREE_TREE_ENUM_ALL_NODES);
  ~NonUniformDegreeTreeIterator();

  CLASS_NAME_FDEF(NonUniformDegreeTreeIterator);

// protected is commented out because NonUniformDegreeTreeIterators are used
// directly by deprecated (but still necessary) code in the Fortran D compiler.
// it should be uncommented when the offending code is rewritten.
// --JMC 4 Nov 1994
// protected:
  NonUniformDegreeTreeNode *Current() const;

private:
  // upcall interface for StackableIterator
  void *CurrentUpCall() const; 

  // upcall for IteratorStack
  StackableIterator *IteratorToPushIfAny(void *current); 
};

CLASS_NAME_EDEF(NonUniformDegreeTreeIterator);


#endif 

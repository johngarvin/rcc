/* $Id: NonUniformDegreeTreeTmpl.h,v 1.1 2005/08/31 05:15:38 johnmc Exp $ */
//**************************************************************************
//                 Copyright (c) 1990-1999 Rice University
//                          All Rights Reserved
//**************************************************************************
#ifndef NonUniformDegreeTreeTmpl_h
#define NonUniformDegreeTreeTmpl_h

//***************************************************************************
//
// File: NonUniformDegreeTreeTmpl.h
//
// Purpose:
//   a typesafe template for deriving classes based on non-uniform degree 
//   trees and their iterators
//
// History:
//  28 November 1995 - John Mellor-Crummey - created
//
//***************************************************************************


#ifndef NonUniformDegreeTree_h
#include <support/trees/NonUniformDegreeTree.h>
#endif



//***************************************************************************
// class NonUniformDegreeTreeNodeTmpl
//***************************************************************************

template <class T>
class NonUniformDegreeTreeNodeTmpl : public NonUniformDegreeTreeNode {
public:
  NonUniformDegreeTreeNodeTmpl(T* tparent = 0) :
  NonUniformDegreeTreeNode(tparent) { };
  
  T* NextSibling() const { 
    return (T*) NonUniformDegreeTreeNode::NextSibling();
  };
  T* PrevSibling() const { 
    return (T*) NonUniformDegreeTreeNode::PrevSibling();
  };
  T* FirstChild() const { 
    return (T*) NonUniformDegreeTreeNode::FirstChild();
  };
  T* LastChild() const { 
    return (T*) NonUniformDegreeTreeNode::LastChild();
  };
  T* Parent() const { 
    return (T*) NonUniformDegreeTreeNode::Parent();
  };
};


//***************************************************************************
// class NonUniformDegreeTreeNodeChildIteratorTmpl
//***************************************************************************

template <class T>
class NonUniformDegreeTreeNodeChildIteratorTmpl : 
public NonUniformDegreeTreeNodeChildIterator {
public:
  NonUniformDegreeTreeNodeChildIteratorTmpl(const NonUniformDegreeTreeNode* node,
                                            bool firstToLast = true) :
    NonUniformDegreeTreeNodeChildIterator(node, firstToLast) {};
  T* Current() const { return (T*) NonUniformDegreeTreeNodeChildIterator::Current(); };
};


//***************************************************************************
// class NonUniformDegreeTreeIteratorTmpl
//***************************************************************************

// Note:  Reverse traversal orders are OK.

template <class T>
class NonUniformDegreeTreeIteratorTmpl : public NonUniformDegreeTreeIterator {
public:
  NonUniformDegreeTreeIteratorTmpl(const NonUniformDegreeTreeNode* node,
                                   TraversalOrder torder = PreOrder,
                                   NonUniformDegreeTreeEnumType how =
                                      NON_UNIFORM_DEGREE_TREE_ENUM_ALL_NODES) :
    NonUniformDegreeTreeIterator(node, torder, how)
    {};
  T* Current() const { return (T*) NonUniformDegreeTreeIterator::Current(); };
};


#endif 

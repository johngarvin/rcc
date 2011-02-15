/* $Id: StackTmpl.h,v 1.1 2005/09/02 13:40:18 johnmc Exp $ */
/*****************************************************************************/
//                 Copyright (c) 1990-1999 Rice University
//                          All Rights Reserved
/*****************************************************************************/

//*******************************************************************
//
// File:
//    StackTmpl.h
//
// Purpose:
//    The StackTmpl is served as a template for a stack.
//
// Description:
//    In order to use it, instantiate an instance first.
//    ex: StackTmpl<int> intStack;
//
// Operations:
//   void Push( WHATEVER ele); puts an element ele in the stack.
//   WHATEVER Pop(); removes top element from the stack and returns it.
//   WHATEVER Top(); returns top element. Stack is unchanged.
//   bool IsEmpty() checks whether the stack is empty or not; 
//                     returns true if the stack is empty.
//   void print_Stack(); prints all elements in the stack.
// 
// Author: Lei Zhou                                   March 1995
//
//                 Copyright (c) 1990-1999 Rice University
//                          All Rights Reserved
//******************************************************************/

#ifndef StackTmpl_h
#define StackTmpl_h

//************************** System Include Files **************************/

#include <ostream>

//*************************** User Include Files ***************************/

//************************ External Declarations ***************************/


//**************************** Class Definitions ***************************/
template <class T> class StackTmpl;

//-------------------------------------------------------------
//  template <class T> class StackTmplCell
//-------------------------------------------------------------
template <class T> 
class StackTmplCell {
private:
  
  StackTmplCell *next;
  T item;
  // constructors
  StackTmplCell(const T& r, StackTmplCell<T> *c): item(r), next(c) {}
  
  // each StackTmplCell instantiation has as its friend
  // the associated StackTmpl instantiation
  friend class StackTmpl<T>;
};


//-------------------------------------------------------------
//  template<class T> class StackTmpl
//-------------------------------------------------------------

template <class T> 
class StackTmpl {
public:
  // constructors
  StackTmpl() : rep(0), depth(0) { }
  
  // destructor
  ~StackTmpl() { while(depth >0) Pop(); }
  
  // modifier methods
  void	Push	(const T& v) { ++depth; rep = new StackTmplCell<T>(v, rep); }
  T	Pop	() { 
    --depth; 
    StackTmplCell<T> *c = rep;    // keep rep ptr in c
    T ret = c->item;     // get c's data (copy it, since c is deleted)
    rep = c->next;       // put c's next element in stack to new rep
    delete c;            // c is useless, delete it
    return ret;          // return the value desired
  }
    
  // accessor methods
  T		Top	() const { return rep->item; }
  bool	IsEmpty	() const { return (rep == 0) ? true : false; }
  int		Depth	() const { return depth; }
  
private:
  StackTmplCell<T> *rep;
  int depth;
};

/*****************************************************************************/

#endif /* end StackTmpl_h */

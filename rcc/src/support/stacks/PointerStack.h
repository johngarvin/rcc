/* $Id: PointerStack.h,v 1.1 2005/08/31 05:15:38 johnmc Exp $ */
/******************************************************************************/
//                 Copyright (c) 1990-1999 Rice University
//                          All Rights Reserved
/******************************************************************************/
//***************************************************************************
// 
// PointerStack.h: 
// 
// Author:  John Mellor-Crummey                               October 1993
//
//                 Copyright (c) 1990-1999 Rice University
//                          All Rights Reserved
//                                                                          
// rjf	2-21-98 Replaced previous versions of PointerStack with
//              a self-contained implementaion for efficiency and
//		to avoid using templates when building runtime libraries
//		on machines on which the compiler itself does not run.
//***************************************************************************

#ifndef PointerStack_h
#define PointerStack_h


class PointerStack {
  int topElement;
  int lastSlot;
  void **theStack;
  void    ExtendAndPush( void* item);
public:
  PointerStack(unsigned int initialSize = 32);
  ~PointerStack();

  void Push(void *item) {  // push a new item on the top of the stack 
    if ( topElement < lastSlot ) {
      theStack[++topElement] = item; 
    }
    else
      ExtendAndPush(item);
  };

  void *Pop() {
    return (topElement >=0) // pop and return the top item (0 when empty)
      ?  theStack[topElement--]
      :  (void *) 0 ;
  };

  void *Top(){
    return (topElement >=0) // return the top item (0 when empty)
      ? theStack[topElement]
      : (void *) 0 ;
  }; 
           
  // return the item that is "depth" elements from the top of the stack
  // Get(0) returns the top of the stack (0 when empty)
  void *Get(unsigned int depth) {
    return   (depth <= topElement)
      ? theStack[topElement - depth]
      : (void *) 0 ;
  };

  unsigned int Depth()  {    // 0 when empty
    return (topElement + 1) ;
  } ;

};

#endif





/* $Id: PointerStack.C,v 1.1 2005/08/31 05:15:38 johnmc Exp $ */
/******************************************************************************/
//                 Copyright (c) 1990-1999 Rice University
//                          All Rights Reserved
/******************************************************************************/
//***************************************************************************
// 
// PointerStack.C: 
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

#include <stdlib.h>
#include <assert.h>

#include <support/stacks/PointerStack.h>

PointerStack::PointerStack(unsigned int initialSize)
{
  theStack = (void **) malloc( initialSize * sizeof(void *));
  topElement = -1; // initially empty
  lastSlot = initialSize -1;
}


PointerStack::~PointerStack()
{
  free( (void *) theStack) ;
}

void
PointerStack::ExtendAndPush(void *item) {
  
  int size = lastSlot + 1; 
  int newsize; 
  if( size < 256 ) {
    newsize = 256;
  }
  else
    newsize = size * 2 ;  // Grow it fast to avoid reallocs.

  theStack = (void **)  realloc( (void *) theStack, newsize * sizeof(void *));

  lastSlot = newsize - 1;

  // Finally do the push.
  theStack[++topElement] = item;
}












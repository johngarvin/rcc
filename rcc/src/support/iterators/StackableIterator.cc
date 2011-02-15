/* $Id: StackableIterator.cc,v 1.2 2006/10/02 01:55:39 garvin Exp $ */
/******************************************************************************/
//                 Copyright (c) 1990-1999 Rice University
//                          All Rights Reserved
/******************************************************************************/
//***************************************************************************
// StackableIterator.C
//
//   a base set of functionality for iterators that can be used with the
//   IteratorStack abstraction to traverse nested structures 
//
// Author: John Mellor-Crummey                                
//
// Creation Date: October 1993
//
// Modification History:
//   see StackableIterator.h 
// 
//***************************************************************************


#include <support/iterators/StackableIterator.h>
#if 0
#include <support/msgHandlers/DumpMsgHandler.h>
#endif



//***************************************************************************
// class StackableIterator interface operations
//***************************************************************************

CLASS_NAME_IMPL(StackableIterator)


StackableIterator::StackableIterator()  
{
  
}


StackableIterator::~StackableIterator()
{
}


void StackableIterator::operator++(int)
{
  this->operator++();
}


bool StackableIterator::IsValid() const
{
  return this->CurrentUpCall() != 0;
}


#if 0
void StackableIterator::Dump() 
{
  dumpHandler.Dump("[%s %x; current = %x]\n", ClassName(), this, 
		   CurrentUpCall());
  DumpUpCall();
}


void StackableIterator::DumpUpCall() 
{
}
#endif






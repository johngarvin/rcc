/* $Id: IteratorStack.cc,v 1.1 2005/08/31 05:15:37 johnmc Exp $ */
/******************************************************************************/
//                 Copyright (c) 1990-1999 Rice University
//                          All Rights Reserved
/******************************************************************************/
//***************************************************************************
// IteratorStack.C
//
//   an iterator that is realized as a stack of iterators. this abstraction 
//   is useful for traversing nested structures.
//
// Author: John Mellor-Crummey                                
//
// Creation Date: October 1993
//
// Modification History:
//  see IteratorStack.h 
//
//***************************************************************************


#include <assert.h>
#include <stdlib.h>

#include <support/iterators/IteratorStack.h>
#include <support/stacks/PointerStack.h>

#if 0
#include <support/msgHandlers/DumpMsgHandler.h>
#endif

struct IteratorStackS {
  PointerStack pstack;
};


CLASS_NAME_IMPL(IteratorStack);


IteratorStack::IteratorStack(TraversalOrder torder, 
			     IterStackEnumType _enumType)  
{
  iteratorStackRepr = new IteratorStackS;
  InitTraversal(torder, _enumType);
}


IteratorStack::~IteratorStack()
{
  FreeStack(0);
  delete iteratorStackRepr;
}

StackableIterator* IteratorStack::Top() const
{
  return (StackableIterator*) iteratorStackRepr->pstack.Top();
}

StackableIterator *IteratorStack::GetIteratorAtPosition(unsigned int depth) const
{
  return (StackableIterator*) iteratorStackRepr->pstack.Get(depth);
}


void IteratorStack::Push(StackableIterator* newtop)
{
  while (newtop != 0) {
    if (newtop->CurrentUpCall() == 0) {
      // don't really push empty iterators
      delete newtop;
      break;
    }
    iteratorStackRepr->pstack.Push(newtop);
    if (traversalOrder != PostOrder) break;
    newtop = IteratorToPushIfAny(newtop->CurrentUpCall());
  }
}


void IteratorStack::operator++()
{
  (*this)++;
}


void IteratorStack::operator++(int)
{
  for(;;) {
    StackableIterator* top = Top();
    if (top == 0) break;
    
    if ((traversalOrder == PreOrder) || (traversalOrder == PreAndPostOrder)) {
      void* current = top->CurrentUpCall();
      (*top)++; // advance iterator at the top of stack
      if (current) {
        Push(IteratorToPushIfAny(current));
        top = Top();
      }
    }
    else
      (*top)++; // advance iterator at the top of stack
    
    if (top->IsValid() == false) {
      bool popped = false;
      while ((Top()->IsValid() == false) &&
	     (iteratorStackRepr->pstack.Depth() > 1)) {
	FreeTop();
	popped = true;
      }
      if (popped && (enumType == ITER_STACK_ENUM_LEAVES_ONLY))
        continue;
    } else if (traversalOrder == PostOrder) {
      void* current = top->CurrentUpCall();
      if (current) {
        Push(IteratorToPushIfAny(current));
      }
    }
    break;
  }
}


void IteratorStack::ReConstruct(TraversalOrder torder, 
				IterStackEnumType _enumType) 
{
  InitTraversal(torder, _enumType);
  FreeStack(0);
}


void IteratorStack::Reset()
{
  FreeStack(1); // leave at most the top element on stack
  StackableIterator* top = Top();
  if (top) {
    top->Reset();
    if (traversalOrder == PostOrder) 
      Push(IteratorToPushIfAny(top->CurrentUpCall()));
  }
}

void IteratorStack::Reset(TraversalOrder torder, IterStackEnumType _enumType)
{
  InitTraversal(torder, _enumType);
  Reset();
}


void* IteratorStack::CurrentUpCall() const
{
  StackableIterator* top = Top();
  return (top ? top->CurrentUpCall() : 0);
}


bool IteratorStack::IsValid() const
{
  StackableIterator* top = Top();
  return (top ? top->IsValid() : false);
}

TraversalVisitType IteratorStack::VisitType() const
{  
  switch(clientTraversalOrder) {
  case PreOrder:
  case ReversePreOrder:
    return PreVisit;
  case PostOrder: 
  case ReversePostOrder:
    return PostVisit;
//case ReversePreAndPostOrder:
  case PreAndPostOrder: {
    StackableIterator* top = (StackableIterator*) Top();
    if (top == 0)
      assert(0);
    else if (top->ClassName() == CLASS_NAME(SingletonIterator))
      return ((SingletonIterator*) top)->VisitType();
    else
      return PreVisit;
  }
  default:
    assert(0);
  }
  return PostVisit;  // bogus return--not reached
}

TraversalOrder IteratorStack::GetTraversalOrder() const
{  
  return clientTraversalOrder;
}

bool IteratorStack::IterationIsForward() const
{
  switch(clientTraversalOrder) {
  case PreOrder:
  case PostOrder: 
  case PreAndPostOrder:
    return true;
  case ReversePreOrder:
  case ReversePostOrder:
//case ReversePreAndPostOrder:
    return false;
  default:
    assert(0);
  }
  return false;  // bogus return--not reached
}

int IteratorStack::Depth() const
{
  return iteratorStackRepr->pstack.Depth();
}


void IteratorStack::FreeTop()
{
  StackableIterator* top= (StackableIterator*) iteratorStackRepr->pstack.Pop();
  if (top)
    delete top;
}


// free the top (depth - maxDepth) elements on the stack, leaving at
// most maxDepth elements on the stack: FreeStack(1) leaves at most one
// element on the stack
void IteratorStack::FreeStack(int maxDepth)
{
  int depth = iteratorStackRepr->pstack.Depth();
  while (depth-- > maxDepth)
    FreeTop();
}


void IteratorStack::InitTraversal(TraversalOrder torder, 
				  IterStackEnumType _enumType)
{
  clientTraversalOrder = torder;
  enumType = _enumType;
  if (enumType == ITER_STACK_ENUM_LEAVES_ONLY)
    traversalOrder = PostOrder;
  else if (torder == ReversePreOrder)
    traversalOrder = PreOrder;  // reversed by IteratorToPushIfAny
  else if (torder == ReversePostOrder)
    traversalOrder = PostOrder;  // reversed by IteratorToPushIfAny
//else if (torder == ReversePreAndPostOrder)
//  traversalOrder = PreAndPostOrder;  // reversed by IteratorToPushIfAny
  else {
    assert((torder == PreOrder) || (torder == PostOrder) || 
	   (torder == PreAndPostOrder));
    traversalOrder = torder;
  }
}


#if 0
void IteratorStack::DumpUpCall()
{
  dumpHandler.BeginScope();
  int depth = iteratorStackRepr->pstack.Depth();
  for (; --depth >= 0; ) {
    StackableIterator* it = 
      (StackableIterator*) iteratorStackRepr->pstack.Get(depth);
    it->Dump();
  }
  dumpHandler.EndScope();
}
#endif



//****************************************************************************
// class SingletonIterator
//****************************************************************************

CLASS_NAME_IMPL(SingletonIterator);


SingletonIterator::SingletonIterator
(const void* singletonValue, TraversalVisitType vtype) :
visitType(vtype), value(singletonValue), done(false) 
{  
}


SingletonIterator::~SingletonIterator()  
{
}


void* SingletonIterator::CurrentUpCall() const
{ 
  const void* retval = done ? 0 : value;
  return (void*) retval;  // const cast away
}


void SingletonIterator::operator++() 
{ 
  done = true; 
}


void SingletonIterator::operator++(int) 
{ 
  done = true; 
}


void SingletonIterator::Reset() 
{ 
  done = false; 
}


TraversalVisitType SingletonIterator::VisitType() const
{ 
  return visitType;
}






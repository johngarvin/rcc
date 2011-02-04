/*! \file
  
  \brief Abstract visitor for MemRefExpr's
  
  \authors Michelle Strout
  \version $Id: MemRefExprVisitor.hpp,v 1.2.6.1 2005/11/04 16:24:12 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef MemRefExprVisitor_H
#define MemRefExprVisitor_H

#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>

namespace OA {

//--------------------------------------------------------------------
/*! MemRefExprVisitor is an abstract base class used to implement the 
    Visitor pattern on MemRefExprs and possible instances of the
    RefOp decorator subclass.

    The only place in the MemRefExpr hierarchy where changes can
    occur are new SubSetRef subclasses.  Each time a new subclass
    is created a new method must be added to this visitor.  As long
    as existing concrete visitors have a default implementation for 
    the SubSetRef class they will still work properly.

    Memory Management:  It is assumed that the concrete visitors
    will NOT keep references to MemRefExpr and that
    the MemRefExpr will not be deallocated while an MemRefExprVisitor
    is visiting a MemRefExpr.
    Concrete visitors should also never take the address of a MemRefExpr
    and pass it to a method that requires an OA_ptr<MemRefExpr>.
*/
class MemRefExprVisitor {
protected:
  MemRefExprVisitor() {}
public:
  virtual ~MemRefExprVisitor() {}

  //---------------------------------------
  // methods for each MemRefExpr subclasses
  //---------------------------------------
  
  virtual void visitNamedRef(NamedRef& ref) = 0;
  virtual void visitUnnamedRef(UnnamedRef& ref) = 0;
  virtual void visitUnknownRef(UnknownRef& ref) = 0;

  // should never have a RefOp instance and will never have more
  // direct subclasses for RefOp
  //virtual void visitRefOp(RefOp& ref) = 0;

  virtual void visitAddressOf(AddressOf& ref) = 0;
  virtual void visitDeref(Deref& ref) = 0;

  // default SubSetRef base class so that visitors can handle 
  // SubSetRef sub-classes added later in a generic fasion
  // and direct instances of SubSetRef
  virtual void visitSubSetRef(SubSetRef& ref) = 0;

  // each subclass of SubSetRef should call visitSubSetRef
  // as default
  virtual void visitIdxAccess(IdxAccess& ref) 
    { visitSubSetRef(ref); }
  virtual void visitIdxExprAccess(IdxExprAccess& ref) 
    { visitSubSetRef(ref); }
  virtual void visitFieldAccess(FieldAccess& ref) 
    { visitSubSetRef(ref); }
};


} // end of OA namespace

#endif

/*! \file
  
  \brief Abstract interface that the SSA AnnotationManagers require.

  \authors Michelle Strout
  \version $Id: SSAIRInterface.hpp,v 1.1 2005/03/08 19:00:12 ntallent Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  An SSA AnnotationManager generates a SSA representation of the program.
  The analysis engine within the SSA AnnotationManager requires that
  the SSAIRInterface be implemented so that queries can be made to
  the Source IR relevant to constructing SSA.
*/

#ifndef SSAIRInterface_h
#define SSAIRInterface_h

//-----------------------------------------------------------------------------
// This file contains the abstract base classes for the IR interface.
//
// See the top level README for a description of the IRInterface and
// how to use it.
//-----------------------------------------------------------------------------

#include <iostream>
#include "IRHandles.hpp"
#include <OpenAnalysis/Utils/OA_ptr.hpp>

namespace OA {
  namespace SSA {


// Enumerate all the variable uses or variable definitions in a statement.
// This is useful for analyses that require information about variable
// references or definitions, such as SSA construction.
// 
// FIXME: Move to IRHandles; even better, merge with MemRefHandleIterator
class IRUseDefIterator {
public:
  enum { Uses, Defs };

  IRUseDefIterator() { }
  virtual ~IRUseDefIterator() { }

  virtual LeafHandle current() const = 0;   // Returns the current item.
  virtual bool isValid () = 0;        // False when all items are exhausted. 
  
  virtual void operator++() = 0;
  void operator++(int) { ++*this; }
  
  virtual void reset() = 0;
};


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

/*! 
 * The SSAIRInterface abstract base class gives a set of methods for 
 * manipulating a program.  This is the primary interface to the underlying 
 * intermediate representation.
 */
class SSAIRInterface : public virtual IRHandlesIRInterface {
 public:
  SSAIRInterface() { }
  virtual ~SSAIRInterface() { }

  // FIXME: this should probably be merged with MemRefHandleIterator
  
  
  //! Given a statement, return uses (variables referenced)
  virtual OA_ptr<IRUseDefIterator> getUses(StmtHandle h) = 0;

  //! Given a statement, return defs (variables defined)
  virtual OA_ptr<IRUseDefIterator> getDefs(StmtHandle h) = 0;

  // Getsymhandle Given a LeafHandle containing a use or def, return
  // the referened SymHandle.
  virtual SymHandle getSymHandle(LeafHandle h) = 0;
  
};


  } // end of namespace SSA
} // end of namespace OA

#endif // SSAIRInterface_h

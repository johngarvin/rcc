/*! \file
  
  \brief Abstract interface that the CallGraph AnnotationManagers require.

  \authors Michelle Strout
  \version $Id: CallGraphIRInterface.hpp,v 1.8 2004/12/06 20:39:44 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  A CallGraph AnnotationManager generates a CallGraph  representation of the
  program.  The analysis engine within the CallGraph AnnotationManager requires
  that the CallGraphIRInterface be implemented so that queries can be made to
  the Source IR relevant to constructing call graphs.  Some of the methods in
  this interface return an IRRegionStmtIterator.  this interface 
*/

#ifndef CallGraphIRInterface_h
#define CallGraphIRInterface_h

//-----------------------------------------------------------------------------
// This file contains the abstract base classes for the IR interface.
//
// See the top level README for a description of the IRInterface and
// how to use it.
//-----------------------------------------------------------------------------

#include <iostream>
#include "IRHandles.hpp"
//#include "CFGIRInterface.hpp"
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>

namespace OA {
  namespace CallGraph {


/*! 
 * The CallGraphIRInterface abstract base class gives a set of methods for 
 * manipulating a program.  This is the primary interface to the underlying 
 * intermediate representation.
 */
class CallGraphIRInterface : public virtual IRHandlesIRInterface {
 public:
  CallGraphIRInterface() { }
  //virtual ~CFGIRInterface() = 0 ;
  virtual ~CallGraphIRInterface() {} 
  
  //! Given a subprogram return an IRStmtIterator* for the entire
  //! subprogram
  //! The user must free the iterator's memory via delete.
  virtual OA_ptr<IRStmtIterator> getStmtIterator(ProcHandle h) = 0; 
  
  //! Return an iterator over all of the callsites in a given stmt
  virtual OA_ptr<IRCallsiteIterator> getCallsites(StmtHandle h) = 0;

  //! Given a ProcHandle, return its SymHandle
  virtual SymHandle getSymHandle(ProcHandle h) const = 0;

  //! Given a procedure call create a memory reference expression
  //! to describe that call.  For example, a normal call is
  //! a NamedRef.  A call involving a function ptr is a Deref.
  virtual OA_ptr<MemRefExpr> getCallMemRefExpr(CallHandle h) = 0;

};


  } // end of namespace CallGraph
} // end of namespace OA

#endif

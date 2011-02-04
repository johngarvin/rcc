/*! \file
  
  \brief Abstract IR interface for SideEffect analysis.  
  
  \authors Michelle Strout
  \version $Id: SideEffectIRInterface.hpp,v 1.7 2005/08/08 20:03:52 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef SideEffectIRInterface_h
#define SideEffectIRInterface_h

//#include <iostream>
#include <list>
//#include <string>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>

namespace OA {
  namespace SideEffect {

//! The SideEffectIRInterface abstract base class gives a set of methods
//! for querying the source IR for information relevant to SideEffect
//! Analysis.
//! This is the primary interface for SideEffect to the underlying
//! intermediate representation.
class SideEffectIRInterface : public virtual IRHandlesIRInterface
{
 public:
  SideEffectIRInterface() { }
  virtual ~SideEffectIRInterface() { }
 
  //! Given a subprogram return an IRStmtIterator for the entire
  //! subprogram
  virtual OA_ptr<IRStmtIterator> getStmtIterator(ProcHandle h) = 0; 

  //! Return a list of all the target memory reference handles that appear
  //! in the given statement.
  virtual OA_ptr<MemRefHandleIterator> getDefMemRefs(StmtHandle stmt) = 0;
  
  //! Return a list of all the source memory reference handles that appear
  //! in the given statement.
  virtual OA_ptr<MemRefHandleIterator> getUseMemRefs(StmtHandle stmt) = 0;
  
  //! Return an iterator over all of the callsites in a given stmt
  virtual OA_ptr<IRCallsiteIterator> getCallsites(StmtHandle h) = 0;

  /*! Given a MemRefHandle return an iterator over
      MemRefExprs that describe this memory reference */
  virtual OA_ptr<MemRefExprIterator> getMemRefExprIterator(MemRefHandle h) = 0;


  virtual SymHandle getSymHandle(ProcHandle h) const = 0;
};  

  } // end of namespace ReachDef
} // end of namespace OA

#endif 

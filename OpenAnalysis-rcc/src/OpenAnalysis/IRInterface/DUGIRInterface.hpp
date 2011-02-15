/*! \file
  
  \brief Abstract interface that the DUG AnnotationManagers require.

  \authors Michelle Strout
  \version $Id: DUGIRInterface.hpp,v 1.2 2005/06/10 02:32:04 mstrout Exp $

  Copyright (c) 2002-2004, Rice University <br>
  Copyright (c) 2004, University of Chicago <br>  
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>

  An DUG AnnotationManager generates an DUG  representation of the
  program.  The analysis engine within the DUG AnnotationManager requires
  that the DUGIRInterface be implemented so that queries can be made to
  the Source IR relevant to constructing call graphs.  

*/

#ifndef DUGIRInterface_h
#define DUGIRInterface_h

#include <iostream>

#include <OpenAnalysis/ExprTree/ExprTree.hpp>
#include "IRHandles.hpp"
//#include "ExprStmtPairIterator.hpp"
//#include <OpenAnalysis/IRInterface/AssignPairIterator.hpp>

namespace OA {

  namespace DUG {

/*! 
 * The DUGIRInterface abstract base class gives a set of methods for 
 * manipulating a program.  This is the primary interface to the underlying 
 * intermediate representation.
 */
class DUGIRInterface : public virtual IRHandlesIRInterface {
 public:
  DUGIRInterface() { }
  virtual ~DUGIRInterface() {} 
  
  //! Return an iterator over all independent MemRefExpr for given proc
  virtual OA_ptr<MemRefExprIterator> getIndepMemRefExprIter(ProcHandle h) = 0;

  //! Return and iterator over all dependent locations for given proc
  virtual OA_ptr<MemRefExprIterator> getDepMemRefExprIter(ProcHandle h) = 0;
  
  //! Given a subprogram return an IRStmtIterator* for the entire
  //! subprogram
  //! The user must free the iterator's memory via delete.
  virtual OA_ptr<IRStmtIterator> getStmtIterator(ProcHandle h) = 0; 

  //! Return an iterator over all of the callsites in a given stmt
  virtual OA_ptr<IRCallsiteIterator> getCallsites(StmtHandle h) = 0;

  //! Get IRCallsiteParamIterator for a callsite.
  //! Iterator visits actual parameters in called order.
  virtual OA::OA_ptr<OA::IRCallsiteParamIterator> 
    getCallsiteParams(OA::CallHandle h) = 0;

  //! Given an ExprHandle, return an ExprTree 
  virtual OA_ptr<ExprTree> getExprTree(ExprHandle h) = 0;
  
  //! Given a function call return the callee symbol handle
  virtual SymHandle getSymHandle(CallHandle expr) const = 0;
  
  //! Given the callee symbol returns the callee proc handle
  virtual ProcHandle getProcHandle(SymHandle sym) = 0;


  //! Return a list of all the def memory reference handles that appear
  //! in the given statement.
  //! User is responsible for deleting the list
  virtual OA_ptr<MemRefHandleIterator> getDefMemRefs(StmtHandle stmt) = 0;

  //! Return a list of all the source and sub memory reference handles 
  //! that appear in the given statement.
  //! User is responsible for deleting the list
  virtual OA_ptr<MemRefHandleIterator> getUseMemRefs(StmtHandle stmt) = 0;

  //! Given a MemRefHandle return an iterator over
  //! MemRefExprs that describe this memory reference
  virtual OA_ptr<MemRefExprIterator> 
      getMemRefExprIterator(MemRefHandle h) = 0;

  //! Return an iterator over all the memory reference handles that appear
  //! in the given statement.  Order that memory references are iterated
  //! over can be arbitrary.
  virtual OA_ptr<MemRefHandleIterator> getAllMemRefs(StmtHandle stmt) = 0;

  virtual OA::OA_ptr<OA::MemRefExpr> 
      convertSymToMemRefExpr(OA::SymHandle sym) = 0;

  virtual OA::OA_ptr<OA::MemRefExprIterator>
  getUseMREs(OA::StmtHandle stmt) = 0;

  virtual OA::OA_ptr<OA::MemRefExprIterator>
  getDefMREs(OA::StmtHandle stmt) = 0;

  virtual OA::SymHandle getFormalSym(OA::ProcHandle, int) = 0;
};

  } // end of namespace DUG
} // end of namespace OA

#endif

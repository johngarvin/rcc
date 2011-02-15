/*! \file
  
  \brief Abstract IR interface for UDDUChains analysis.  
  
  \authors Michelle Strout
  \version $Id: UDDUChainsIRInterface.hpp,v 1.8 2004/12/21 21:15:19 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  The source IR will be responsible for ...

  Preliminary version of this interface so that Nathan can implement
  getVisibleSymIterator.
*/

#ifndef UDDUChainsIRInterface_h
#define UDDUChainsIRInterface_h

#include <iostream>
#include <list>
#include <string>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>

namespace OA {
  namespace UDDUChains {

//! The UDDUChainsIRInterface abstract base class gives a set of methods
//! for querying the source IR for information relevant to computing
//! UD and DU chains.
//! This is the primary interface for UDDUChains to the underlying
//! intermediate representation.
class UDDUChainsIRInterface : public virtual IRHandlesIRInterface {
 public:
  UDDUChainsIRInterface() { }
  virtual ~UDDUChainsIRInterface() { }
 
  //! Given a subprogram return an IRStmtIterator* for the entire
  //! subprogram
  //! The user must free the iterator's memory via delete.
  virtual OA_ptr<IRStmtIterator> getStmtIterator(ProcHandle h) = 0; 

  //! Return a list of all the def memory reference handles that appear
  //! in the given statement.
  //! User is responsible for deleting the list
  virtual OA_ptr<MemRefHandleIterator> getDefMemRefs(StmtHandle stmt) = 0;
   
  //! Return a list of all the source and sub memory reference handles 
  //! that appear in the given statement.
  //! User is responsible for deleting the list
  virtual OA_ptr<MemRefHandleIterator> getUseMemRefs(StmtHandle stmt) = 0;
  
  //! Return an iterator over all of the callsites in a given stmt
  virtual OA_ptr<IRCallsiteIterator> getCallsites(StmtHandle h) = 0;


  virtual OA::OA_ptr<OA::MemRefExprIterator> getMemRefExprIterator(MemRefHandle) = 0;

  //! Get IRCallsiteParamIterator for a callsite.
  //! Iterator visits actual parameters in called order.
  virtual OA::OA_ptr<OA::IRCallsiteParamIterator>
    getCallsiteParams(OA::CallHandle h) = 0;


  //! Given an ExprHandle, return an ExprTree
  virtual OA_ptr<ExprTree> getExprTree(ExprHandle h) = 0;


};  

  } // end of namespace UDDUChains
} // end of namespace OA

#endif 

/*! \file
  
  \brief Abstract IR interface for ReachDef analysis.  
  
  \authors Michelle Strout
  \version $Id: ReachDefsIRInterface.hpp,v 1.10 2004/11/19 19:21:51 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  The source IR will be responsible for ...

  Preliminary version of this interface so that Nathan can implement
  getVisibleSymIterator.
*/

#ifndef ReachDefsIRInterface_h
#define ReachDefsIRInterface_h

#include <iostream>
#include <list>
#include <string>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/IRInterface/AssignPairIterator.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>

namespace OA {
  namespace ReachDefs {

//! The ReachDefIRInterface abstract base class gives a set of methods
//! for querying the source IR for information relevant to reaching definition
//! Analysis.
//! This is the primary interface for ReachDef to the underlying
//! intermediate representation.
class ReachDefsIRInterface : public virtual IRHandlesIRInterface {
 public:
  ReachDefsIRInterface() { }
  virtual ~ReachDefsIRInterface() { }
 
  //! Given a subprogram return an IRSymIterator for all
  //! symbols that are visible in the subprogram
  //virtual OA_ptr<IRSymIterator> getVisibleSymIterator(ProcHandle h) = 0; 

  //! Given a subprogram return an IRStmtIterator* for the entire
  //! subprogram
  virtual OA_ptr<IRStmtIterator> getStmtIterator(ProcHandle h) = 0; 

  //! Return a list of all the memory reference handles that appear
  //! in the given statement.
  virtual OA_ptr<MemRefHandleIterator> getAllMemRefs(StmtHandle stmt) = 0;
  
  //! Return a list of all the target memory reference handles that appear
  //! in the given statement.
  virtual OA_ptr<MemRefHandleIterator> getDefMemRefs(StmtHandle stmt) = 0;

  virtual OA_ptr<MemRefHandleIterator> getUseMemRefs(StmtHandle stmt) = 0;
  
  //! Return an iterator over all of the callsites in a given stmt
  virtual OA_ptr<IRCallsiteIterator> getCallsites(StmtHandle h) = 0;

  virtual OA_ptr<ExprTree> getExprTree(ExprHandle h) = 0;

  //! Return Def MemRefExprs
  virtual OA::OA_ptr<OA::MemRefExprIterator> getDefMREs(OA::StmtHandle stmt) = 0;

};  

  } // end of namespace ReachDef
} // end of namespace OA

#endif 

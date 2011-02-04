/*! \file
  
  \brief Abstract IR interface for differentiable location visitor, which 
         is ultimately used for activity analysis.
  
  \authors Michelle Strout
  \version $Id: ActivityIRInterface.hpp,v 1.4 2005/06/10 02:32:04 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ActivityIRInterface_h
#define ActivityIRInterface_h

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>
//#include <OpenAnalysis/IRInterface/CalleeToCallerVisitorIRInterface.hpp>
//#include "ExprStmtPairIterator.hpp"
#include <OpenAnalysis/IRInterface/AssignPairIterator.hpp>

namespace OA {

typedef IRCallsiteParamIterator ExprHandleIterator;

  namespace Activity {

//! The ActivityIRInterface abstract base class gives a 
//! set of methods for querying the source IR for information relevant 
//! to the Dep phase of activity analysis.
//class ActivityIRInterface : public virtual IRHandlesIRInterface , public virtual DataFlow::CalleeToCallerVisitorIRInterface {
class ActivityIRInterface : public virtual IRHandlesIRInterface {
 public:
  ActivityIRInterface() { }
  virtual ~ActivityIRInterface() { }

  //! Return an iterator over all independent MemRefExpr for given proc
  virtual OA_ptr<MemRefExprIterator> getIndepMemRefExprIter(ProcHandle h) = 0;
  
  //! Return and iterator over all dependent locations for given proc
  virtual OA_ptr<MemRefExprIterator> getDepMemRefExprIter(ProcHandle h) = 0;
  
  //! Given a subprogram return an IRStmtIterator for the entire
  //! subprogram
  virtual OA_ptr<IRStmtIterator> getStmtIterator(ProcHandle h) = 0; 

  //! Return an iterator over all of the callsites in a given stmt
  virtual OA_ptr<IRCallsiteIterator> getCallsites(StmtHandle h) = 0;

  //! Get IRCallsiteParamIterator for a callsite.
  //! Iterator visits actual parameters in called order.
  virtual OA::OA_ptr<OA::IRCallsiteParamIterator> 
    getCallsiteParams(OA::CallHandle h) = 0;

  //! Given an ExprHandle, return an ExprTree 
  virtual OA_ptr<ExprTree> getExprTree(ExprHandle h) = 0;
  
  //! Return a list of all the target memory reference handles that appear
  //! in the given statement.
  virtual OA_ptr<MemRefHandleIterator> getDefMemRefs(StmtHandle stmt) = 0;
  
  //! Return a list of all the source memory reference handles that appear
  //! in the given statement.
  virtual OA_ptr<MemRefHandleIterator> getUseMemRefs(StmtHandle stmt) = 0;
 
  //! Given a statement return a list to the pairs of 
  //! target MemRefHandle, ExprHandle where
  //! target = expr
  virtual OA_ptr<AssignPairIterator> 
      getAssignPairIterator(StmtHandle h) = 0; 


  virtual OA_ptr<AssignPairIterator>
      getAssignPairIterator(CallHandle h) = 0;


  //! given a symbol return the size in bytes of that symbol
  virtual int getSizeInBytes(SymHandle h) = 0;

  virtual OA::OA_ptr<OA::MemRefHandleIterator>
          getAllMemRefs(OA::StmtHandle stmt) = 0;

  virtual OA::OA_ptr<OA::MemRefExprIterator>
          getUseMREs(OA::StmtHandle stmt) = 0;

  virtual OA::OA_ptr<OA::MemRefExprIterator>
          getDefMREs(OA::StmtHandle stmt) = 0;

  virtual OA::OA_ptr<OA::MemRefExprIterator>
          getDiffUseMREs(OA::StmtHandle stmt) = 0;

  virtual OA::OA_ptr<OA::MemRefExprIterator>
          getMemRefExprIterator(OA::MemRefHandle memref) = 0;

};  

  } // end of namespace Activity
} // end of namespace OA

#endif 

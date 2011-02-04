/*! \file
  
  \brief Abstract IR interface for interprocedural analysis.  
  
  \authors Michelle Strout
  \version $Id: CallGraphDFProblemIRInterface.hpp,v 1.4 2005/03/17 21:47:46 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef CallGraphDFProblemIRInterface_h
#define CallGraphDFProblemIRInterface_h

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>

namespace OA {

typedef IRCallsiteParamIterator ExprHandleIterator;

  namespace DataFlow {


//! The CallGraphDFProblem abstract base class gives a set of methods
//! for querying the source IR for information relevant to interprocedural
//! Analysis.
//! This is the primary interface for interprocedural analysis to the 
//! underlying intermediate representation.
class CallGraphDFProblemIRInterface : public virtual IRHandlesIRInterface {
 public:
  CallGraphDFProblemIRInterface() { }
  virtual ~CallGraphDFProblemIRInterface() { }

  //! Get IRCallsiteParamIterator for a callsite.
  //! Iterator visits actual parameters in called order.
  virtual OA::OA_ptr<OA::IRCallsiteParamIterator> 
    getCallsiteParams(OA::CallHandle h) = 0;

  //! return the formal parameter that an actual parameter is associated with 
  virtual SymHandle getFormalForActual(ProcHandle caller, CallHandle call, 
                                       ProcHandle callee, ExprHandle param) = 0; 
 
  //! Given an ExprHandle, return an ExprTree 
  virtual OA_ptr<ExprTree> getExprTree(ExprHandle h) = 0;
  
};  

  } // end of namespace DataFlow
} // end of namespace OA

#endif 

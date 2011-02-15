/*! \file
  
  \brief Abstract IR interface for determining parameter bindings.
  
  \authors Michelle Strout
  \version $Id: ParamBindingsIRInterface.hpp,v 1.3 2005/08/15 19:29:55 utke Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ParamBindingsIRInterface_h
#define ParamBindingsIRInterface_h

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>

namespace OA {

typedef IRCallsiteParamIterator ExprHandleIterator;

  namespace DataFlow {


//! This is the primary interface for determining parameter bindings to
//! underlying intermediate representation.
class ParamBindingsIRInterface : public virtual IRHandlesIRInterface {
 public:
  ParamBindingsIRInterface() { }
  virtual ~ParamBindingsIRInterface() { }
  
  //! Iterator visits actual parameters in called order.
  virtual OA_ptr<IRCallsiteParamIterator> 
          getCallsiteParams(CallHandle h) = 0;

  //! return the formal parameter that an actual parameter is associated with 
  virtual SymHandle 
          getFormalForActual(ProcHandle caller, CallHandle call, 
                             ProcHandle callee, ExprHandle param) = 0; 
  
  //! Given an ExprHandle, return an ExprTree 
  virtual OA_ptr<ExprTree> 
          getExprTree(ExprHandle h) = 0;

  //! Given an SymHandle, return the SymHandle
  virtual OA::OA_ptr<OA::MemRefExpr> 
          convertSymToMemRefExpr(OA::SymHandle sym) = 0;

};  

  } // end of namespace DataFlow
} // end of namespace OA

#endif 

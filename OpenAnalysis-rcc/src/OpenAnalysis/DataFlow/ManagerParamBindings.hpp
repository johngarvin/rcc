/*! \file
  
  \brief Declarations of the AnnotationManager that determines
         parameter bindings and generates a ParamBindings object.

  \authors Michelle Strout
  \version $Id: ManagerParamBindings.hpp,v 1.1 2005/03/07 19:32:46 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ManagerParamBindings_h
#define ManagerParamBindings_h

//--------------------------------------------------------------------
// OpenAnalysis headers

#include "ParamBindings.hpp"
#include <OpenAnalysis/CallGraph/CallGraphInterface.hpp>
#include <OpenAnalysis/IRInterface/ParamBindingsIRInterface.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExprVisitor.hpp>

namespace OA {
  namespace DataFlow {


/*! 
   Generates ParamBindings
*/
class ManagerParamBindings {
public:
    
  ManagerParamBindings(OA_ptr<ParamBindingsIRInterface> _ir);
  ~ManagerParamBindings () {}

  //! Construct ParameterBinding for given program.
  OA_ptr<ParamBindings> 
      performAnalysis( OA_ptr<CallGraph::CallGraphInterface> callGraph );

private: 
  
  //! member variables
  OA_ptr<ParamBindingsIRInterface> mIR;

};

  } // end of DataFlow namespace
} // end of OA namespace

#endif

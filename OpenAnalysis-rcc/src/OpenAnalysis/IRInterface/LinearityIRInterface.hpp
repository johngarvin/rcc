/*! \file
  
  \brief Abstract IR interface for Linearity analysis.  
  
  \authors Luis Ramos
  \version $Id: LinearityIRInterface.hpp
  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  The source IR will be responsible for ...

  Preliminary version of this interface so that Nathan can implement
  getVisibleSymIterator.
*/

#ifndef LinearityIRInterface_h
#define LinearityIRInterface_h

#include <iostream>
#include <list>
#include <string>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>
//#include <OpenAnalysis/IRInterface/ExprStmtPairIterator.hpp>
#include <OpenAnalysis/IRInterface/AssignPairIterator.hpp>

namespace OA {
  namespace Linearity {


// Operations are classified into one of the following types:
enum LinOpType {
  OPR_ADDSUB,                   // Operation is either Add or Subtract
  OPR_MPYDIV,                   // Operation is either Multiply or Divide
  OPR_POWER,                    // Operation is a Power
  OPR_LINEAR,                   // Generic Linear Operation
  OPR_NONLINEAR,                // Generic Nonlinear Operation
  OPR_NONE
};

//! The ReachDefIRInterface abstract base class gives a set of methods
//! for querying the source IR for information relevant to reaching definition
//! Analysis.
//! This is the primary interface for ReachDef to the underlying
//! intermediate representation.
class LinearityIRInterface : public virtual IRHandlesIRInterface {
 public:
  LinearityIRInterface() { }
  virtual ~LinearityIRInterface() { }
 
  //! Given a OpHandle, return its Linearity::LinOpType
  virtual LinOpType getLinearityOpType(OpHandle op) = 0;

  //! Given a statement return a list to the pairs of 
  //! target MemRefHandle, ExprHandle where
  //! target = expr
  virtual OA_ptr<AssignPairIterator> 
      getAssignPairIterator(StmtHandle h) = 0; 

  //! Given an ExprHandle, return an ExprTree 
  virtual OA_ptr<ExprTree> getExprTree(ExprHandle h) = 0;

};  

  } // end of namespace Linearity
} // end of namespace OA

#endif 


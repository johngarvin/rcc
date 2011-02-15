/*! \file
  
  \brief Abstract IR interface for visitor that evaluates and
         expression tree to a ConstValBasicInterface.
  
  \authors Michelle Strout
  \version $Id: EvalToConstVisitorIRInterface.hpp,v 1.5 2005/03/17 21:47:46 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef EvalToConstVisitorIRInterface_h
#define EvalToConstVisitorIRInterface_h

//#include <list>
//#include <utility>      // for pair datatype
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/ConstValBasicInterface.hpp>
#include <OpenAnalysis/IRInterface/OpBasicInterface.hpp>

namespace OA {

//typedef std::pair<MemRefHandle,ExprHandle> ExprStmtPair;
//typedef std::list<ExprStmtPair> ExprStmtPairList;

//! The ReachConstsIRInterface abstract base class gives a set of methods
//! for querying the source IR for information relevant to constant propagation
//! Analysis.
//! This is the primary interface for ReachConsts to the underlying
//! intermediate representation.
class EvalToConstVisitorIRInterface : public virtual IRHandlesIRInterface {
 public:
  EvalToConstVisitorIRInterface() { }
  virtual ~EvalToConstVisitorIRInterface() { }

  //! Given an OpHandle and two operands (unary ops will just
  //! use the first operand and the second operand should be NULL)
  //! return a ConstValBasicInterface 
  //! User must free the ConstValBasicInterface
  virtual OA_ptr<ConstValBasicInterface> 
      evalOp( OA_ptr<OpBasicInterface> opH,
              OA_ptr<ConstValBasicInterface> operand1, 
              OA_ptr<ConstValBasicInterface> operand2) = 0;
  
  //! Given a ConstSymHandle return an abstraction representing the 
  //! constant value
  //! User must free the ConstValBasicInterface
  virtual OA_ptr<ConstValBasicInterface> 
      getConstValBasic(ConstSymHandle c) = 0;

  //! Given a ConstValHandle return an abstraction representing the 
  //! constant value
  //! User must free the ConstValBasicInterface
  virtual OA_ptr<ConstValBasicInterface> 
      getConstValBasic(ConstValHandle c) = 0;

  //! Temporary routine for testing MPICFG
  // given a ConstValBasicInterface, print out value if any
  virtual std::string toString(OA_ptr<ConstValBasicInterface> cvPtr) = 0;
  // need all for full override
  virtual std::string toString(const ProcHandle h) const = 0;
  virtual std::string toString(const StmtHandle h) const = 0;
  virtual std::string toString(const ExprHandle h) const = 0;
  virtual std::string toString(const OpHandle h) const = 0;
  virtual std::string toString(const MemRefHandle h) const = 0;
  virtual std::string toString(const SymHandle h) const = 0;
  virtual std::string toString(const ConstSymHandle h) const = 0;
  virtual std::string toString(const ConstValHandle h) const = 0;

  //! Temporary routine for testing ReachConsts
  // Given an unsigned int, return a ConstValBAsicInterface for it
  virtual OA_ptr<ConstValBasicInterface> 
      getConstValBasic (unsigned int val) = 0;

  //! temporary routine to find things out
  // should be removed after testing
  virtual int returnOpEnumValInt(OA::OpHandle op) = 0;

};

} // end of namespace OA

#endif 

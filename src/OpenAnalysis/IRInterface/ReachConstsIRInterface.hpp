/*! \file
  
  \brief Abstract IR interface for Reaching Constants analysis.  
  
  \authors Michelle Strout, Barbara Kreaseck
  \version $Id: ReachConstsIRInterface.hpp,v 1.7 2005/03/17 21:47:46 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  The source IR will be responsible for ...

  Preliminary version of this interface so that Nathan can implement
  some of the stuff we need.
*/

#ifndef ReachConstsIRInterface_h
#define ReachConstsIRInterface_h

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>
#include <OpenAnalysis/IRInterface/ConstValBasicInterface.hpp>
#include <OpenAnalysis/IRInterface/EvalToConstVisitorIRInterface.hpp>
//#include <OpenAnalysis/IRInterface/ExprStmtPairIterator.hpp>
#include <OpenAnalysis/IRInterface/AssignPairIterator.hpp>
#include <OpenAnalysis/IRInterface/OpBasicInterface.hpp>

namespace OA {
  namespace ReachConsts {

//! The ReachConstsIRInterface abstract base class gives a set of methods
//! for querying the source IR for information relevant to constant propagation
//! Analysis.
//! This is the primary interface for ReachConsts to the underlying
//! intermediate representation.
class ReachConstsIRInterface : public virtual IRHandlesIRInterface,
                    public virtual EvalToConstVisitorIRInterface
{
 public:
  ReachConstsIRInterface() { }
  virtual ~ReachConstsIRInterface() { }

  //! Given a subprogram return an IRStmtIterator* for the entire
  //! subprogram
  virtual OA_ptr<IRStmtIterator> getStmtIterator(ProcHandle h) = 0; 

  //! Given a statement handle, get a string expressing its value
  //virtual std::string toString(const StmtHandle h) = 0;

  //! Given a memory reference handle, get a succinct string representation
  //virtual std::string toString(const MemRefHandle h) = 0;

  //! Return a list of all the memory reference handles that appear
  //! in the given statement.
  virtual OA_ptr<MemRefHandleIterator> getAllMemRefs(StmtHandle stmt) = 0;
  
  //! Return a list of all the target memory reference handles that appear
  //! in the given statement.
  virtual OA_ptr<MemRefHandleIterator> getDefMemRefs(StmtHandle stmt) = 0;

  //! Return a list of all the use memory reference handles that appear
  //! in the given statement
  virtual OA_ptr<MemRefHandleIterator> getUseMemRefs(StmtHandle stmt) = 0;
  
  //! Given a statement return a list to the pairs of 
  //! target MemRefHandle, ExprHandle where
  //! target = expr
  virtual OA_ptr<AssignPairIterator> 
      getAssignPairIterator(StmtHandle h) = 0; 
  
  //! Given an OpHandle and two operands (unary ops will just
  //! use the first operand and the second operand should be NULL)
  //! return a ConstValBasicInterface 
  virtual OA_ptr<ConstValBasicInterface> 
      evalOp(OA_ptr<OpBasicInterface> opH, 
                    OA_ptr<ConstValBasicInterface> operand1, 
                    OA_ptr<ConstValBasicInterface> operand2) = 0;
  
  //! Given a ConstSymHandle return an abstraction representing the 
  //! constant value
  virtual OA_ptr<ConstValBasicInterface> getConstValBasic(ConstSymHandle c) = 0;

  //! Given a ConstValHandle return an abstraction representing the 
  //! constant value
  virtual OA_ptr<ConstValBasicInterface> getConstValBasic(ConstValHandle c) = 0;

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
  virtual OA_ptr<ConstValBasicInterface> getConstValBasic (unsigned int val) = 0;

  //! Given an ExprHandle, return an ExprTree 
  virtual OA_ptr<ExprTree> getExprTree(ExprHandle h) = 0;

  //! Return an iterator over all of the callsites in a given stmt
  virtual OA_ptr<IRCallsiteIterator> getCallsites(StmtHandle h) = 0;

  //! temporary routine to find things out
  // should be removed after testing
  virtual int returnOpEnumValInt(OA::OpHandle op) = 0;

  virtual OA_ptr<MemRefExprIterator> 
          getMemRefExprIterator(MemRefHandle memref) = 0;
 

  virtual OA::OA_ptr<OA::MemRefExprIterator>
          getDefMREs(OA::StmtHandle stmt) = 0;



  virtual OA_ptr<AssignPairIterator>
       getAssignPairIterator(CallHandle h) = 0;

 
};

  } // end of namespace ReachConsts
} // end of namespace OA

#endif 

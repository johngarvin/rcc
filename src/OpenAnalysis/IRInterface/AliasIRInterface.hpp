/*! \file
  
  \brief Abstract IR interface for Alias analysis.  
  
  \authors Michelle Strout, Brian White
  \version $Id: AliasIRInterface.hpp,v 1.15.6.6 2005/11/23 05:01:04 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  The source IR will be responsible for iterating over all the statements,
  iterating over top memory references for a stmt, and categorizing statements
  specifically for alias analysis.
*/

#ifndef AliasIRInterface_h
#define AliasIRInterface_h

#include <iostream>
#include <string>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
//#include <OpenAnalysis/Location/Locations.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>

namespace OA {

  //! Namespace for all Alias Analysis interfaces and implementations.
  namespace Alias {

class PtrAssignPairStmtIterator {
  public:
    PtrAssignPairStmtIterator() {}
    virtual ~PtrAssignPairStmtIterator() {}

    //! right hand side
    virtual OA_ptr<MemRefExpr> currentSource() const = 0;
    //! left hand side
    virtual OA_ptr<MemRefExpr> currentTarget() const = 0;

    virtual bool isValid() const = 0;
                    
    virtual void operator++() = 0;
    void operator++(int) { ++*this; }
};

class ParamBindPtrAssignIterator {
  public:
    ParamBindPtrAssignIterator() {}
    virtual ~ParamBindPtrAssignIterator() {}

    //! right hand side
    virtual OA_ptr<MemRefExpr> currentActual() const = 0;
    //! left hand side
    virtual int currentFormalId() const = 0;

    virtual bool isValid() const = 0;
                    
    virtual void operator++() = 0;
    void operator++(int) { ++*this; }
};

//! The AliasIRInterface abstract base class gives a set of methods
//! for querying the source IR for information relevant to alias analysis.
//! This is the primary interface for Alias to the underlying
//! intermediate representation.
class AliasIRInterface : public virtual IRHandlesIRInterface {
 public:
  AliasIRInterface() { }
  virtual ~AliasIRInterface() { }
 
  //! Given a subprogram return an IRStmtIterator for the entire
  //! subprogram
  virtual OA_ptr<IRStmtIterator> getStmtIterator(ProcHandle h) = 0; 
  
  //! Return an iterator over all the memory reference handles that appear
  //! in the given statement.  Order that memory references are iterated
  //! over can be arbitrary.
  virtual OA_ptr<MemRefHandleIterator> getAllMemRefs(StmtHandle stmt) = 0;
  
  //! If this is a PTR_ASSIGN_STMT then return an iterator over MemRefHandle
  //! pairs where there is a source and target such that target
  virtual OA_ptr<PtrAssignPairStmtIterator>
      getPtrAssignStmtPairIterator(StmtHandle stmt) = 0;

  //! Return an iterator over <int, MemRefExpr> pairs
  //! where the integer represents which formal parameter 
  //! and the MemRefExpr describes the corresponding actual argument. 
  virtual OA_ptr<ParamBindPtrAssignIterator>
      getParamBindPtrAssignIterator(CallHandle call) = 0;

  //! Return the symbol handle for the nth formal parameter to proc
  //! Number starts at 0 and implicit parameters should be given
  //! a number in the order as well.  This number should correspond
  //! to the number provided in getParamBindPtrAssign pairs
  //! Should return SymHandle(0) if there is no formal parameter for 
  //! given num
  virtual SymHandle getFormalSym(ProcHandle,int) = 0;

  //! Return an iterator over all of the callsites in a given stmt
  virtual OA_ptr<IRCallsiteIterator> getCallsites(StmtHandle h) = 0;

  //! Given a procedure call create a memory reference expression
  //! to describe that call.  For example, a normal call is
  //! a NamedRef.  A call involving a function ptr is a Deref.
  virtual OA_ptr<MemRefExpr> getCallMemRefExpr(CallHandle h) = 0;

  //! Given the callee symbol returns the callee proc handle
  virtual ProcHandle getProcHandle(SymHandle sym) = 0;

  //! Given a procedure return associated SymHandle
  virtual SymHandle getSymHandle(ProcHandle h) const = 0;
  
  //! Given a subprogram return an IRSymIterator for all
  //! symbols that are visible in the subprogram
  //virtual OA_ptr<IRSymIterator> getVisibleSymIterator(ProcHandle h) = 0; 

  //! Given a subprogram return an IRStmtIterator for all
  //! statements that involve an unnamed dynamic memory allocation
  //! in the subprogram
  //virtual OA_ptr<IRStmtIterator> getUnnamedDynAllocStmtIterator(ProcHandle h) = 0; 

  //! For the given symbol create a Location that indicates statically
  //! overlapping locations and information about whether the location
  //! is local or not for the given procedure, local means only visible
  //! in this procedure
  //MMS deprecating, 1/3/08
  //virtual OA_ptr<Location> getLocation(ProcHandle p, SymHandle s) = 0;

  //! Given a MemRefHandle return an iterator over
  //! MemRefExprs that describe this memory reference
  virtual OA_ptr<MemRefExprIterator> 
      getMemRefExprIterator(MemRefHandle h) = 0;
};  

  } // end of namespace Alias
} // end of namespace OA

#endif 

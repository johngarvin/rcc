/*! \file
  
  \brief Abstract interface that the CallGraph AnnotationManagers require.

  \authors Michelle Strout, Barbara Kreaseck
  \version $Id: MPICFGIRInterface.hpp,v 1.19 2005/03/17 21:47:46 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  An MPICFG AnnotationManager generates an MPI-aware CFG representation of the
  program.  The analysis engine within the MPICFG AnnotationManager requires
  that the MPICFGIRInterface be implemented so that queries can be made to
  the Source IR relevant when constructing MPICFGs.  Some of the methods in
  this interface return ... ? 
*/

#ifndef MPICFGIRInterface_h
#define MPICFGIRInterface_h

//-----------------------------------------------------------------------------
// This file contains the abstract base classes for the IR interface.
//
// See the top level README for a description of the IRInterface and
// how to use it.
//-----------------------------------------------------------------------------

#include <iostream>
#include <map>
#include <list>

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/IRInterface/ConstValBasicInterface.hpp>
#include <OpenAnalysis/IRInterface/ConstValIntInterface.hpp>
#include <OpenAnalysis/IRInterface/EvalToConstVisitorIRInterface.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>

// FIXME: this needs to go away at some point
#include "CFGIRInterface.hpp"

namespace OA {
  namespace MPICFG {

//! procedure call names are classified into one of the following types 
//! for MPICFG (i.e., which MPI call type (non-MPI calls are NONE)) :
//
//  The Open64IRInterface in UseNewOA has a mapping of strings to IRProcType
//  so if we add more recognizable types here, we should add them there, too
//  Also, there is a mapping of IRProcType and IRParamType to param position
//  (int) there also.  Update that structure when a new IRProcType is added.
enum IRProcType {
  MPI_NONE,                   // Any non-MPI call
  MPI_OTHER,                  // Any MPI call not covered below
  MPI_INIT,                   // MPI_Init - called before any other MPI call
  MPI_COMM_RANK,              // MPI_Comm_Rank - get own process id in group 
  MPI_COMM_SIZE,              // MPI_Comm_Size - # processors in group
  MPI_COMM_DUP,               // MPI_Comm_Dup - duplicates oldcomm to comm 
  MPI_COMM_SPLIT,             // MPI_Comm_Split - partition comm to subcomm
  MPI_SEND,                   // MPI_Send
  MPI_RECV,                   // MPI_Recv
  MPI_BCAST,                  // MPI_Bcast
  MPI_REDUCE,                 // MPI_Reduce
  MPI_FINALIZE                // MPI_Finalize
};

//! parameter types used within the IRProcType's above.
//  The Open64IRInterface in UseNewOA has a mapping of IRProcType and
//  IRParamType to parameter position.  So when a new IRProcType is added
//  that has a parameter type not in IRParamType, update here.
enum IRParamType {
  BUF,
  COLOR,
  COMM,
  COUNT,
  DATATYPE,
  DEST,
  GROUP,
  IERROR,
  KEY,
  NEWCOMM,
  OP,
  RANK,
  RECVBUF,
  ROOT,
  SENDBUF,
  SIZE,
  SOURCE,
  STATUS,
  TAG
};

//    typedef pair<IRProcType,IRParamType> ProcParamPair;

// return type for examining a parameter expression tree
// first() is true if second() has a useful const value from the parameter
typedef std::pair<bool,OA_ptr<ConstValBasicInterface> > MaybeConstValue;

//------------------------------------
// for use in MPICFGIdRankVar

enum IRRankVarStmtType { 
  RANK_NO_STMT,    // not a statement, error situation
  RANK_ANY_STMT,   // Any statement not covered below
  RANK_DEF_STMT    // A statement with a call to MPI_COMM_RANK
};

enum IRCopyStmtType {
  COPY_NO_STMT,    // not a statement, error situation
  COPY_ANY_STMT,   // Any statement not covered below
  COPY_STMT        // A statement involving only (target = use) pairs
};

//typedef std::pair<MemRefHandle,MemRefHandle> CopyStmtPair;
class CopyStmtPairIterator {
  public:
    CopyStmtPairIterator() {}
    virtual ~CopyStmtPairIterator() {}

    virtual MemRefHandle currentSource() const = 0;
    virtual MemRefHandle currentTarget() const = 0;

    virtual bool isValid() const = 0;
                    
    virtual void operator++() = 0;
    void operator++(int) { ++*this; }
};

//typedef std::list<CopyStmtPair> CopyStmtPairList;

//-----------------------------
// for use in MPICFGIdRankVar and MPICFGUpdateLabels

enum LatticeType {TOP, VALUE, BOTTOM};

//-----------------------------
// for use in MPICFGUpdateLabels
//----------------
// We want to eventually get here
enum RelOp {GREATER,LESSER,EQUAL,GREATER_EQUAL,LESSER_EQUAL,NOT_EQUAL};
//enum BoolOp {AND, OR, NOT};
//enum RelOp {EQUAL, NOT_EQUAL};
enum BoolOp {AND, OR};
// RankExprTree has toString functions for RelOp, BoolOp, and LatticeType



/*! 
 * The MPICFGIRInterface abstract base class gives a set of methods for 
 * manipulating a program.  This is the primary interface to the underlying 
 * intermediate representation.
 */
class MPICFGIRInterface : public virtual IRHandlesIRInterface,
                      public virtual EvalToConstVisitorIRInterface 
{
 public:
  MPICFGIRInterface() { }
  virtual ~MPICFGIRInterface() { } 
  
  //--------------------------------------------------------
  // Iterators
  //--------------------------------------------------------

  //! Given a subprogram return an IRStmtIterator for the entire
  //! subprogram
  virtual OA_ptr<IRStmtIterator> getStmtIterator(ProcHandle h) = 0; 

  //! Return an iterator over all of the callsites in a given stmt
  virtual OA_ptr<IRCallsiteIterator> getCallsites(StmtHandle h) = 0;

  // Get IRCallsiteParamIterator for a callsite. 
  virtual OA_ptr<IRCallsiteParamIterator> getCallsiteParams(ExprHandle h) = 0;
 
  //--------------------------------------------------------
  // Symbol Handles
  //--------------------------------------------------------

  //! Return a symbol handle for a given expression handle
  virtual SymHandle getSymHandle(ExprHandle expr) = 0;
  
  //! Return a symbol handle for a given procedure handle
  virtual SymHandle getProcSymHandle(ProcHandle h) = 0;
 
  //--------------------------------------------------------
  // Utilities
  //--------------------------------------------------------

  //! Return a string version of the symbol handle
  virtual std::string toString(const SymHandle h) = 0;

  //! Return a string version of an expression handle
  virtual std::string toString(const ExprHandle h) = 0;

  //! Given an ExprHandle, return an ExprTree 
  virtual OA_ptr<ExprTree> getExprTree(ExprHandle h) = 0;

  //--------------------------------------------------------
  // MPI-specific functionalities
  //--------------------------------------------------------

  //! Return an IRProcType (an MPI call procedure designator)
  //! for a given symbol handle
  virtual IRProcType getMPICFGProcType(SymHandle h) = 0; 

  //! Return a boolean answer to the question of whether the
  //! given constant value handle corresponds to the MPI wild
  //! card MPI_ANY_TAG
  virtual bool isMPICFGAnyTag(ConstValHandle h) = 0;

  //! Return a boolean answer to the question of whether the
  //! given constant symbol handle corresponds to the MPI wild
  //! card MPI_ANY_TAG
  virtual bool isMPICFGAnyTag(ConstSymHandle h) = 0;

  //! Return a boolean answer to the question of whether the
  //! given ConstValBasicInterface* corresponds to the MPI wild
  //! card MPI_ANY_TAG
  virtual bool isMPICFGAnyTag(OA_ptr<ConstValBasicInterface> c) = 0;

  //! Return a boolean answer to the question of whether the
  //! given constant value handle corresponds to the MPI wild
  //! card MPI_ANY_SOURCE
  virtual bool isMPICFGAnySource(ConstValHandle h) = 0;

  //! Return a boolean answer to the question of whether the
  //! given constant symbol handle corresponds to the MPI wild
  //! card MPI_ANY_SOURCE
  virtual bool isMPICFGAnySource(ConstSymHandle h) = 0;

  //! Return a boolean answer to the question of whether the
  //! given ConstValBasicInterface* corresponds to the MPI wild
  //! card MPI_ANY_SOURCE
  virtual bool isMPICFGAnySource(OA_ptr<ConstValBasicInterface> c) = 0;

  // The following routines return expression handles to the 
  // requested parameter of the MPI call of the given callsite
  // represented by an expression handle.  The algorithms
  // behind these routines will be dependent upon the MPI interface
  // in the source language of the application program.  Currently,
  // only implemented for Fortran90.

  //! Return an expression handle for the 'communicator' parameter in the
  // the MPI call within the given expression handle callsite
  virtual ExprHandle getMPICFGCommParam(OA::ExprHandle h) = 0;

  //! Return an expression handle for the 'data type' parameter in the
  // the MPI call within the given expression handle callsite
  virtual ExprHandle getMPICFGDataTypeParam(OA::ExprHandle h) = 0;

  //! Return an expression handle for the 'dest' parameter in the
  // the MPI call within the given expression handle callsite
  virtual ExprHandle getMPICFGDestParam(OA::ExprHandle h) = 0;

  //! Return an expression handle for the 'group' parameter in the
  // the MPI call within the given expression handle callsite
  virtual ExprHandle getMPICFGGroupParam(OA::ExprHandle h) = 0;

  //! Return an expression handle for the 'op' parameter in the
  // the MPI call within the given expression handle callsite
  virtual ExprHandle getMPICFGOpParam(OA::ExprHandle h) = 0;

  //! Return an expression handle for the 'rank' parameter in the
  // the MPI call within the given expression handle callsite
  virtual ExprHandle getMPICFGRankParam(OA::ExprHandle h) = 0;

  //! Return an expression handle for the 'root' parameter in the
  // the MPI call within the given expression handle callsite
  virtual ExprHandle getMPICFGRootParam(OA::ExprHandle h) = 0;

  //! Return an expression handle for the 'size' parameter in the
  // the MPI call within the given expression handle callsite
  virtual ExprHandle getMPICFGSizeParam(OA::ExprHandle h) = 0;

  //! Return an expression handle for the 'source' parameter in the
  // the MPI call within the given expression handle callsite
  virtual ExprHandle getMPICFGSourceParam(OA::ExprHandle h) = 0;

  //! Return an expression handle for the 'tag' parameter in the
  // the MPI call within the given expression handle callsite
  virtual ExprHandle getMPICFGTagParam(OA::ExprHandle h) = 0;

  //--------------------------------------------------------
  // DEBUGGING
  //--------------------------------------------------------

  // !Given an expression handle to a parameter, 
  // pretty-print it to the output stream os.
  virtual void dumpMPICFGparam(ExprHandle h, std::ostream& os) = 0;

  //! temporary routine to find things out
  // should be removed after testing
  virtual int returnOpEnumValInt(OA::OpHandle op) = 0;

  //! Given a statement, pretty-print it to the output stream os.
  //! The default implementation does nothing.
  virtual void dump(StmtHandle stmt, std::ostream& os) 
    { os << "Override CFGIRInterfaceDefault::dump(StmtHandle,os)"; }

  //! Temporary routine for testing MPICFG
  // given a ConstValBasicInterface, print out value if any
  virtual std::string toString(OA_ptr<OA::ConstValBasicInterface> cvPtr) = 0;

  //--------------------------------------------------------
  // Following are extra for ManagerMPICFGIdRankVars
  //--------------------------------------------------------

  //! RankVarStmtType: interested in RANK_DEF_STMTs (define the MPI rank var)
  // But defined above class MPICFGIRInterface
  //enum IRRankVarStmtType { RANK_NO_STMT, RANK_ANY_STMT, RANK_DEF_STMT };

  //! Return an IRRankVarStmtType for the given stmt
  virtual IRRankVarStmtType getMPICFGRankVarStmtType(OA::StmtHandle h) = 0;

  //! Return a memory reference handle for the 'rank' parameter in the
  //! MPI call within the given stmt.  Should only be called with a
  //! MPICFG::RANK_DEF_STMT statement.
  virtual MemRefHandle getMPICFGRankDefMemRef(OA::StmtHandle h) = 0;
  
  //! Return a stmt handle for the given memory reference handle
  virtual StmtHandle getStmtFromMemRef(OA::MemRefHandle h) = 0;
  
  //! Return a list of all the memory reference handles that appear
  //! in the given statement.
  //! User is responsible for deleting the list
  virtual OA_ptr<MemRefHandleIterator> getAllMemRefs(StmtHandle stmt) = 0;
  
  //! Return a list of all the target memory reference handles that appear
  //! in the given statement.
  //! User is responsible for deleting the list
  virtual OA_ptr<MemRefHandleIterator> getDefMemRefs(StmtHandle stmt) = 0;

  //! Return a list of all the use memory reference handles that appear
  //! in the given statement
  //! User is responsible for deleting the list
  virtual OA_ptr<MemRefHandleIterator> getUseMemRefs(StmtHandle stmt) = 0;
  
  //! Given a statement handle, get a string expressing its value
  virtual std::string toString(const StmtHandle h) = 0;

  //! Given a memory reference handle, get a succinct string representation
  virtual std::string toString(const MemRefHandle h) = 0;

  //! Given a ConstSymHandle return an abstraction representing the 
  //! constant value
  virtual OA_ptr<ConstValBasicInterface> getConstValBasic(ConstSymHandle c) = 0;
  
  //! Given a ConstValHandle return an abstraction representing the 
  //! constant value
  virtual OA_ptr<ConstValBasicInterface> getConstValBasic(ConstValHandle c) = 0;

  //! Given a statement, return its MPICFG::IRCopyStmtType
  virtual IRCopyStmtType getCopyStmtType(OA::StmtHandle h) = 0;; 
  
  //! Given a statement return a list to the pairs of 
  //! target MemRefHandle, use MemRefHandle where
  //! target = source
  virtual OA_ptr<CopyStmtPairIterator> 
      getCopyStmtPairIterator(OA::StmtHandle h) = 0;

  //--------------------------------------------------------
  // Following are extra for ManagerMPICFGUpdateLabels
  //--------------------------------------------------------

  //! two routines to help convert from Open64 operator to a OA RelOp
  //! isaRelOp will work with any OpHandle, getRelOp only works with
  //! a OpHandle that will convert to a RelOp.
  virtual bool isaRelOp(OA::OpHandle h) = 0;
  virtual OA::MPICFG::RelOp getRelOp(OA::OpHandle h) = 0;

  //! Convert a ConstValBasicInterface into a ConstValIntInterface if
  //! the constant is an integer, NULL otherwise
  virtual OA_ptr<ConstValIntInterface> 
      getConstValInt(OA_ptr<ConstValBasicInterface> c) =0;

  //! get condition expression handles from various stmt types
  virtual ExprHandle getLoopCondition(OA::StmtHandle h) =0;
  virtual ExprHandle getCondition(OA::StmtHandle h) =0;
  virtual ExprHandle getSMultiCondition (OA::StmtHandle h, int bodyIndex) =0;
  virtual ExprHandle getSMultiTest(OA::StmtHandle h) =0;
  virtual ExprHandle getUMultiCondition(OA::StmtHandle h, int targetIndex) =0;
  virtual ExprHandle getUMultiTest(OA::StmtHandle h) =0;

  // get number of cases or targets for use in get*MultiCondition above
  virtual int numMultiCases(OA::StmtHandle h)=0;
  virtual int numUMultiTargets(OA::StmtHandle h)=0;

  //! Given a statement, return its CFG::IRStmtType
  virtual CFG::IRStmtType getCFGStmtType(StmtHandle h) = 0; 

  //! Hack! convert from the expression handle on switch case edge
  //! to the constant it contains, otherwise, NULL.
  virtual OA_ptr<OA::ConstValBasicInterface>
    getConstFromSwitchCase(OA::ExprHandle h) = 0;

  //--------------------------------------------------------
  // Following are extra for ManagerInterMPICFGUpBound
  //--------------------------------------------------------

  //! Return an iterator over all of the procedures in a proc_forest
  //  virtual OA_ptr<IRProcIterator> getProcIterator(PU_Info* puForest) = 0;
  // wish the above existed!  Am passing the iterator directly into 
  // the performAnalysis routine from the test program

};


  } // end of namespace MPICFG
} // end of namespace OA

#endif // MPICFGIRInterface_h

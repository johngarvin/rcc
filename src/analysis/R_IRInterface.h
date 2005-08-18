// -*-C++-*-
// * BeginRiceCopyright *****************************************************
// 
// Copyright ((c)) 2004, Rice University 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// 
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// 
// * Neither the name of Rice University (RICE) nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
// 
// This software is provided by RICE and contributors "as is" and any
// express or implied warranties, including, but not limited to, the
// implied warranties of merchantability and fitness for a particular
// purpose are disclaimed. In no event shall RICE or contributors be
// liable for any direct, indirect, incidental, special, exemplary, or
// consequential damages (including, but not limited to, procurement of
// substitute goods or services; loss of use, data, or profits; or
// business interruption) however caused and on any theory of liability,
// whether in contract, strict liability, or tort (including negligence
// or otherwise) arising in any way out of the use of this software, even
// if advised of the possibility of such damage. 
// 
// ******************************************************* EndRiceCopyright *
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef R_IRInterface_h
#define R_IRInterface_h

//-----------------------------------------------------------------------------
// Derived class for the IR interface to the R AST.
//
//-----------------------------------------------------------------------------

#include <list>
#include <string>
#include <ostream>
#include <assert.h>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/IRInterface/CFGIRInterfaceDefault.hpp>
#include <OpenAnalysis/IRInterface/SSAIRInterface.hpp>
#include <OpenAnalysis/CFG/ManagerCFGStandard.hpp>
#include <util.h>
#include "SimpleIterators.hpp"
#include "R_Analyst.h"
#include "R_Utils.hpp"
#include <rinternals.h>

//-----------------------------------------------------------------------------
// R IR representation

//! OpenAnalysis interface to the R AST
class R_IRInterface : public OA::CFG::CFGIRInterfaceDefault,
		      public OA::SSA::SSAIRInterface
{
public:
  R_IRInterface() {}
  virtual ~R_IRInterface() {}
  
  //--------------------------------------------------------
  // Procedures and call sites
  //--------------------------------------------------------

  //! Given a procedure, return its IRProcType.
  //  CFG::IRProcType GetProcType(OA::ProcHandle h);
  
  //! Given a ProcHandle, return an IRRegionStmtIterator for the
  //! procedure.
  OA::OA_ptr<OA::IRRegionStmtIterator> procBody(OA::ProcHandle h);
  
  //! Get IRCallsiteIterator* for a statement. The user must free the
  //! iterator's memory via delete.
  //IRCallsiteIterator *GetCallsites(OA::StmtHandle h);

  //! Get IRCallsiteParamIterator* for a statement. The user must free the
  //! iterator's memory via delete.
  //IRCallsiteParamIterator *GetCallsiteParams(OA::ExprHandle h);

  //bool IsParamProcRef(OA::ExprHandle h);

  //! Given an expression representing a callsite, is this an
  //! invocation through a procedure parameter?
  //bool IsCallThruProcParam(OA::ExprHandle h);


  //--------------------------------------------------------
  // Statements: General
  //--------------------------------------------------------

  //! Are return statements allowed?
  bool returnStatementsAllowed() { return true; }

  //! Given a statement, return its CFG::IRStmtType.
  OA::CFG::IRStmtType getCFGStmtType(OA::StmtHandle h);

  //! Given a statement, return a label (or NULL if
  //! there is no label associated with the statement).
  OA::StmtLabel getLabel(OA::StmtHandle h);

  //! Given a compound statement, return an IRRegionStmtIterator for
  //! the statements.  A compound is a list of statements.
  OA::OA_ptr<OA::IRRegionStmtIterator> getFirstInCompound(OA::StmtHandle h);
  
  //--------------------------------------------------------
  // Loops
  //--------------------------------------------------------

  //! Given a loop statement, return an IRRegionStmtIterator pointer
  //! for the loop body.
  OA::OA_ptr<OA::IRRegionStmtIterator> loopBody(OA::StmtHandle h);

  //! Given a loop statement, return the loop header statement.  This 
  //! would be the initialization statement in a C 'for' loop, for example.
  OA::StmtHandle loopHeader(OA::StmtHandle h);

  //! Given a loop statement, return the increment statement. 
  OA::StmtHandle getLoopIncrement(OA::StmtHandle h);

  //! Given a loop statement, return:
  //! 
  //! True: If the number of loop iterations is defined
  //! at loop entry (i.e. Fortran semantics).  This causes the CFG builder 
  //! to add the loop statement representative to the header node so that
  //! definitions from inside the loop don't reach the condition and increment
  //! specifications in the loop statement.
  //!
  //! False: If the number of iterations is not defined at
  //! entry (i.e. C semantics), we add the loop statement to a node that
  //! is inside the loop in the CFG so definitions inside the loop will 
  //! reach uses in the conditional test. For C style semantics, the 
  //! increment itself may be a separate statement. if so, it will appear
  //! explicitly at the bottom of the loop. 
  bool loopIterationsDefinedAtEntry(OA::StmtHandle h);

  //--------------------------------------------------------
  // Invariant: a two-way conditional or a multi-way conditional MUST provide
  // provide either a target, or a target label
  //--------------------------------------------------------

  //--------------------------------------------------------
  // Structured two-way conditionals
  //
  // Note: An important pre-condition for structured conditionals is
  // that chains of else-ifs must be represented as nested elses.  For
  // example, this Matlab statement:
  //   if (c1)
  //     s1;
  //   elseif (c2)
  //     s2;
  //   else
  //     s3;
  //   end;
  //
  // would need be represented by the underlying IR as:
  //   if (c1)
  //     s1;
  //   else
  //     if (c2)
  //       s2;
  //     else
  //       s3;
  //     end;
  //   end; 
  //--------------------------------------------------------

  //! Given a structured two-way conditional statement, return an
  //! IRRegionStmtIterator for the "true" part (i.e., the statements
  //! under the "if" clause).
  OA::OA_ptr<OA::IRRegionStmtIterator> trueBody(OA::StmtHandle h);

  //! Given a structured two-way conditional statement, return an
  //! IRRegionStmtIterator for the "else" part (i.e., the statements
  //! under the "else" clause).
  OA::OA_ptr<OA::IRRegionStmtIterator> elseBody(OA::StmtHandle h);

  //--------------------------------------------------------
  // Structured multiway conditionals
  //--------------------------------------------------------

  //! Given a structured multi-way branch, return the number of cases.
  //! The count does not include the default/catchall case.
  int numMultiCases(OA::StmtHandle h);

  //! Given a structured multi-way branch, return an IRRegionStmtIterator* for
  //! the body corresponding to target 'bodyIndex'. The n targets are 
  //! indexed [0..n-1].
  OA::OA_ptr<OA::IRRegionStmtIterator> multiBody(OA::StmtHandle h, int bodyIndex);

  //! Given a structured multi-way branch, return true if the cases have
  //! implied break semantics.  For example, this method would return false
  //! for C since one case will fall-through to the next if there is no
  //! explicit break statement.  Matlab, on the other hand, implicitly exits
  //! the switch statement once a particular case has executed, so this
  //! method would return true.
  bool isBreakImplied(OA::StmtHandle multicond);

  //! Given a structured multi-way branch, return true if the body 
  //! corresponding to target 'bodyIndex' is the default/catchall/ case.
  bool isCatchAll(OA::StmtHandle h, int bodyIndex);

  //! Given a structured multi-way branch, return an IRRegionStmtIterator*
  //! for the body corresponding to default/catchall case.
  OA::OA_ptr<OA::IRRegionStmtIterator> getMultiCatchall (OA::StmtHandle h);

  //!! Given a structured multi-way branch, return the condition
  //! expression corresponding to target 'bodyIndex'. The n targets are
  //! indexed [0..n-1].
  OA::ExprHandle getSMultiCondition(OA::StmtHandle h, int bodyIndex);

  //--------------------------------------------------------
  // Unstructured two-way conditionals: 
  //--------------------------------------------------------

  //! Given an unstructured two-way branch, return the label of the
  //! target statement.  The second parameter is currently unused.
  OA::StmtLabel getTargetLabel(OA::StmtHandle h, int n);

  //--------------------------------------------------------
  // Unstructured multi-way conditionals
  // FIXME: Review all of the multi-way stuff.
  //--------------------------------------------------------

  //! Given an unstructured multi-way branch, return the number of targets.
  //! The count does not include the optional default/catchall case.
  int numUMultiTargets(OA::StmtHandle h);

  //! Given an unstructured multi-way branch, return the label of the target
  //! statement at 'targetIndex'. The n targets are indexed [0..n-1]. 
  OA::StmtLabel getUMultiTargetLabel(OA::StmtHandle h, int targetIndex);

  //! Given an unstructured multi-way branch, return label of the target
  //! corresponding to the optional default/catchall case.  Return 0
  //! if there is no default target.
  OA::StmtLabel getUMultiCatchallLabel(OA::StmtHandle h);

  //! Given an unstructured multi-way branch, return the condition
  //! expression corresponding to target 'targetIndex'. The n targets
  //! are indexed [0..n-1].
  //! multiway target condition 
  OA::ExprHandle getUMultiCondition(OA::StmtHandle h, int targetIndex);

  //--------------------------------------------------------
  // Def/use info for SSA
  //--------------------------------------------------------

  OA::OA_ptr<OA::SSA::IRUseDefIterator> getDefs(OA::StmtHandle h);
  OA::OA_ptr<OA::SSA::IRUseDefIterator> getUses(OA::StmtHandle h);

  //--------------------------------------------------------
  // Symbol Handles
  //--------------------------------------------------------

  OA::SymHandle getProcSymHandle(OA::ProcHandle h);
  OA::SymHandle getSymHandle(OA::LeafHandle h);

  // toString routines
  std::string toString(OA::ProcHandle h) {
    return "";
  }

  std::string toString(OA::StmtHandle h) {
    return "";
  }

  std::string toString(OA::ExprHandle h) {
    return "";
  }

  std::string toString(OA::OpHandle h) {
    return "";
  }

  std::string toString(OA::MemRefHandle h) {
    return "";
  }

  std::string toString(OA::SymHandle h) {
    SEXP e = (SEXP)h.hval();
    assert(TYPEOF(e) == SYMSXP);
    return std::string(CHAR(PRINTNAME(e)));
  }

  std::string toString(OA::ConstSymHandle h) {
    return "";
  }

  std::string toString(OA::ConstValHandle h) {
    return "";
  }

  void dump(OA::StmtHandle h, ostream &os);
  
  void dump(OA::MemRefHandle h, ostream &stream) {}

  void currentProc(OA::ProcHandle p) {}

};

//! Get the statement type of an R expression.
OA::CFG::IRStmtType getSexpCfgType(SEXP e);

//--------------------------------------------------------------------
// ITERATORS
//--------------------------------------------------------------------

// FIXME: I don't think R_IRProcIterator is being used.
// If it is, need to change build_procs to use the scope tree.

//! Enumerate all the procedures in a certain IR
class R_IRProcIterator : public OA::IRProcIterator {
private:
  const SEXP exp;
  R_PreorderIterator iter;
  std::list<SEXP> procs;
  std::list<SEXP>::iterator proc_iter;
  void build_procs();
public:
  R_IRProcIterator(OA::StmtHandle _exp)
    : exp((SEXP)_exp.hval()), iter((SEXP)_exp.hval()) {
    build_procs();
    proc_iter = procs.begin();
  }
  virtual ~R_IRProcIterator() { }

  OA::ProcHandle current() const { return (OA::irhandle_t)(*proc_iter); }
  bool isValid() const { return (proc_iter != procs.end()); }
  void operator++() { ++proc_iter; }
  void reset() { proc_iter = procs.begin(); }
};

//! Enumerate all the statements in a program region, e.g. all the statements
//! in a procedure or a loop. Does not descend into compound statements.
class R_RegionStmtIterator : public OA::IRRegionStmtIterator {
private:
  R_ExpIterator *stmt_iter_ptr;
  void build_stmt_list(OA::StmtHandle stmt);
public:
  R_RegionStmtIterator(OA::StmtHandle stmt) { build_stmt_list(stmt); }  // stmt_iter_ptr = new ...
  virtual ~R_RegionStmtIterator() { delete stmt_iter_ptr; };
  
  OA::StmtHandle current() const { return (OA::irhandle_t)stmt_iter_ptr->current(); }
  bool isValid() const { return stmt_iter_ptr->isValid(); }
  void operator++() { ++*stmt_iter_ptr; }
  void reset() { stmt_iter_ptr->reset(); }
};

//! Special-case version of R_RegionStmtListIterator: iterates through
//! each of a list of statements. Each element is returned as a cell.
//! Useful when you can't pass a cell to R_RegionStmtIterator.
class R_RegionStmtListIterator : public OA::IRRegionStmtIterator {
private:
  R_ListIterator iter;
public:
  R_RegionStmtListIterator(SEXP ls) : iter(ls) { }
  virtual ~R_RegionStmtListIterator() { };
  
  OA::StmtHandle current() const { return (OA::irhandle_t)iter.current(); }
  bool isValid() const { return iter.isValid(); }
  void operator++() { ++iter; }
  void reset() { iter.reset(); }
};

//! Enumerate all the variable uses or variable definitions in a statement.
//! This is useful for analyses that require information about variable
//! references or definitions, such as SSA construction.
class R_IRUseDefIterator : public OA::SSA::IRUseDefIterator {
public:
  R_IRUseDefIterator(OA::OA_ptr<R_VarRefSetIterator> _iter) : iter(_iter) { }
  virtual ~R_IRUseDefIterator() { }
  
  OA::LeafHandle current() const {
    return OA::LeafHandle((OA::irhandle_t)iter->current()->get_sexp());
  }
  bool isValid() { return iter->isValid(); }
  void operator++() { ++*iter; }
  void reset() { iter->reset(); }
protected:
  OA::OA_ptr<R_VarRefSetIterator> iter;
};


#if 0
class R_IRUseIterator : public R_IRUseDefIterator {
private:
  void build_vars_lhs(SEXP e);
  void build_vars_rhs(SEXP e);

public:
  R_IRUseIterator(OA::StmtHandle stmt) {
    build_vars_rhs((SEXP)stmt.hval());
    vars_iter = vars.begin();
  }
};

class R_IRDefIterator : public R_IRUseDefIterator {
private:
  void build_vars_lhs(SEXP e);
  void build_vars_rhs(SEXP e);

public:
  R_IRDefIterator(OA::StmtHandle stmt) {
    build_vars_rhs((SEXP)stmt.hval());
    vars_iter = vars.begin();
  }
};
#endif

//! Iterates over the tree of nested scopes. Needed?
class ScopeTreeIterator {
};

#endif // #ifndef R_IRInterface_h

#if 0
// Enumerate all the procedure calls in a statement.
// Not yet implemented.

class R_IRCallsiteIterator : public IRCallsiteIterator {
private:
  const SEXP stmt;
  R_PreorderIterator exp_iter;
  std::list<SEXP> callsites;
  std::list<SEXP>::iterator cs_iter;
  void build_callsites();
public:
  R_IRCallsiteIterator(OA::StmtHandle _stmt)
    : stmt((SEXP)_stmt), exp_iter(stmt) { build_callsites(); cs_iter = callsites.begin();}
  virtual ~R_IRCallsiteIterator() { }

  OA::ExprHandle current() const { return (OA::ExprHandle)*cs_iter; }
  bool isValid() const { return (cs_iter != callsites.end()); }
  void operator++() { ++cs_iter; }
  void reset() { cs_iter = callsites.begin(); }
};

//! Enumerate all (actual) parameters within a callsite
class R_IRCallsiteParamIterator : public IRCallsiteParamIterator {
private:
  const SEXP args;
  R_ListIterator args_iter;
public:
  // CDR of the expression is the list of arguments
  R_IRCallsiteParamIterator(OA::ExprHandle stmt) : args(CDR((SEXP)stmt)), args_iter(args) { }
  virtual ~R_IRCallsiteParamIterator() { }

  OA::ExprHandle current() const { return (OA::ExprHandle)args_iter.current(); }
  bool isValid() const { return args_iter.isValid(); }
  void operator++() { ++args_iter; }
  void reset() { args_iter.reset(); }
};

#endif

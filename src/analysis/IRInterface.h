// -*- Mode: C++ -*-

#ifndef IR_INTERFACE_H
#define IR_INTERFACE_H

// Copyright (c) 2006 Rice University
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 

// File: IRInterface.h
//
// Derived class of OpenAnalysis base classes; defines the interface
// between R and OA.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <list>
#include <string>
#include <ostream>
#include <assert.h>

#include <OpenAnalysis/CFG/ManagerCFG.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/IRInterface/CFGIRInterfaceDefault.hpp>
#include <OpenAnalysis/IRInterface/CallGraphIRInterface.hpp>
#include <OpenAnalysis/IRInterface/InterSideEffectIRInterface.hpp>
#include <OpenAnalysis/IRInterface/SSAIRInterface.hpp>
#include <OpenAnalysis/Location/Location.hpp>

#include <include/R/R_RInternals.h>

#include <analysis/ExpressionInfo.h>
#include <analysis/FuncInfo.h>
#include <analysis/SimpleIterators.h>
#include <analysis/Utils.h>
#include <analysis/VarRefSet.h>

//-----------------------------------------------------------------------------
// R IR representation

// TODO: make this a singleton
// TODO: split this into separate singleton classes
/// OpenAnalysis interface to the R AST
class R_IRInterface : public virtual OA::CFG::CFGIRInterfaceDefault,
		      public virtual OA::CallGraph::CallGraphIRInterface,
	     //		      public virtual OA::SideEffect::InterSideEffectIRInterface,
		      public virtual OA::SSA::SSAIRInterface
{
public:
  R_IRInterface() {}
  virtual ~R_IRInterface() {}
  
  //--------------------------------------------------------
  // Procedures and call sites
  //--------------------------------------------------------

  /// Given a ProcHandle, return an IRRegionStmtIterator for the
  /// procedure.
  OA::OA_ptr<OA::IRRegionStmtIterator> procBody(OA::ProcHandle h);
  
  //--------------------------------------------------------
  // Statements: General
  //--------------------------------------------------------

  /// Are return statements allowed?
  bool returnStatementsAllowed();

  /// Given a statement, return its CFG::IRStmtType.
  OA::CFG::IRStmtType getCFGStmtType(OA::StmtHandle h);

  /// Given a statement, return a label (or NULL if
  /// there is no label associated with the statement).
  OA::StmtLabel getLabel(OA::StmtHandle h);

  /// Given a compound statement, return an IRRegionStmtIterator for
  /// the statements.  A compound is a list of statements.
  OA::OA_ptr<OA::IRRegionStmtIterator> getFirstInCompound(OA::StmtHandle h);
  
  //--------------------------------------------------------
  // Loops
  //--------------------------------------------------------

  /// Given a loop statement, return an IRRegionStmtIterator pointer
  /// for the loop body.
  OA::OA_ptr<OA::IRRegionStmtIterator> loopBody(OA::StmtHandle h);

  /// Given a loop statement, return the loop header statement.  This 
  /// would be the initialization statement in a C 'for' loop, for example.
  OA::StmtHandle loopHeader(OA::StmtHandle h);

  /// Given a loop statement, return the increment statement. 
  OA::StmtHandle getLoopIncrement(OA::StmtHandle h);

  /// Given a loop statement, return:
  /// 
  /// True: If the number of loop iterations is defined
  /// at loop entry (i.e. Fortran semantics).  This causes the CFG builder 
  /// to add the loop statement representative to the header node so that
  /// definitions from inside the loop don't reach the condition and increment
  /// specifications in the loop statement.
  ///
  /// False: If the number of iterations is not defined at
  /// entry (i.e. C semantics), we add the loop statement to a node that
  /// is inside the loop in the CFG so definitions inside the loop will 
  /// reach uses in the conditional test. For C style semantics, the 
  /// increment itself may be a separate statement. if so, it will appear
  /// explicitly at the bottom of the loop. 
  bool loopIterationsDefinedAtEntry(OA::StmtHandle h);

  /// Given a structured two-way conditional statement, return an
  /// IRRegionStmtIterator for the "true" part (i.e., the statements
  /// under the "if" clause).
  OA::OA_ptr<OA::IRRegionStmtIterator> trueBody(OA::StmtHandle h);

  /// Given a structured two-way conditional statement, return an
  /// IRRegionStmtIterator for the "else" part (i.e., the statements
  /// under the "else" clause).
  OA::OA_ptr<OA::IRRegionStmtIterator> elseBody(OA::StmtHandle h);

  //--------------------------------------------------------
  // Structured multiway conditionals
  //--------------------------------------------------------

  /// Given a structured multi-way branch, return the number of cases.
  /// The count does not include the default/catchall case.
  int numMultiCases(OA::StmtHandle h);

  /// Given a structured multi-way branch, return an IRRegionStmtIterator* for
  /// the body corresponding to target 'bodyIndex'. The n targets are 
  /// indexed [0..n-1].
  OA::OA_ptr<OA::IRRegionStmtIterator> multiBody(OA::StmtHandle h, int bodyIndex);

  /// Given a structured multi-way branch, return true if the cases have
  /// implied break semantics.  For example, this method would return false
  /// for C since one case will fall-through to the next if there is no
  /// explicit break statement.  Matlab, on the other hand, implicitly exits
  /// the switch statement once a particular case has executed, so this
  /// method would return true.
  bool isBreakImplied(OA::StmtHandle multicond);

  /// Given a structured multi-way branch, return true if the body 
  /// corresponding to target 'bodyIndex' is the default/catchall/ case.
  bool isCatchAll(OA::StmtHandle h, int bodyIndex);

  /// Given a structured multi-way branch, return an IRRegionStmtIterator*
  /// for the body corresponding to default/catchall case.
  OA::OA_ptr<OA::IRRegionStmtIterator> getMultiCatchall (OA::StmtHandle h);

  /// Given a structured multi-way branch, return the condition
  /// expression corresponding to target 'bodyIndex'. The n targets are
  /// indexed [0..n-1].
  OA::ExprHandle getSMultiCondition(OA::StmtHandle h, int bodyIndex);

  //--------------------------------------------------------
  // Unstructured two-way conditionals: 
  //--------------------------------------------------------

  /// Given an unstructured two-way branch, return the label of the
  /// target statement.  The second parameter is currently unused.
  OA::StmtLabel getTargetLabel(OA::StmtHandle h, int n);

  /// Given an unstructured multi-way branch, return the number of targets.
  /// The count does not include the optional default/catchall case.
  int numUMultiTargets(OA::StmtHandle h);

  /// Given an unstructured multi-way branch, return the label of the target
  /// statement at 'targetIndex'. The n targets are indexed [0..n-1]. 
  OA::StmtLabel getUMultiTargetLabel(OA::StmtHandle h, int targetIndex);

  /// Given an unstructured multi-way branch, return label of the target
  /// corresponding to the optional default/catchall case.  Return 0
  /// if there is no default target.
  OA::StmtLabel getUMultiCatchallLabel(OA::StmtHandle h);

  /// Given an unstructured multi-way branch, return the condition
  /// expression corresponding to target 'targetIndex'. The n targets
  /// are indexed [0..n-1].
  /// multiway target condition 
  OA::ExprHandle getUMultiCondition(OA::StmtHandle h, int targetIndex);

  //----------------------------------------------------------------------
  // Information for building call graphs
  //----------------------------------------------------------------------

  /// Given a subprogram return an IRStmtIterator for the entire
  /// subprogram
  OA::OA_ptr<OA::IRStmtIterator> getStmtIterator(OA::ProcHandle h);

  /// Return an iterator over all of the callsites in a given stmt
  OA::OA_ptr<OA::IRCallsiteIterator> getCallsites(OA::StmtHandle h);

  /// Given a ProcHandle, return its SymHandle
  // OA::SymHandle getProcSymHandle(OA::ProcHandle h);
  // already defined for implementing CFG interface

  /// Given a procedure call create a memory reference expression
  /// to describe that call.  For example, a normal call is
  /// a NamedRef.  A call involving a function ptr is a Deref.  
  OA::OA_ptr<OA::MemRefExpr> getCallMemRefExpr(OA::CallHandle call);

  //--------------------------------------------------------
  // Information for solving call graph data flow problems
  //--------------------------------------------------------

  /// Get IRCallsiteParamIterator for a callsite.
  /// Iterator visits actual parameters in called order.
  virtual OA::OA_ptr<OA::IRCallsiteParamIterator> getCallsiteParams(OA::CallHandle h);

  /// return the formal parameter that an actual parameter is associated with 
  virtual OA::SymHandle getFormalForActual(OA::ProcHandle caller, OA::CallHandle call, 
					   OA::ProcHandle callee, OA::ExprHandle param);
 
  /// For the given symbol create a Location that indicates statically
  /// overlapping locations and information about whether the location
  /// is local or not for the given procedure, local means only visible
  /// in this procedure
  virtual OA::OA_ptr<OA::Location> getLocation(OA::ProcHandle p, OA::SymHandle s);

  /// Given an ExprHandle, return an ExprTree 
  virtual OA::OA_ptr<OA::ExprTree> getExprTree(OA::ExprHandle h);

  //--------------------------------------------------------
  // Def/use info for SSA
  //--------------------------------------------------------

  OA::OA_ptr<OA::SSA::IRUseDefIterator> getDefs(OA::StmtHandle h);
  OA::OA_ptr<OA::SSA::IRUseDefIterator> getUses(OA::StmtHandle h);

  void dump(OA::StmtHandle h, ostream &os);
  void dump(OA::MemRefHandle h, ostream &stream);

  //--------------------------------------------------------
  // Symbol Handles
  //--------------------------------------------------------

  OA::SymHandle getProcSymHandle(OA::ProcHandle h);
  OA::SymHandle getSymHandle(OA::LeafHandle h);

  std::string toString(OA::ProcHandle h);
  std::string toString(OA::StmtHandle h);
  std::string toString(OA::ExprHandle h);
  std::string toString(OA::OpHandle h);
  std::string toString(OA::MemRefHandle h);
  std::string toString(OA::CallHandle h);
  std::string toString(OA::SymHandle h);
  std::string toString(OA::ConstSymHandle h);
  std::string toString(OA::ConstValHandle h);

  void currentProc(OA::ProcHandle p);

};

/// Get the statement type of an R expression.
OA::CFG::IRStmtType getSexpCfgType(SEXP e);

//--------------------------------------------------------------------
// Iterators
//--------------------------------------------------------------------

/// Enumerate all the statements in a program region, e.g. all the statements
/// in a procedure or a loop. Does not descend into compound statements.
class R_RegionStmtIterator : public OA::IRRegionStmtIterator {
public:
  R_RegionStmtIterator(OA::StmtHandle stmt) { build_stmt_list(stmt); }  // stmt_iter_ptr = new ...
  virtual ~R_RegionStmtIterator() { delete stmt_iter_ptr; };
  
  OA::StmtHandle current() const;
  bool isValid() const;
  void operator++();
  void reset();

private:
  R_ExpIterator *stmt_iter_ptr;
  void build_stmt_list(OA::StmtHandle stmt);
};

/// Special-case version of R_RegionStmtListIterator: iterates through
/// each of a list of statements. Each element is returned as a cell.
/// Useful when you can't pass a cell to R_RegionStmtIterator.
class R_RegionStmtListIterator : public OA::IRRegionStmtIterator {
public:
  R_RegionStmtListIterator(SEXP ls) : iter(ls) { }
  virtual ~R_RegionStmtListIterator() { };
  
  OA::StmtHandle current() const;
  bool isValid() const;
  void operator++();
  void reset();

private:
  R_ListIterator iter;
};

/// Enumerate all the variable uses or variable definitions in a statement.
/// This is useful for analyses that require information about variable
/// references or definitions, such as SSA construction.
class R_IRUseDefIterator : public OA::SSA::IRUseDefIterator {
public:
  R_IRUseDefIterator(OA::OA_ptr<R_VarRefSetIterator> _iter) : iter(_iter) { }
  virtual ~R_IRUseDefIterator() { }
  
  OA::LeafHandle current() const;
  bool isValid();
  void operator++();
  void reset();
private:
  OA::OA_ptr<R_VarRefSetIterator> iter;
};


//--------------------------------------------------------------------
// R_IRCallsiteIterator
//
// Enumerate all the procedure calls in a statement.
//--------------------------------------------------------------------

class R_IRCallsiteIterator : public OA::IRCallsiteIterator {
public:
  R_IRCallsiteIterator(OA::StmtHandle _h);
  virtual ~R_IRCallsiteIterator();

  OA::CallHandle current() const;
  bool isValid() const;
  void operator++();
  void reset();

private:
  RAnnot::ExpressionInfo * const m_annot;
  RAnnot::ExpressionInfo::const_call_site_iterator m_begin;
  RAnnot::ExpressionInfo::const_call_site_iterator m_end;
  RAnnot::ExpressionInfo::const_call_site_iterator m_current;
};

#if 0

/// Enumerate all (actual) parameters within a callsite

// TODO: implement

class R_IRCallsiteParamIterator : public IRCallsiteParamIterator {
private:
  const SEXP args;
  R_ListIterator args_iter;
public:
  // CDR of the expression is the list of arguments
  R_IRCallsiteParamIterator(OA::ExprHandle stmt) : args(CDR(make_sexp(stmt))), args_iter(args) { }
  virtual ~R_IRCallsiteParamIterator() { }

  OA::ExprHandle current() const { return (OA::ExprHandle)args_iter.current(); }
  bool isValid() const { return args_iter.isValid(); }
  void operator++() { ++args_iter; }
  void reset() { args_iter.reset(); }
};

#endif

class R_ProcHandleIterator : public OA::ProcHandleIterator {
public:
  R_ProcHandleIterator(RAnnot::FuncInfo * fi);
  ~R_ProcHandleIterator();
  OA::ProcHandle current() const;
  bool isValid() const;
  void operator++();
  void reset();

private:
  RAnnot::FuncInfoIterator * m_fii;
};

#endif // #ifndef IR_INTERFACE_H

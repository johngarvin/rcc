// -*- Mode: C++ -*-
//
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

// File: IRInterface.cc
//
// Derived class of OpenAnalysis base classes; defines the interface
// between R and OA.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/AnalysisResults.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/Var.h>
#include <analysis/DefVar.h>
#include <analysis/Utils.h>

#include <support/RccError.h>

#include <analysis/IRInterface.h>

using namespace OA;
using namespace RAnnot;
using namespace HandleInterface;

//--------------------------------------------------------
// Procedures and call sites
//--------------------------------------------------------

/// Given a ProcHandle, return an IRRegionStmtIterator for the
/// procedure.
OA_ptr<IRRegionStmtIterator> R_IRInterface::procBody(ProcHandle h) {
  OA_ptr<IRRegionStmtIterator> ptr;
  ptr = new R_RegionStmtIterator(make_stmt_h(fundef_body_c(make_sexp(h))));
  return ptr;
}

//--------------------------------------------------------
// Statements: General
//--------------------------------------------------------

/// Return statements are allowed in R.
bool R_IRInterface::returnStatementsAllowed() { return true; }

/// Local R-specific function: return the CFG type (loop, conditional,
/// etc.) of an SEXP.
CFG::IRStmtType getSexpCfgType(SEXP e) {
  switch(TYPEOF(e)) {
  case NILSXP:  // expressions as statements
  case REALSXP:
  case LGLSXP:
  case STRSXP:
  case SYMSXP:
    return CFG::SIMPLE;
    break;
  case LANGSXP:
    if (is_fundef(e) || is_paren_exp(e) || is_curly_list(e)) {
      return CFG::COMPOUND;
    } else if (is_loop(e)) {
      return CFG::LOOP;
    } else if (is_if(e)) {
      return CFG::STRUCT_TWOWAY_CONDITIONAL;
    } else if (is_return(e)) {
      return CFG::RETURN;
    } else if (is_break(e) || is_stop(e)) {
      return CFG::BREAK;
    } else if (is_next(e)) {
      return CFG::LOOP_CONTINUE;
    } else {
      return CFG::SIMPLE;
    }
    break;
  default:
    assert(0);
  }
  return CFG::SIMPLE; // never reached
}

/// Given a statement handle, return its IRStmtType.
CFG::IRStmtType R_IRInterface::getCFGStmtType(StmtHandle h) {
  return getSexpCfgType(CAR(make_sexp(h)));
}

/// Given a statement, return a label (or StmtHandle(0) if there is no
/// label associated with the statement). No statements have labels in
/// R.
StmtLabel R_IRInterface::getLabel(StmtHandle h) {
  return 0;
}

/// Given a compound statement (which contains a list of statements),
/// return an IRRegionStmtIterator for the statements.
OA_ptr<IRRegionStmtIterator> R_IRInterface::getFirstInCompound(StmtHandle h) {
  SEXP cell = make_sexp(h);
  SEXP e = CAR(cell);
  OA_ptr<IRRegionStmtIterator> ptr;
  if (is_curly_list(e)) {
    // use the iterator that doesn't take a cell
    ptr = new R_RegionStmtListIterator(curly_body(e));
  } else if (is_paren_exp(e)) {
    ptr = new R_RegionStmtIterator(make_stmt_h(paren_body_c(e)));
  } else if (is_fundef(e)) {
    ptr = new R_RegionStmtIterator(make_stmt_h(fundef_body_c(e)));
#if 0
  } else if (is_loop(e)) {
    ptr = new R_RegionStmtIterator(make_stmt_h(loop_body_c(e)));
#endif
  } else {
    rcc_error("getFirstInCompound: unrecognized statement type");
  }
  return ptr;
}
  
//--------------------------------------------------------
// Loops
//--------------------------------------------------------

/// Given a loop statement, return an IRRegionStmtIterator for the loop body.
OA_ptr<IRRegionStmtIterator> R_IRInterface::loopBody(StmtHandle h) {
  OA_ptr<IRRegionStmtIterator> ptr;
  ptr = new R_RegionStmtIterator(make_stmt_h(loop_body_c(CAR(make_sexp(h)))));
  return ptr;
}

/// Given a loop statement, return the loop header statement.  This 
/// would be the initialization statement in a C 'for' loop, for example.
///
/// This doesn't exactly exist in R. Currently just returning the whole
/// compound statement pointer so later analyses can parse it.
StmtHandle R_IRInterface::loopHeader(StmtHandle h) {
  // TODO: signal that this is a header for use/def analysis
  return h;
}

/// Given a loop statement, return the increment statement.
///
/// This doesn't exactly exist in R. Currently just returning the whole
/// compound statement pointer so later analyses can parse it.
StmtHandle R_IRInterface::getLoopIncrement(StmtHandle h) {
  // TODO: signal that this is the increment for use/def analysis
  return h;
}

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
bool R_IRInterface::loopIterationsDefinedAtEntry(StmtHandle h) {
  return true;
}

/// Given a structured two-way conditional statement, return an
/// IRRegionStmtIterator for the "true" part (i.e., the statements
/// under the "if" clause).
OA_ptr<IRRegionStmtIterator> R_IRInterface::trueBody(StmtHandle h) {
  OA_ptr<IRRegionStmtIterator> ptr;
  ptr = new R_RegionStmtIterator(make_stmt_h(if_truebody_c(CAR(make_sexp(h)))));
  return ptr;
}

/// Given a structured two-way conditional statement, return an
/// IRRegionStmtIterator for the "else" part (i.e., the statements
/// under the "else" clause).
OA_ptr<IRRegionStmtIterator> R_IRInterface::elseBody(StmtHandle h) {
  OA_ptr<IRRegionStmtIterator> ptr;
  ptr = new R_RegionStmtIterator(make_stmt_h(if_falsebody_c(CAR(make_sexp(h)))));
  return ptr;
}

//--------------------------------------------------------
// Structured multiway conditionals
//--------------------------------------------------------

// Given a structured multi-way branch, return the number of cases.
// The count does not include the default/catchall case.
int R_IRInterface::numMultiCases(StmtHandle h) {
  rcc_error("multicase branches don't exist in R");
  return -1;
}

// Given a structured multi-way branch, return an
// IRRegionStmtIterator* for the body corresponding to target
// 'bodyIndex'. The n targets are indexed [0..n-1].  The user must
// free the iterator's memory via delete.
OA_ptr<IRRegionStmtIterator> R_IRInterface::multiBody(StmtHandle h, int bodyIndex) {
  rcc_error("multicase branches don't exist in R");
  OA_ptr<IRRegionStmtIterator> dummy;
  return dummy;
}

// Given a structured multi-way branch, return true if the cases have
// implied break semantics.  For example, this method would return false
// for C since one case will fall-through to the next if there is no
// explicit break statement.  Matlab, on the other hand, implicitly exits
// the switch statement once a particular case has executed, so this
// method would return true.
bool R_IRInterface::isBreakImplied(StmtHandle multicond) {
  rcc_error("multicase branches don't exist in R");
  return false;
}

// Given a structured multi-way branch, return true if the body 
// corresponding to target 'bodyIndex' is the default/catchall/ case.
bool R_IRInterface::isCatchAll(StmtHandle h, int bodyIndex) {
  rcc_error("multicase branches don't exist in R");
  return false;
}

// Given a structured multi-way branch, return an IRRegionStmtIterator*
// for the body corresponding to default/catchall case.  The user
// must free the iterator's memory via delete.
OA_ptr<IRRegionStmtIterator> R_IRInterface::getMultiCatchall (StmtHandle h) {
  rcc_error("multicase branches don't exist R");
  OA_ptr<IRRegionStmtIterator> dummy;
  return dummy;
}

/// Given a structured multi-way branch, return the condition
/// expression corresponding to target 'bodyIndex'. The n targets are
/// indexed [0..n-1].
ExprHandle R_IRInterface::getSMultiCondition(StmtHandle h, int bodyIndex) {
  rcc_error("multicase branches don't exist R");
  return 0;
}


//--------------------------------------------------------
// Unstructured two-way conditionals: 
//--------------------------------------------------------

// Given an unstructured two-way branch, return the label of the
// target statement.  The second parameter is currently unused.
StmtLabel R_IRInterface::getTargetLabel(StmtHandle h, int n) {
  rcc_error("unstructured two-way branches don't exist in R");
  return 0;
}

//--------------------------------------------------------
// Unstructured multi-way conditionals
// TODO: Review all of the multi-way stuff.
//--------------------------------------------------------

// Given an unstructured multi-way branch, return the number of targets.
// The count does not include the optional default/catchall case.
int R_IRInterface::numUMultiTargets(StmtHandle h) {
  rcc_error("unstructured multi-way branches don't exist in R");
  return -1;
}

// Given an unstructured multi-way branch, return the label of the target
// statement at 'targetIndex'. The n targets are indexed [0..n-1]. 
StmtLabel R_IRInterface::getUMultiTargetLabel(StmtHandle h, int targetIndex) {
  rcc_error("unstructured multi-way branches don't exist in R");
  return 0;
}

// Given an unstructured multi-way branch, return label of the target
// corresponding to the optional default/catchall case.  Return 0
// if there is no default target.
StmtLabel R_IRInterface::getUMultiCatchallLabel(StmtHandle h) {
  rcc_error("unstructured multi-way branches don't exist in R");
  return 0;
}

// Given an unstructured multi-way branch, return the condition
// expression corresponding to target 'targetIndex'. The n targets
// are indexed [0..n-1].
// multiway target condition 
ExprHandle R_IRInterface::getUMultiCondition(StmtHandle h, int targetIndex) {
  rcc_error("unstructured multi-way branches don't exist in R");
  return 0;
}

//----------------------------------------------------------------------
// Information for building call graphs
//----------------------------------------------------------------------

/// Given a subprogram return an IRStmtIterator for the entire
/// subprogram
OA_ptr<IRStmtIterator> R_IRInterface::getStmtIterator(ProcHandle h) {
  // TODO
  rcc_error("OpenAnalysis call graph interface not yet implemented");
}

/// Return an iterator over all of the callsites in a given stmt
OA_ptr<IRCallsiteIterator> R_IRInterface::getCallsites(StmtHandle h) {
  // TODO
  rcc_error("OpenAnalysis call graph interface not yet implemented");
}

OA_ptr<MemRefExpr> R_IRInterface::getCallMemRefExpr(OA::CallHandle h) {
  // TODO
  rcc_error("OpenAnalysis call graph interface not yet implemented");
}

//----------------------------------------------------------------------
// Information for solving call graph data flow problems
//----------------------------------------------------------------------

OA_ptr<IRCallsiteParamIterator> R_IRInterface::getCallsiteParams(CallHandle h) {
  // TODO
  rcc_error("OpenAnalysis call graph interface not yet implemented");
}

SymHandle R_IRInterface::getFormalForActual(ProcHandle caller, CallHandle call,
					    ProcHandle callee, ExprHandle param)
{
  // TODO
  rcc_error("OpenAnalysis call graph interface not yet implemented");
}

OA_ptr<OA::Location> R_IRInterface::getLocation(ProcHandle p, SymHandle s) {
  // TODO
  rcc_error("OpenAnalysis call graph interface not yet implemented");
}

OA_ptr<ExprTree> R_IRInterface::getExprTree(ExprHandle h) {
  // TODO
  rcc_error("OpenAnalysis call graph interface not yet implemented");
}
  

//--------------------------------------------------------
// Obtain uses and defs for SSA
// TODO: Currently doesn't handle uses/defs via procedure calls
//--------------------------------------------------------

OA_ptr<SSA::IRUseDefIterator> R_IRInterface::getDefs(StmtHandle h) {
  ExpressionInfo * stmt_info = getProperty(ExpressionInfo, make_sexp(h));
  assert(stmt_info != 0);

  // For each variable, insert only if it's a def
  OA_ptr<R_VarRefSet> defs;
  ExpressionInfo::const_var_iterator var_iter;
  for(var_iter = stmt_info->begin_vars(); var_iter != stmt_info->end_vars(); ++var_iter) {
    if ((*var_iter)->getUseDefType() == Var::Var_DEF) {
      OA::OA_ptr<R_BodyVarRef> bvr; bvr = new R_BodyVarRef((*var_iter)->getMention_c());
      defs->insert_ref(bvr);
    }
  }
  OA_ptr<SSA::IRUseDefIterator> retval;
  retval = new R_IRUseDefIterator(defs->get_iterator());
  return retval;
}

OA_ptr<SSA::IRUseDefIterator> R_IRInterface::getUses(StmtHandle h) {
  ExpressionInfo * stmt_info = getProperty(ExpressionInfo, make_sexp(h));
  assert(stmt_info == 0);

  // For each variable, insert only if it's a def
  OA_ptr<R_VarRefSet> defs;
  ExpressionInfo::const_var_iterator var_iter;
  for(var_iter = stmt_info->begin_vars(); var_iter != stmt_info->end_vars(); ++var_iter) {
    if ((*var_iter)->getUseDefType() == Var::Var_DEF) {
      OA::OA_ptr<R_BodyVarRef> bvr; bvr = new R_BodyVarRef((*var_iter)->getMention_c());
      defs->insert_ref(bvr);
    }
  }
  OA_ptr<SSA::IRUseDefIterator> retval;
  retval = new R_IRUseDefIterator(defs->get_iterator());
  return retval;
}

void R_IRInterface::dump(OA::StmtHandle h, ostream &os) {
  Rf_PrintValue(CAR(make_sexp(h)));
  ExpressionInfo * annot = getProperty(ExpressionInfo, make_sexp(h));
  if (annot) {
    annot->dump(os);
  }
}

void R_IRInterface::dump(OA::MemRefHandle h, ostream &stream) {}
void R_IRInterface::currentProc(OA::ProcHandle p) {}

//--------------------------------------------------------
// Symbol Handles
//--------------------------------------------------------

SymHandle R_IRInterface::getProcSymHandle(ProcHandle h) {
  return make_sym_h(Rf_install("<procedure>"));
}

// TODO: symbols in different scopes should be called
// different, even if they have the same name
SymHandle R_IRInterface::getSymHandle(LeafHandle h) {
  SEXP e = make_sexp(h);
  assert(TYPEOF(e) == SYMSXP);
  return make_sym_h(e);
}

// TODO: fill these in
std::string R_IRInterface::toString(OA::ProcHandle h) {
  return "";
}

std::string R_IRInterface::toString(OA::StmtHandle h) {
  return "";
}

std::string R_IRInterface::toString(OA::ExprHandle h) {
  return "";
}

std::string R_IRInterface::toString(OA::OpHandle h) {
  return "";
}

std::string R_IRInterface::toString(OA::MemRefHandle h) {
  return "";
}

std::string R_IRInterface::toString(OA::SymHandle h) {
  return var_name(make_sexp(h));
}

std::string R_IRInterface::toString(OA::ConstSymHandle h) {
  return "";
}

std::string R_IRInterface::toString(OA::ConstValHandle h) {
  return "";
}

std::string R_IRInterface::toString(OA::CallHandle h) {
  return "";
}

//--------------------------------------------------------------------
// R_RegionStmtIterator
//--------------------------------------------------------------------

OA::StmtHandle R_RegionStmtIterator::current() const {
  return make_stmt_h(stmt_iter_ptr->current());
}

bool R_RegionStmtIterator::isValid() const { 
  return stmt_iter_ptr->isValid(); 
}

void R_RegionStmtIterator::operator++() {
  ++*stmt_iter_ptr;
}

void R_RegionStmtIterator::reset() {
  stmt_iter_ptr->reset();
}

/// Build the list of statements for iterator. Does not descend into
/// compound statements. The given StmtHandle is a CONS cell whose CAR
/// is the actual expression.
void R_RegionStmtIterator::build_stmt_list(StmtHandle stmt) {
  SEXP cell = make_sexp(stmt);
  SEXP exp = CAR(cell);
  switch (getSexpCfgType(exp)) {
  case CFG::SIMPLE:
    if (exp == R_NilValue) {
      stmt_iter_ptr = new R_ListIterator(exp);  // empty iterator
    } else {
      stmt_iter_ptr = new R_SingletonIterator(cell);
    }
    break;
  case CFG::COMPOUND:
    if (is_curly_list(exp)) {
      // Special case for a list of statments in curly braces.
      // CDR(exp), our list of statements, is not in a cell. But it is
      // guaranteed to be a list or nil, so we can call the iterator
      // that doesn't take a cell.
      stmt_iter_ptr = new R_ListIterator(CDR(exp));
    } else if (is_loop(exp)) {
      stmt_iter_ptr = new R_SingletonIterator(cell);
    } else {
      // We have a non-loop compound statement with a body. (body_c is
      // the cell containing the body.) This body might be a list for
      // which we're returning an iterator, or it might be some other
      // thing. First we figure out what the body is.
      SEXP body_c;
      if (is_fundef(exp)) {
	body_c = fundef_body_c(exp);
      } else if (is_paren_exp(exp)) {
	body_c = paren_body_c(exp);
      }
      // Now return the appropriate iterator.
      if (TYPEOF(CAR(body_c)) == NILSXP || TYPEOF(CAR(body_c)) == LISTSXP) {
	stmt_iter_ptr = new R_ListIterator(CAR(body_c));
      } else {
	stmt_iter_ptr = new R_SingletonIterator(body_c);
      }
    }
    break;
  default:
    stmt_iter_ptr = new R_SingletonIterator(cell);
    break;
  }
}

//--------------------------------------------------------------------
// R_RegionStmtListIterator
//--------------------------------------------------------------------

OA::StmtHandle R_RegionStmtListIterator::current() const {
  return make_stmt_h(iter.current());
}

bool R_RegionStmtListIterator::isValid() const {
  return iter.isValid();
}

void R_RegionStmtListIterator::operator++() {
  ++iter;
}

void R_RegionStmtListIterator::reset() {
  iter.reset(); 
}

//--------------------------------------------------------------------
// R_IRUseDefIterator
//--------------------------------------------------------------------

OA::LeafHandle R_IRUseDefIterator::current() const {
  return OA::LeafHandle(make_leaf_h(iter->current()->get_sexp()));
}
 
bool R_IRUseDefIterator::isValid() {
  return iter->isValid();
}

void R_IRUseDefIterator::operator++() {
  ++*iter;
}

void R_IRUseDefIterator::reset() {
  iter->reset();
}

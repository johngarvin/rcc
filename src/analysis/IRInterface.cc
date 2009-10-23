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

#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/SideEffect/ManagerSideEffectStandard.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectInterface.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectStandard.hpp>
#include <OpenAnalysis/SideEffect/SideEffectStandard.hpp>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/DefVar.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/ExprTreeBuilder.h>
#include <analysis/FuncInfoAnnotationMap.h>
#include <analysis/HandleInterface.h>
#include <analysis/HellProcedure.h>
#include <analysis/LexicalScope.h>
#include <analysis/MemRefExprInterface.h>
#include <analysis/OACallGraphAnnotationMap.h>
#include <analysis/SimpleIterators.h>
#include <analysis/ScopeAnnotationMap.h>
#include <analysis/SymbolTableFacade.h>
#include <analysis/Var.h>
#include <analysis/VarAnnotationMap.h>
#include <analysis/VarInfo.h>
#include <analysis/VarRefFactory.h>
#include <analysis/Utils.h>

#include <support/RccError.h>
#include <support/StringUtils.h>

#include <analysis/IRInterface.h>

using namespace OA;
using namespace RAnnot;
using namespace HandleInterface;

static OA_ptr<MemRefExprIterator> make_singleton_mre_iterator(OA_ptr<MemRefExpr> mre);

//--------------------------------------------------------
// Procedures and call sites
//--------------------------------------------------------

/// Given a ProcHandle, return an IRRegionStmtIterator for the
/// procedure.
OA_ptr<IRRegionStmtIterator> R_IRInterface::procBody(ProcHandle h) {
  assert(h != HellProcedure::get_instance());
  OA_ptr<IRRegionStmtIterator> ptr;
  ptr = new R_RegionStmtIterator(make_stmt_h(procedure_body_c(make_sexp(h))));
  return ptr;
}

//--------------------------------------------------------
// Statements: General
//--------------------------------------------------------

/// Return statements are allowed in R.
bool R_IRInterface::returnStatementsAllowed() {
  return true;
}

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
    } else if (is_explicit_return(e)) {
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

//-------------------------------------------------------
// Structured two-way conditionals (if statements)
//-------------------------------------------------------

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
  SEXP else_body = if_falsebody_c(CAR(make_sexp(h)));
  if (else_body == R_NilValue) {
    ptr = new R_RegionStmtListIterator(R_NilValue);  // empty iterator
  } else {
    ptr = new R_RegionStmtIterator(make_stmt_h(else_body));
  }
  return ptr;
}

//------------------------------------------------------------------
// Structured multiway conditionals.
// R doesn't have multiway conditionals, so these are all stubs.
//------------------------------------------------------------------

// Given a structured multi-way branch, return the number of cases.
// The count does not include the default/catchall case.
int R_IRInterface::numMultiCases(StmtHandle h) {
  rcc_error("numMultiCases: multicase branches don't exist in R");
  return -1;
}

// Given a structured multi-way branch, return an
// IRRegionStmtIterator* for the body corresponding to target
// 'bodyIndex'. The n targets are indexed [0..n-1].  The user must
// free the iterator's memory via delete.
OA_ptr<IRRegionStmtIterator> R_IRInterface::multiBody(StmtHandle h, int bodyIndex) {
  rcc_error("multiBody: multicase branches don't exist in R");
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
  rcc_error("isBreakImplied: multicase branches don't exist in R");
  return false;
}

// Given a structured multi-way branch, return true if the body 
// corresponding to target 'bodyIndex' is the default/catchall/ case.
bool R_IRInterface::isCatchAll(StmtHandle h, int bodyIndex) {
  rcc_error("isCatchAll: multicase branches don't exist in R");
  return false;
}

// Given a structured multi-way branch, return an IRRegionStmtIterator*
// for the body corresponding to default/catchall case.  The user
// must free the iterator's memory via delete.
OA_ptr<IRRegionStmtIterator> R_IRInterface::getMultiCatchall (StmtHandle h) {
  rcc_error("getMultiCatchall: multicase branches don't exist in R");
  OA_ptr<IRRegionStmtIterator> dummy;
  return dummy;
}

/// Given a structured multi-way branch, return the condition
/// expression corresponding to target 'bodyIndex'. The n targets are
/// indexed [0..n-1].
ExprHandle R_IRInterface::getSMultiCondition(StmtHandle h, int bodyIndex) {
  rcc_error("getSMultiCondition: multicase branches don't exist in R");
  return 0;
}


//--------------------------------------------------------
// Unstructured two-way conditionals: 
//--------------------------------------------------------

// Given an unstructured two-way branch, return the label of the
// target statement.  The second parameter is currently unused.
StmtLabel R_IRInterface::getTargetLabel(StmtHandle h, int n) {
  rcc_error("getTargetLabel: unstructured two-way branches don't exist in R");
  return 0;
}

//--------------------------------------------------------
// Unstructured multi-way conditionals
// TODO: Review all of the multi-way stuff.
//--------------------------------------------------------

// Given an unstructured multi-way branch, return the number of targets.
// The count does not include the optional default/catchall case.
int R_IRInterface::numUMultiTargets(StmtHandle h) {
  rcc_error("numUMultiTargets: unstructured multi-way branches don't exist in R");
  return -1;
}

// Given an unstructured multi-way branch, return the label of the target
// statement at 'targetIndex'. The n targets are indexed [0..n-1]. 
StmtLabel R_IRInterface::getUMultiTargetLabel(StmtHandle h, int targetIndex) {
  rcc_error("getUMultiTargetLabel: unstructured multi-way branches don't exist in R");
  return 0;
}

// Given an unstructured multi-way branch, return label of the target
// corresponding to the optional default/catchall case.  Return 0
// if there is no default target.
StmtLabel R_IRInterface::getUMultiCatchallLabel(StmtHandle h) {
  rcc_error("getUMultiCatchallLabel: unstructured multi-way branches don't exist in R");
  return 0;
}

// Given an unstructured multi-way branch, return the condition
// expression corresponding to target 'targetIndex'. The n targets
// are indexed [0..n-1].
// multiway target condition 
ExprHandle R_IRInterface::getUMultiCondition(StmtHandle h, int targetIndex) {
  rcc_error("getUMultiCondition: unstructured multi-way branches don't exist in R");
  return 0;
}

//------------------------------------------------------------
// AliasIRInterface
//------------------------------------------------------------

/// Given a subprogram return an IRStmtIterator for the entire
/// subprogram
OA_ptr<IRStmtIterator> R_IRInterface::getStmtIterator(ProcHandle h) {
  assert(h != HellProcedure::get_instance());
  // unlike procBody, here we want an iterator that descends into compound statements.
  OA_ptr<IRRegionStmtIterator> ptr;
  ptr = new R_DescendingStmtIterator(make_stmt_h(procedure_body_c(make_sexp(h))));
  return ptr;
}

/// Return an iterator over all the memory reference handles that appear
/// in the given statement.  Order that memory references are iterated
/// over can be arbitrary.
OA_ptr<MemRefHandleIterator> R_IRInterface::getAllMemRefs(StmtHandle stmt) {
  ExpressionInfo * ei = getProperty(ExpressionInfo, make_sexp(stmt));
  OA_ptr<MemRefHandleIterator> iter; iter = new R_ExpMemRefHandleIterator(ei);
  return iter;
}

/// If this is a PTR_ASSIGN_STMT then return an iterator over MemRefHandle
/// pairs where there is a source and target such that target
OA_ptr<Alias::PtrAssignPairStmtIterator> R_IRInterface::getPtrAssignStmtPairIterator(StmtHandle stmt) {
  // we have no PTR_ASSIGN_STMTs, so return an empty iterator
  class EmptyPAPSIterator : public Alias::PtrAssignPairStmtIterator {
    OA_ptr<MemRefExpr> currentSource() const { throw AnalysisException("compiler bug: currentSource called on empty iterator"); }
    OA_ptr<MemRefExpr> currentTarget() const { throw AnalysisException("compiler bug: currentSource called on empty iterator"); }
    bool isValid() const { return false; }
    void operator++() { }
  };
  OA_ptr<Alias::PtrAssignPairStmtIterator> empty; empty = new EmptyPAPSIterator();
  return empty;
}

/// OA: Return an iterator over <int, MemRefExpr> pairs
/// where the integer represents which formal parameter 
/// and the MemRefExpr describes the corresponding actual argument.
///
/// RCC: does not include constants because they are irrelevant to
/// alias analysis.
OA_ptr<Alias::ParamBindPtrAssignIterator> R_IRInterface::getParamBindPtrAssignIterator(CallHandle call) {
  OA_ptr<Alias::ParamBindPtrAssignIterator> retval; retval = new R_ParamBindIterator(make_sexp(call));
  return retval;
}

/// Return the symbol handle for the nth formal parameter to proc
/// Number starts at 0 and implicit parameters should be given
/// a number in the order as well.  This number should correspond
/// to the number provided in getParamBindPtrAssign pairs
/// Should return SymHandle(0) if there is no formal parameter for 
/// given num
SymHandle R_IRInterface::getFormalSym(ProcHandle proc, int n) {
  if (proc == HellProcedure::get_instance()) {
    // TODO: is this the right thing to do?
    return SymHandle(0);
  }
  FuncInfo * fi = getProperty(FuncInfo, make_sexp(proc));
  if (n >= fi->get_num_args()) {
    return SymHandle(0);
  } else {
    SEXP cell = fi->get_arg(n + 1);  // FuncInfo gives 1-based params
    VarInfo * vi = SymbolTableFacade::get_instance()->find_entry(getProperty(Var, cell));
    return make_sym_h(vi);
  }
}

/// Return an iterator over all of the callsites in a given stmt
OA_ptr<IRCallsiteIterator> R_IRInterface::getCallsites(StmtHandle h) {
  // R_IRProgramCallsiteIterator (as opposed to R_IRCallsiteIterator) does not include internal calls
  OA_ptr<IRCallsiteIterator> iter; iter = new R_IRProgramCallsiteIterator(h);
  return iter;
}

/// Given a procedure call create a memory reference expression
/// to describe that call.  For example, a normal call is
/// a NamedRef.  A call involving a function ptr is a Deref.
OA_ptr<MemRefExpr> R_IRInterface::getCallMemRefExpr(CallHandle h) {
  SymbolTableFacade * symbol_table = SymbolTableFacade::get_instance();
  SEXP e = make_sexp(h);
  OA_ptr<MemRefExpr> mre;
  if (is_var(call_lhs(e))) {
    // TODO: redo Annotations so we can use getProperty without requiring the annotations to have a certain name
    FuncInfo * fi = dynamic_cast<FuncInfo *>(ScopeAnnotationMap::get_instance()->get(e));
    VarInfo * vi = symbol_table->find_entry(getProperty(Var, e));  // e is the cell that contains the mention
    SymHandle sym = make_sym_h(vi);
    return MemRefExprInterface::convert_sym_handle(sym);
    // TODO: do something different if this is a parameter.
    // TODO: make sharper distinction
  } else {
    mre = new UnknownRef(MemRefExpr::USE);
  }
  return mre;
}

/// Given the callee symbol returns the callee proc handle
ProcHandle R_IRInterface::getProcHandle(SymHandle sym) {
  VarInfo * vi = make_var_info(sym);
  if (vi->is_param()) {
    rcc_error("getProcHandle: tried to get ProcHandle of procedure symbol parameter");
    return HellProcedure::get_instance();
  }
  VarInfo::ConstDefIterator iter = vi->begin_defs();
  if (iter == vi->end_defs()) {
    rcc_warn("getProcHandle: procedure symbol has no definitions");
    return HellProcedure::get_instance();
  }
  //  std::cout << to_string((*iter)->getRhs_c());
  //  std::cout << to_string(CAR((*iter)->getRhs_c()));
  ProcHandle retval = make_proc_h(CAR((*iter)->getRhs_c()));
  if (++iter != vi->end_defs()) {  // if more than one def
    rcc_warn("getProcHandle: procedure symbol has more than one definition");
    return HellProcedure::get_instance();
  }
  return retval;
}

/// Given a procedure return associated SymHandle
SymHandle R_IRInterface::getSymHandle(ProcHandle h) const {
  /// TODO: R procedures are first class. We need to find a way to
  /// handle more than one procedure bound to the same name in the same
  /// scope. For now we're just giving the first name assigned.
  SymbolTableFacade * symbol_table = SymbolTableFacade::get_instance();
  if (h == HellProcedure::get_instance()) {
    return SymHandle(0);
  }
  FuncInfo * fi = getProperty(FuncInfo, make_sexp(h));
  SEXP name = fi->get_first_name_c();
  if (VarAnnotationMap::get_instance()->is_valid(name)) {
    VarInfo * sym = symbol_table->find_entry(getProperty(Var, name));
    if (sym->size_defs() == 1) {
      return make_sym_h(sym);
    } else {
      // fail gracefully if there's more than one definition
      throw AnalysisException("getSymHandle: more than one definition of procedure");
    }
  } else {
    // the global-scope procedure and anonymous functions won't have a Var annotation.
    // In this case, return 0.
    return SymHandle(0);
  }
}

/// Given a MemRefHandle return an iterator over
/// MemRefExprs that describe this memory reference
///
/// Michelle says there's only one MemRefExpr per MemRefHandle in
/// almost all cases. Here, just return a singleton iterator
/// containing the MemRefExpr.
OA_ptr<MemRefExprIterator> R_IRInterface::getMemRefExprIterator(MemRefHandle h) {
  OA_ptr<MemRefExprIterator> iter;
  OA_ptr<MemRefExpr> mre = MemRefExprInterface::convert_mem_ref_handle(h);
  return make_singleton_mre_iterator(mre);
}

//----------------------------------------------------------------------
// ParamBindingsIRInterface
//----------------------------------------------------------------------

OA_ptr<IRCallsiteParamIterator> R_IRInterface::getCallsiteParams(CallHandle h) {
  SEXP args = call_args(make_sexp(h));
  
  // create wrapper around R_ListIterator that iterates through the
  // params, returning ExprHandles instead of SEXPs
  class R_ParamIterator : public IRCallsiteParamIterator {
  public:
    explicit R_ParamIterator(SEXP _exp) : m_list_iter(_exp) {
    }

    ExprHandle current() const {
      return make_expr_h(m_list_iter.current());
    }

    bool isValid() const {
      return m_list_iter.isValid();
    }

    void operator++() {
      ++m_list_iter;
    }

    void reset() {
      m_list_iter.reset();
    }
    
  private:
    R_ListIterator m_list_iter;
  };

  OA_ptr<IRCallsiteParamIterator> iter; iter = new R_ParamIterator(args);
  return iter;
}

SymHandle R_IRInterface::getFormalForActual(ProcHandle caller, CallHandle call,
					    ProcHandle callee, ExprHandle param)
{
  // TODO: handle named arguments
  // TODO: handle Hell procedure

  SymbolTableFacade * symbol_table = SymbolTableFacade::get_instance();

  // param is the nth actual arg of call: find n
  SEXP actual_c = make_sexp(param);
  SEXP args = call_args(make_sexp(call));
  int n = 1;
  R_ListIterator li(args);
  for( ; li.isValid(); ++li) {
    if (li.current() == actual_c) break;
    n++;
  }
  if (!li.isValid()) {
    rcc_error("getFormalForActual: actual param not found");
  }
    
  // get nth formal of callee
  FuncInfo * fi = getProperty(FuncInfo, make_sexp(callee));
  // hack to deal with varargs (used in .rcc.assert.exp, etc.)
  // TODO: figure out what to do
  if (n > fi->get_num_args()) {
    return SymHandle(0);
  }

  SEXP formal_c = fi->get_arg(n);

  if (TAG(formal_c) == R_DotsSymbol) {
    return make_sym_h(new VarInfo(formal_c));
  }

  VarInfo * vi = symbol_table->find_entry(getProperty(Var, formal_c));
  return make_sym_h(vi);
}

OA_ptr<ExprTree> R_IRInterface::getExprTree(ExprHandle h) {
  return ExprTreeBuilder::get_instance()->build_c(make_sexp(h));
}
  
OA_ptr<MemRefExpr> R_IRInterface::convertSymToMemRefExpr(SymHandle sym) {
  // note: when ManagerParamBindings uses this, it requires the result
  // to be a Deref.
  OA_ptr<MemRefExpr> base = MemRefExprInterface::convert_sym_handle(sym);
  OA_ptr<MemRefExpr> retval; retval = new Deref(MemRefExpr::DEF, base);   // TODO: is DEF right?
  return retval;
}

// getSideEffect
/// For the given callee subprocedure symbol return side-effect results
/// Can only indicate that the procedure has no side effects, has
/// side effects on unknown locations, or on global locations.
/// Can't indicate subprocedure has sideeffects on parameters because
/// don't have a way to get mapping of formal parameters to actuals
/// in caller.
///
/// Note: ManagerInterSideEffectStandard only uses this method for undefined callees.
OA_ptr<SideEffect::SideEffectStandard> R_IRInterface::getSideEffect(ProcHandle caller, SymHandle callee)
{
  assert(caller != HellProcedure::get_instance());
  OA_ptr<R_IRInterface> this_copy; this_copy = new R_IRInterface(*this);
  OA_ptr<SideEffectIRInterface> se_this; se_this = this_copy.convert<SideEffectIRInterface>();
  OA_ptr<AliasIRInterface> alias_this; alias_this = this_copy.convert<AliasIRInterface>();
  VarInfo * vi = make_var_info(callee);
  if (vi->is_internal()) {
    // assuming internal procedures have no effect on names in userland
    OA_ptr<SideEffect::SideEffectStandard> retval; retval = new SideEffect::SideEffectStandard(OACallGraphAnnotationMap::get_instance()->get_OA_alias().convert<Alias::Interface>());
    retval->emptyLMOD();
    retval->emptyMOD();
    retval->emptyLDEF();
    retval->emptyDEF();
    retval->emptyLUSE();
    retval->emptyUSE();
    retval->emptyLREF();
    retval->emptyREF();
    return retval;
  } else {
    // extremely conservative;
    // InterSideEffectIRInterfaceDefault::getSideEffect returns a new
    // SideEffectStandard, indicating side effects on unknown locations
    return InterSideEffectIRInterfaceDefault::getSideEffect(caller, callee);
  }
}

/// from SideEffectIRInterface
/// Return a list of all the target memory reference handles that appear
/// in the given statement.
OA_ptr<MemRefHandleIterator> R_IRInterface::getDefMemRefs(StmtHandle h) {
  DefVar * def;
  VarRefFactory * fact = VarRefFactory::get_instance();
  ExpressionInfo * stmt_info = getProperty(ExpressionInfo, make_sexp(h));
  assert(stmt_info != 0);

  OA_ptr<R_VarRefSet> defs; defs = new R_VarRefSet;
  EXPRESSION_FOR_EACH_DEF(stmt_info, def) {
    OA_ptr<R_BodyVarRef> bvr; bvr = fact->make_body_var_ref(def->getMention_c());
    defs->insert_ref(bvr);
  }
  OA_ptr<MemRefHandleIterator> retval;
  retval = new R_UseDefAsMemRefIterator(defs->get_iterator());
  return retval;
}

/// from SideEffectIRInterface
/// Return a list of all the source memory reference handles that appear
/// in the given statement.
OA_ptr<MemRefHandleIterator> R_IRInterface::getUseMemRefs(StmtHandle h) {
  UseVar * use;
  VarRefFactory * fact = VarRefFactory::get_instance();
  ExpressionInfo * stmt_info = getProperty(ExpressionInfo, make_sexp(h));
  assert(stmt_info != 0);

  OA_ptr<R_VarRefSet> uses; uses = new R_VarRefSet;
  EXPRESSION_FOR_EACH_USE(stmt_info, use) {
    OA_ptr<R_BodyVarRef> bvr; bvr = fact->make_body_var_ref(use->getMention_c());
    uses->insert_ref(bvr);
  }
  OA_ptr<MemRefHandleIterator> retval;
  retval = new R_UseDefAsMemRefIterator(uses->get_iterator());
  return retval;
}

//--------------------------------------------------------
// Obtain uses and defs for SSA
//--------------------------------------------------------

OA_ptr<SSA::IRUseDefIterator> R_IRInterface::getDefs(StmtHandle h) {
  DefVar * def;
  ExpressionInfo * stmt_info = getProperty(ExpressionInfo, make_sexp(h));
  assert(stmt_info != 0);

  OA_ptr<R_VarRefSet> defs; defs = new R_VarRefSet;
  VarRefFactory * fact = VarRefFactory::get_instance();
  EXPRESSION_FOR_EACH_DEF(stmt_info, def) {
    OA_ptr<R_BodyVarRef> bvr; bvr = fact->make_body_var_ref(def->getMention_c());
    defs->insert_ref(bvr);
  }
  OA_ptr<SSA::IRUseDefIterator> retval;
  retval = new R_UseDefAsLeafIterator(defs->get_iterator());
  return retval;
}

OA_ptr<SSA::IRUseDefIterator> R_IRInterface::getUses(StmtHandle h) {
  UseVar * use;
  ExpressionInfo * stmt_info = getProperty(ExpressionInfo, make_sexp(h));
  assert(stmt_info != 0);

  OA_ptr<R_VarRefSet> uses; uses = new R_VarRefSet;
  VarRefFactory * fact = VarRefFactory::get_instance();
  EXPRESSION_FOR_EACH_USE(stmt_info, use) {
    OA_ptr<R_BodyVarRef> bvr; bvr = fact->make_body_var_ref(use->getMention_c());
    uses->insert_ref(bvr);
  }
  OA_ptr<SSA::IRUseDefIterator> retval;
  retval = new R_UseDefAsLeafIterator(uses->get_iterator());
  return retval;
}


SymHandle R_IRInterface::getSymHandle(LeafHandle h) {
  SEXP sexp = make_sexp(h);
  assert(is_var(sexp));
  const Var * var = getProperty(Var, sexp);
  const VarInfo * vi = SymbolTableFacade::get_instance()->find_entry(var);
  return make_sym_h(vi);
}

//------------------------------------------------------------
// Dump routines for debugging
//------------------------------------------------------------

/// from IRHandlesIRInterface
void R_IRInterface::dump(StmtHandle h, ostream &os) {
  os << toString(h);
  ExpressionInfo * annot = getProperty(ExpressionInfo, make_sexp(h));
  if (annot) {
    annot->dump(os);
  }
}

/// from IRHandlesIRInterface
void R_IRInterface::dump(MemRefHandle h, ostream &stream) {
  stream << toString(h);
}

std::string R_IRInterface::toString(ProcHandle h) const {
  if (h == ProcHandle(0)) {
    return "ProcHandle(0)";
  } else if (h == HellProcedure::get_instance()) {
    return "*Hell Procedure*";
  } else {
    FuncInfo * fi = getProperty(FuncInfo, make_sexp(h));
    return var_name(CAR(fi->get_first_name_c()));
  }
}

std::string R_IRInterface::toString(StmtHandle h) const {
  return to_string(make_sexp(h));
}

std::string R_IRInterface::toString(ExprHandle h) const {
  return to_string(make_sexp(h));
}

std::string R_IRInterface::toString(OpHandle h) const {
  return to_string(make_sexp(h));
}

std::string R_IRInterface::toString(MemRefHandle h) const {
  return to_string(make_sexp(h));
}

std::string R_IRInterface::toString(SymHandle h) const {
  VarInfo * vi = make_var_info(h);
  return to_string(vi->get_name()) + "@" + vi->get_scope()->get_name();
}

std::string R_IRInterface::toString(ConstSymHandle h) const {
  return "*ConstSymHandle*";
}

std::string R_IRInterface::toString(ConstValHandle h) const {
  return to_string(make_sexp(h));
}

std::string R_IRInterface::toString(CallHandle h) const {
  return to_string(make_sexp(h));
}

//--------------------------------------------------------------------
// R_RegionStmtIterator
//
// Statement iterator that does not descend into compound statements.
//--------------------------------------------------------------------

R_RegionStmtIterator::R_RegionStmtIterator(OA::StmtHandle stmt) {
  build_stmt_list(stmt);
}

R_RegionStmtIterator::~R_RegionStmtIterator() {
  delete stmt_iter_ptr;
}

StmtHandle R_RegionStmtIterator::current() const {
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
  if (cell == R_NilValue) {
    return;
  }
  assert(is_cons(cell));
  SEXP exp = CAR(cell);
  switch (getSexpCfgType(exp)) {
  case CFG::SIMPLE:
    if (exp == R_NilValue) {
      stmt_iter_ptr = new R_ListIterator(R_NilValue);  // empty iterator
    } else {
      stmt_iter_ptr = new R_SingletonSEXPIterator(cell);
    }
    break;
  case CFG::LOOP:
  case CFG::STRUCT_TWOWAY_CONDITIONAL:
  case CFG::RETURN:
  case CFG::BREAK:
  case CFG::LOOP_CONTINUE:
    stmt_iter_ptr = new R_SingletonSEXPIterator(cell);
    break;
  case CFG::COMPOUND:
    if (is_curly_list(exp)) {
      // Special case for a list of statments in curly braces.
      // CDR(exp), our list of statements, is not in a cell. But it is
      // guaranteed to be a list or nil, so we can call the iterator
      // that doesn't take a cell.
      stmt_iter_ptr = new R_ListIterator(CDR(exp));
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
	stmt_iter_ptr = new R_SingletonSEXPIterator(body_c);
      }
    }
    break;
  default:
    rcc_error("R_RegionStmtIterator::build_stmt_list: unrecognized CFG statement type");
    break;
  }
}

//------------------------------------------------------------
// R_DescendingStmtIterator
//------------------------------------------------------------

R_DescendingStmtIterator::R_DescendingStmtIterator(OA::StmtHandle stmt) {
  build_stmt_list(make_sexp(stmt));
  m_iter = m_stmts.begin();
}

R_DescendingStmtIterator::~R_DescendingStmtIterator() {
}

StmtHandle R_DescendingStmtIterator::current() const {
  return *m_iter;
}

bool R_DescendingStmtIterator::isValid() const {
  return (m_iter != m_stmts.end());
}

void R_DescendingStmtIterator::operator++() {
  ++m_iter;
}

void R_DescendingStmtIterator::reset() {
  m_iter = m_stmts.begin();
}

void R_DescendingStmtIterator::build_stmt_list(SEXP exp_c) {
  assert(exp_c != 0);
  if (exp_c == R_NilValue) {
    return;
  }
  assert(is_cons(exp_c));
  SEXP exp = CAR(exp_c);
  switch(getSexpCfgType(exp)) {
  case CFG::COMPOUND:
    if (is_curly_list(exp)) {
      for (SEXP e = curly_body(exp); e != R_NilValue; e = CDR(e)) {
	build_stmt_list(e);
      }
    } else if (is_fundef(exp)) {
      // no-op
      // (nested fundefs are NOT counted as part of the parent)
    } else if (is_paren_exp(exp)) {
      build_stmt_list(paren_body_c(exp));
    }
    break;
  case CFG::LOOP:
    if (is_for(exp)) {
      build_stmt_list(for_range_c(exp));
      build_stmt_list(for_body_c(exp));
    } else if (is_while(exp)) {
      build_stmt_list(while_cond_c(exp));
      build_stmt_list(while_body_c(exp));
    } else if (is_repeat(exp)) {
      build_stmt_list(repeat_body_c(exp));
    }
    break;
  case CFG::STRUCT_TWOWAY_CONDITIONAL:  // a.k.a. "if"
    build_stmt_list(if_truebody_c(exp));
    build_stmt_list(if_falsebody_c(exp));
    break;
  default:  // including CFG::SIMPLE
    if (exp != R_NilValue) {
      m_stmts.push_back(make_stmt_h(exp_c));
    }
  }
}

//--------------------------------------------------------------------
// R_RegionStmtListIterator
//--------------------------------------------------------------------

StmtHandle R_RegionStmtListIterator::current() const {
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
// R_UseDefAsLeafIterator
//--------------------------------------------------------------------

R_UseDefAsLeafIterator::R_UseDefAsLeafIterator(OA_ptr<R_VarRefSetIterator> iter)
  : m_iter(iter)
{
}

R_UseDefAsLeafIterator::~R_UseDefAsLeafIterator() {
}

LeafHandle R_UseDefAsLeafIterator::current() const {
  return make_leaf_h(m_iter->current()->get_sexp());
}
 
bool R_UseDefAsLeafIterator::isValid() {
  return m_iter->isValid();
}

void R_UseDefAsLeafIterator::operator++() {
  ++*m_iter;
}

void R_UseDefAsLeafIterator::reset() {
  m_iter->reset();
}

//------------------------------------------------------------
// R_UseDefAsMemRefIterator
//------------------------------------------------------------

R_UseDefAsMemRefIterator::R_UseDefAsMemRefIterator(OA_ptr<R_VarRefSetIterator> iter)
  : m_iter(iter)
{
}

R_UseDefAsMemRefIterator::~R_UseDefAsMemRefIterator() {
}

MemRefHandle R_UseDefAsMemRefIterator::current() const {
  return make_mem_ref_h(m_iter->current()->get_sexp());
}
 
bool R_UseDefAsMemRefIterator::isValid() const {
  return m_iter->isValid();
}

void R_UseDefAsMemRefIterator::operator++() {
  ++*m_iter;
}

void R_UseDefAsMemRefIterator::reset() {
  m_iter->reset();
}

//--------------------------------------------------------------------
// R_IRCallsiteIterator
//--------------------------------------------------------------------

R_IRCallsiteIterator::R_IRCallsiteIterator(StmtHandle _h)
  : m_annot(getProperty(ExpressionInfo, make_sexp(_h))),
    m_begin(m_annot->begin_call_sites()),
    m_end(m_annot->end_call_sites()),
    m_current(m_begin)
{
}

R_IRCallsiteIterator::~R_IRCallsiteIterator() {
}

CallHandle R_IRCallsiteIterator::current() const {
  return make_call_h(CAR(*m_current));
}

bool R_IRCallsiteIterator::isValid() const {
  return (m_current != m_end);
}

void R_IRCallsiteIterator::operator++() {
  ++m_current;
}

void R_IRCallsiteIterator::reset() {
  m_current = m_begin;
}

//--------------------------------------------------------------------
// R_IRProgramCallsiteIterator
//--------------------------------------------------------------------

R_IRProgramCallsiteIterator::R_IRProgramCallsiteIterator(StmtHandle _h)
  : m_annot(getProperty(ExpressionInfo, make_sexp(_h)))
{
  SEXP csi_c;
  // In the ExpressionInfo's call sites, collect only non-internal calls
  EXPRESSION_FOR_EACH_CALL_SITE(m_annot, csi_c) {
    if (is_var(call_lhs(CAR(csi_c))) && !is_library(call_lhs(CAR(csi_c)))) {
      m_program_call_sites.push_back(csi_c);
    }
  }
  m_current = m_program_call_sites.begin();
}

R_IRProgramCallsiteIterator::~R_IRProgramCallsiteIterator() {
}

CallHandle R_IRProgramCallsiteIterator::current() const {
  return make_call_h(CAR(*m_current));
}

bool R_IRProgramCallsiteIterator::isValid() const {
  return (m_current != m_program_call_sites.end());
}

void R_IRProgramCallsiteIterator::operator++() {
  ++m_current;
}

void R_IRProgramCallsiteIterator::reset() {
  m_current = m_program_call_sites.begin();
}


//----------------------------------------------------------------------
// R_ProcHandleIterator
//----------------------------------------------------------------------

// Basically a wrapper around FuncInfoIterator that fits the OA ProcHandleIterator interface

R_ProcHandleIterator::R_ProcHandleIterator(FuncInfo * fi) : m_fii(new FuncInfoIterator(fi)) {
}

R_ProcHandleIterator::~R_ProcHandleIterator() {
  delete m_fii;
}

ProcHandle R_ProcHandleIterator::current() const {
  return make_proc_h(m_fii->Current()->get_sexp());
}

bool R_ProcHandleIterator::isValid() const {
  return m_fii->IsValid();
}

void R_ProcHandleIterator::operator++() {
  ++(*m_fii);
}

void R_ProcHandleIterator::reset() {
  m_fii->Reset();
}

//------------------------------------------------------------
// R_ExpMemRefHandleIterator
//------------------------------------------------------------

R_ExpMemRefHandleIterator::R_ExpMemRefHandleIterator(ExpressionInfo * stmt)
  : m_stmt(stmt),
    m_use_iter(stmt->begin_uses()),
    m_def_iter(stmt->begin_defs())
{
}

MemRefHandle R_ExpMemRefHandleIterator::current() const {
  MemRefHandle handle;
  if (m_use_iter != m_stmt->end_uses()) {
    handle = make_mem_ref_h((*m_use_iter)->getMention_c());
  } else {
    handle = make_mem_ref_h((*m_def_iter)->getMention_c());
  }
  assert(handle != MemRefHandle(0));
  return handle;
}

bool R_ExpMemRefHandleIterator::isValid() const {
  return (!(m_use_iter == m_stmt->end_uses() &&
	    m_def_iter == m_stmt->end_defs()));
}

void R_ExpMemRefHandleIterator::operator++() {
  assert(isValid());
  if (m_use_iter != m_stmt->end_uses()) {
    ++m_use_iter;
  } else {
    ++m_def_iter;
  }
}

void R_ExpMemRefHandleIterator::reset() {
  m_use_iter = m_stmt->begin_uses();
  m_def_iter = m_stmt->begin_defs();
}

//------------------------------------------------------------
// R_ParamBindIterator
//------------------------------------------------------------

R_ParamBindIterator::R_ParamBindIterator(const SEXP call) {
  // fill in m_pairs
  int i = 0;
  SEXP arg_c = call_args(call);
  while (arg_c != R_NilValue) {
    if (!is_const(CAR(arg_c))) {
      m_pairs.push_back(std::pair<int, OA_ptr<MemRefExpr> >(i, MemRefExprInterface::convert_sexp_c(arg_c)));
    }
    arg_c = CDR(arg_c);
    i++;
  }
  m_iter = m_pairs.begin();
}

R_ParamBindIterator::~R_ParamBindIterator() {}

OA_ptr<MemRefExpr> R_ParamBindIterator::currentActual() const {
  return m_iter->second;
}

int R_ParamBindIterator::currentFormalId() const {
  return m_iter->first;
}

bool R_ParamBindIterator::isValid() const {
  return (m_iter != m_pairs.end());
}

void R_ParamBindIterator::operator++() {
  ++m_iter;
}


/// Create new set containing only the given mem ref; return an
/// iterator to the singleton set
OA_ptr<MemRefExprIterator> make_singleton_mre_iterator(OA_ptr<MemRefExpr> mre) {
  OA_ptr<set<OA_ptr<MemRefExpr> > > s; s = new set<OA_ptr<MemRefExpr> >();
  s->insert(mre);
  OA_ptr<MemRefExprIterator> iter; iter = new MemRefExprIterator(s);
  return iter;
}

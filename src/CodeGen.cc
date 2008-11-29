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

// File: CodeGen.cc
//
// A class with methods that output C code given R objects. Intended
// as a replacement for the ugliness in SubexpBuffer, but conversion
// is not yet complete.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "CodeGen.h"

#include <string>
#include <map>

#include <include/R/R_Defn.h>

#include <support/StringUtils.h>
#include <support/RccError.h>
#include <analysis/Utils.h>
#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <GetName.h>
#include <Macro.h>
#include <CodeGenUtils.h>
#include <ParseInfo.h>
#include <LoopContext.h>

#include <analysis/AnalysisResults.h>
#include <analysis/VarBinding.h>

using namespace RAnnot;

using namespace std;

// op_exp
//
// outputs a representation of the CAR of the given SEXP. Note that
// op_exp takes as the argument the CONS cell containing the
// expression instead of the expression itself; this allows op_var to
// use annotations. (Otherwise, we wouldn't be able to distinguish
// different mentions of the same variable.)

Output CodeGen::op_exp(SEXP cell, string rho,
		       bool fullyEvaluatedResult /* = false */)
{
  SEXP e = CAR(cell);
  switch(TYPEOF(e)) {
  case NILSXP:
    return Output::nil;
    break;
  case STRSXP:
    return op_string(e);
    break;
  case LGLSXP:
  case INTSXP:
  case REALSXP:
  case CPLXSXP:
    return op_vector(e);
    break;
  case VECSXP:
    ParseInfo::flag_problem();
    return Output::bogus;
    break;
  case SYMSXP:
    return op_var_use(cell, rho, Protected, fullyEvaluatedResult);
    break;
  case LISTSXP:
    return op_list(e, rho, false, false);
    break;
  case CLOSXP:
    return op_closure(e, rho);
#if 0
    {
      Output formals = op_list(FORMALS(e), rho, true);
      Output body = op_literal(BODY(e), rho);
      bool is_global = (formals.dependence() == CONST &&
			body.dependence() == CONST);
      if (is_global) {
	string clos = ParseInfo::global_constants->new_var();
	string call = emit_prot_assign(clos, emit_call3("mkCLOSXP",
							formals.handle(),
							body.handle(),
							rho));
	string g_decls = formals.g_decls() + body.g_decls() + emit_static_decl(clos);
	string g_code = formals.g_code() + body.g_code() + call;
	return Output::global(GDecls(g_decls), GCode(g_code), Handle(clos),INVISIBLE);
      } else {                              // dependent on environment
	string clos = m_scope.new_label();
	string call = emit_prot_assign(clos, emit_call3("mkCLOSXP",
							formals.handle(),
							body.handle(),
							rho));
	string g_decls = formals.g_decls() + body.g_decls();
	string g_code = formals.g_code() + body.g_code();
	string decls = formals.decls() + body.decls() + emit_decl(clos);
	string code = formals.code() + body.code() + call;
	string del_text = formals.del_text() + body.del_text() + emit_unprotect(clos);
	return Output::dependent(Decls(decls),
				 Code(code),
				 Handle(clos),
				 DelText(del_text),
				 INVISIBLE);
      }
    }
#endif
    break;
  case ENVSXP:
    ParseInfo::flag_problem();
    return Output::bogus;
    break;
  case PROMSXP:
    ParseInfo::flag_problem();
    return Output::bogus;
    break;
  case LANGSXP:
    return op_lang(e, rho);
    break;
  case CHARSXP:
    {
      string name = emit_call1("mkChar", quote(string(CHAR(e))));
      return Output::visible_const(Handle(name));
    }
    break;
  case SPECIALSXP:
  case BUILTINSXP:
    return op_primsxp(e, rho);
    break;
  default:
    ParseInfo::flag_problem();
    return Output::bogus;
    break;
  }
}

Output CodeGen::op_var_use(SEXP cell, string rho, 
			   Protection resultProtected, 
			   bool fullyEvaluatedResult)
{
  SEXP e = CAR(cell);
  string name = CHAR(PRINTNAME(e));
  if (e == R_MissingArg) {
    return Output::invisible_const(Handle("R_MissingArg"));
  } else {
    VarBinding * annot = getProperty(VarBinding, cell);
    if (annot->is_single()) {
      string location = annot->get_location(NULL, NULL);  // TODO: fix this
      string h = m_scope.new_label();
      string call = emit_assign(h, emit_call1("R_GetVarLocValue", location));
      return Output(Decls(emit_decl(h)),
		    Code(call),
		    GDecls(""),
		    GCode(""),
		    Handle(h),
		    DelText(""),
		    DEPENDENT,
		    VISIBLE);
    } else {
      string sym = make_symbol(e);
      string v = m_scope.new_label();
      string code = emit_assign(v, emit_call2("findVar", sym, rho));
      string decls = emit_decl(v);
      string del_text = "";
      if (fullyEvaluatedResult) {
	string v1 = m_scope.new_label();
	code += emit_prot_assign(v1, emit_call2("eval", v, rho));
	decls += emit_decl(v1);
	if (resultProtected == Protected) {
	  del_text += emit_unprotect(v1);
	}
	v = v1;
      }
      return Output::dependent(Decls(decls),
			       Code(code),
			       Handle(v),
			       DelText(del_text),
			       VISIBLE);
    }
  }
}

Output CodeGen::op_closure(SEXP e, string rho) {
  rcc_error("CodeGen::op_closure not yet implemented");
  return Output::bogus;
}

Output CodeGen::op_primsxp(SEXP e, string rho) {
  string var;
  int is_builtin;
  switch (TYPEOF(e)) {
  case SPECIALSXP:
    is_builtin = 0;
    break;
  case BUILTINSXP:
    is_builtin = 1;
    break;
  default:
    rcc_error("Internal error: op_primsxp called on non-(special or builtin)");
    is_builtin = 0; // silence the -Wall Who Cried "Uninitialized variable."
  }
  
  // unique combination of offset and is_builtin for memoization
  int value = 2 * PRIMOFFSET(e) + is_builtin;
  if (ParseInfo::primsxp_constant_exists(value)) {  // primsxp already defined
    return Output::invisible_const(Handle(ParseInfo::get_primsxp_constant(value)));
  } else {
    string var = ParseInfo::global_constants->new_var();
    const string args[] = {var,
			   i_to_s(PRIMOFFSET(e)),
			   i_to_s(is_builtin),
			   string(PRIMNAME(e))};
    string primsxp_def = mac_primsxp.call(4, args);
    ParseInfo::insert_primsxp_constant(value, var);
    return Output::global(GDecls(emit_static_decl(var)),
			  GCode(primsxp_def),
			  Handle(var),
			  INVISIBLE);
  }  
}

Output CodeGen::op_lang(SEXP e, string rho) {
  SEXP lhs = CAR(e);
  SEXP r_args = CDR(e);
  if (TYPEOF(lhs) == SYMSXP) {
    string r_sym = var_name(lhs);
    if (!is_library(lhs)) {
      // not in global env; presumably, user-defined function
      string func = m_scope.new_label();
      string code = emit_prot_assign(func, emit_call2("findFun",
						      make_symbol(lhs),
						      rho));
      Output out = op_clos_app(func, r_args, rho);
      return Output(Decls(out.decls()),
		    Code(out.code()),
		    GDecls(out.g_decls()),
		    GCode(out.g_code()),
		    Handle(out.handle()),
		    DelText(out.del_text() + emit_unprotect(func)),
		    DEPENDENT,
		    VISIBLE);
    } else {        // builtin function, special function, or library closure
      SEXP op = library_value(lhs);
      if (TYPEOF(op) == SPECIALSXP) {
	return op_special(e, op, rho);
      } else if (TYPEOF(op) == BUILTINSXP) {
	return op_builtin(e, op, rho);
      } else if (TYPEOF(op) == CLOSXP) {
	// generate code to look up the function
	string func = m_scope.new_label();
	string code = emit_prot_assign(func, emit_call2("findFun",
							make_symbol(lhs),
							rho));
	Output out = op_clos_app(func, r_args, rho);
	return Output(Decls(out.decls()),
		      Code(out.code()),
		      GDecls(out.g_decls()),
		      GCode(out.g_code()),
		      Handle(out.handle()),
		      DelText(out.del_text() + emit_unprotect(func)),
		      DEPENDENT,
		      VISIBLE);
      } else if (TYPEOF(op) == PROMSXP) {
	rcc_error("Hey! I don't think we should see a promise in LANGSXP!");
	return Output::bogus;
      } else {
	rcc_error("LANGSXP encountered non-function op");
	return Output::bogus;
      }
    }
  } else {     // LHS is not a symbol
    Output lhs = op_exp(e, rho, false);
    Output call = op_clos_app(lhs.handle(), CDR(e), rho);
    return Output(Decls(lhs.decls() + call.decls()),
		  Code(lhs.code() + call.code()),
		  GDecls(lhs.g_decls() + call.g_decls()),
		  GCode(lhs.g_code() + call.g_code()),
		  Handle(call.handle()),
		  DelText(lhs.del_text() + call.del_text()),
		  DEPENDENT, CHECK_VISIBLE);
		  
  }
}

Output CodeGen::op_promise(SEXP e) {
  rcc_error("CodeGen::op_promise not yet implemented");
  return Output::bogus;
}

Output CodeGen::op_begin(SEXP e, string rho) {
  if (e == R_NilValue) {
    return Output::nil;
  }
  string var = m_scope.new_label();
  string g_decls, g_code;
  string code;
  // output each one but the last expression.
  // Each one is in its own { }'s.
  while (CDR(e) != R_NilValue) {
    Output exp = op_exp(e, rho, false);
    code += emit_in_braces(exp.decls() + exp.code() + exp.del_text(),
			   false /* unbalanced */);
    g_decls += exp.g_decls();
    g_code += exp.g_code();
    e = CDR(e);
  }
  // output final expression
  Output exp = op_exp(e, rho, false);
  code += emit_in_braces(exp.decls() +
			 exp.code() +
			 emit_prot_assign(var, exp.handle()) +
			 exp.del_text(),
			 false /* unbalanced */);
  return Output(Decls(emit_decl(var)),
		Code(code),
		GDecls(g_decls),
		GCode(g_code),
		Handle(var),
		DelText(emit_unprotect(var)),
		exp.dependence(),
		exp.visibility());
}

Output CodeGen::op_if(SEXP e, string rho) {
  string out = m_scope.new_label();
  if (Rf_length(e) == 3) {  // just the one clause, no else
    Output cond = op_exp(if_cond_c(e), rho, false);
    Output t = op_exp(if_truebody_c(e), rho, false);
    string clause = "if (my_asLogicalNoNA(" + cond.handle() + ")) " +
      emit_in_braces(cond.del_text() +
		     t.decls() +
		     t.code() +
		     emit_prot_assign(out, t.handle()) +
		     Visibility::emit_set(t.visibility()),
		     false /* unbalanced */) +
      " else " +
      emit_in_braces(cond.del_text() +
		     Visibility::emit_set(INVISIBLE) +
		     emit_prot_assign(out, "R_NilValue"),
		     false /* unbalanced */);
    return Output(Decls(cond.decls()),
		  Code(cond.code()),
		  GDecls(cond.g_decls() + t.g_decls()),
		  GCode(cond.g_code() + t.g_code()),
		  Handle(out),
		  DelText(emit_unprotect(out)),
		  DEPENDENT,
		  CHECK_VISIBLE);
  } else if (Rf_length(e) == 4) {          // both true and false clauses
    Output cond = op_exp(if_cond_c(e), rho, false);
    Output te = op_exp(if_truebody_c(e), rho, false);
    Output fe = op_exp(if_falsebody_c(e), rho, false);
    string out = m_scope.new_label();
    string mac_args[] = {cond.handle(),
			 out,
			 te.code(),
			 te.handle(),
			 fe.code(),
			 fe.handle()};
    string code = mac_ifelse.call(6, mac_args);
    return Output(Decls(cond.decls() + te.decls() + fe.decls()),
		  Code(code),
		  GDecls(cond.g_decls() + te.g_decls() + fe.decls()),
		  GCode(cond.g_code() + te.g_code() + fe.g_code()),
		  Handle(out),
		  DelText(cond.del_text() + te.del_text() + fe.del_text() +
			  emit_unprotect(out)),
		  DEPENDENT, CHECK_VISIBLE);
  } else {
    rcc_error("Didn't understand form of if statement");
    return Output::bogus;
  }
}

Output CodeGen::op_for(SEXP e, string rho) {
  string decls = "";
  SEXP r_iv = CAR(for_iv_c(e));
  if (!is_var(r_iv)) rcc_error("non-symbol induction variable in for loop");

  Output range = op_exp(for_range_c(e), rho, false);
  decls += "int n;\n";
  decls += "SEXP ans, v;\n";
  decls += "PROTECT_INDEX vpi, api;\n";
  LoopContext thisLoop;
  string defs;
  defs += emit_call3("defineVar", make_symbol(r_iv), "R_NilValue", rho);
  defs += emit_call1("if", emit_call1("isList", range.handle()) + " || " + emit_call1("isNull", range.handle())) + " {";

  defs += indent("n = length(" + range.handle() + ");\n");
  defs += indent("PROTECT_WITH_INDEX(v = R_NilValue, &vpi);\n");
  defs += "} else {\n";
  defs += indent("n = LENGTH(" + range.handle() + ");\n");
  defs += indent("PROTECT_WITH_INDEX(v = allocVector(TYPEOF(" + range.handle() + "), 1), &vpi);\n");
  defs += "}\n";
  defs += "ans = R_NilValue;\n";
  defs += "PROTECT_WITH_INDEX(ans, &api);\n";
  defs += "for (i=0; i < n; i++) {\n";
  
  rcc_error("CodeGen::op_for not yet implemented");
  return Output::bogus;
}

Output CodeGen::op_while(SEXP e, string rho) {
  rcc_error("CodeGen::op_while not yet implemented");
  return Output::bogus;
}

Output CodeGen::op_return(SEXP e, string rho) {
  rcc_error("CodeGen::op_return not yet implemented");
  return Output::bogus;
}

Output CodeGen::op_break(SEXP e, string rho) {
  LoopContext *loop = LoopContext::Top(); 
  string code;
  if (Rf_install("next") == e)
    code = "continue;\n";
  else
    code = loop->doBreak() + ";\n";
  return Output::dependent(Decls(""),
			   Code(code),
			   Handle("R_NilValue"),
			   DelText(""),
			   INVISIBLE);
}

Output CodeGen::op_fundef(SEXP e, string rho, string opt_R_name /* = "" */) {
  rcc_error("CodeGen::op_fundef not yet implemented");
  return Output::bogus;
}

Output CodeGen::op_special(SEXP e, SEXP op, string rho) {
  rcc_error("CodeGen::op_special not yet implemented");
  return Output::bogus;
}

Output CodeGen::op_builtin(SEXP e, SEXP op, string rho) {
  string out, decls, code, g_decls, g_code;

  VisibilityType op_vis = 1 - PRIMPRINT(op) ? VISIBLE : INVISIBLE;

  Output op1 = op_primsxp(op, rho);
  SEXP args = CDR(e);
  // special case for arithmetic operations
  if (PRIMFUN(op) == (CCODE)do_arith) {

    // R_unary if there's one argument and it's a non-object
    if (Rf_length(args) == 1
	&& !Rf_isObject(CAR(args))) {
      Output x = op_exp(args, rho, true);
      out = m_scope.new_label();
      code = 
	emit_prot_assign(out, emit_call3("R_unary",
					 "R_NilValue",
					 op1.handle(),
					 x.handle()));
      return Output(Decls(op1.decls() + emit_decl(out)),
		    Code(op1.code() + code),
		    GDecls(op1.g_decls() + x.g_decls()),
		    GCode(op1.g_code() + x.g_code()),
		    Handle(out),
		    DelText(op1.del_text() +
			    x.del_text() +
			    emit_unprotect(out)),
		    DEPENDENT,
		    op_vis);

    // R_binary if two non-object arguments
    } else if (Rf_length(args) == 3 &&
	       !Rf_isObject(CAR(args)) &&
	       !Rf_isObject(CADR(args))) {
      Output x = op_exp(args, rho, true);
      Output y = op_exp(CDR(args), rho, true);
      out = m_scope.new_label();
      code = emit_prot_assign(out, emit_call4("R_binary",
					      "R_NilValue",
					      op1.handle(),
					      x.handle(),
					      y.handle()));
      return Output(Decls(op1.decls() + emit_decl(out)),
		    Code(op1.code() + code),
		    GDecls(op1.g_decls() + x.g_decls() + y.g_decls()),
		    GCode(op1.g_code() + x.g_code() + y.g_code()),
		    Handle(out),
		    DelText(op1.del_text() +
			    x.del_text() +
			    y.del_text() +
			    emit_unprotect(out)),
		    DEPENDENT,
		    op_vis);
    }
  } else {  // common case: call the do_ function
    Output args1 = op_list(args, rho, false, true);
    out = m_scope.new_label();
    code = emit_prot_assign(out, emit_call4(get_name(PRIMOFFSET(op)),
					    "R_NilValue ",
					    op1.handle(),
					    args1.handle(),
					    rho));
    return Output(Decls(op1.decls() + emit_decl(out)),
		  Code(op1.code() + code),
		  GDecls(op1.g_decls() + args1.g_decls()),
		  GCode(op1.g_code() + args1.g_code()),
		  Handle(out),
		  DelText(op1.del_text() +
			  args1.del_text() +
			  emit_unprotect(out)),
		  DEPENDENT,
		  op_vis);
  }
  return Output::bogus;
}

Output CodeGen::op_set(SEXP e, SEXP op, string rho) {
  rcc_error("CodeGen::op_set not yet implemented");
  return Output::bogus;
}

Output CodeGen::op_subscriptset(SEXP e, string rho) {
  rcc_error("CodeGen::op_subscriptset not yet implemented");
  return Output::bogus;
}

Output CodeGen::op_clos_app(string clos_h, SEXP args, string rho) {
  rcc_error("CodeGen::op_clos_app not yet implemented");
  return Output::bogus;
}

#if 0
Output CodeGen::op_arglist(SEXP e, string rho) {
  int len = Rf_length(e);
  if (len == 0) return Output::nil;
  SEXP *args = new SEXP[len];

  SEXP arg = e;
  for(int i=0; i<len; i++) {
    args[i] = arg;
    arg = CDR(arg);
  }

  CScope tmp_scope("tmp_arglist");

  // Construct the list iterating backwards through the list
  // Don't unprotect R_NilValue, just the conses
  string tmp = tmp_scope.new_label();
  string tmp_decls = emit_decl(tmp);
  string tmp_code = emit_prot_assign(tmp, emit_call2("cons", make_symbol(TAG(args[len-1])), "R_NilValue"));
  if (len > 1) {
    for(int i=len-2; i>=0; i--) {
      string tmp1 = tmp_scope.new_label();
      tmp_decls += emit_decl(tmp1);
      tmp_code += emit_prot_assign(tmp1, emit_call2("cons", make_symbol(TAG(args[i])), tmp));
      tmp_code += emit_unprotect(tmp);
      tmp = tmp1;
    }
  }

  delete [] args;
  string out = m_scope.new_label();
  string final_code = emit_in_braces(tmp_decls + tmp_code + emit_assign(out, tmp));
  return Output::dependent(Decls(emit_decl(out)),
			   Code(final_code),
			   Handle(out),
			   DelText(emit_unprotect(out)),
			   INVISIBLE);
}
#endif

Output CodeGen::op_literal(SEXP e, string rho) {
  string v;
  switch(TYPEOF(e)) {
  case NILSXP:
    return Output::nil;
    break;
  case STRSXP:
    return op_string(e);
    break;
  case LGLSXP: case REALSXP:
    return op_vector(e);
    break;
  case SYMSXP:
    return Output::visible_const(Handle(make_symbol(e)));
    break;
  case LISTSXP:
  case LANGSXP:
    return op_list(e, rho, true);
    break;
  case CHARSXP:
    return Output::visible_const(Handle(emit_call1("mkChar", quote(string(CHAR(e))))));
    break;
  case SPECIALSXP:
  case BUILTINSXP:
    return op_primsxp(e, rho);
    break;
  default:
    ParseInfo::flag_problem();
    return Output::visible_const(Handle("<<unimplemented type " + i_to_s(TYPEOF(e)) + ">>"));
    break;
  }
}

Output CodeGen::op_list(SEXP e,
			string rho,
			bool literal,
			bool fullyEvaluatedResult /* = false */)
{
  int i;
  if (e == R_NilValue) return Output::nil;
  int len = Rf_length(e);
  string my_cons;
  if (len == 1) {
    // one element
    switch (TYPEOF(e)) {
    case LISTSXP:
      my_cons = "cons";
      break;
    case LANGSXP:
      my_cons = "lcons";
      break;
    default:
      rcc_error("non-list passed to CodeGen::op_list");
      return Output::bogus;
    }

    Output car = (literal ? op_literal(CAR(e), rho) : op_exp(e, rho, fullyEvaluatedResult));
    string code;
    string var = m_scope.new_label();
    if (TAG(e) == R_NilValue) {
      // No tag; regular cons or lcons
      code = emit_prot_assign(var, emit_call2(my_cons, car.handle(), "R_NilValue"));
      if (car.dependence() == CONST) {
	return Output::global(GDecls(car.g_decls() + emit_static_decl(var)),
			      GCode(car.g_code() + code),
			      Handle(var),
			      VISIBLE);
      } else {
	return Output::dependent(Decls(car.decls() + emit_decl(var)),
				 Code(car.code() + code),
				 Handle(var),
				 DelText(car.del_text() + unp(var)),
				 VISIBLE);
      }
    } else {
      // Tag exists; need car, cdr, and tag
      Output tag = op_literal(TAG(e), rho);
      code = emit_prot_assign(var, emit_call3("tagged_cons", car.handle(), tag.handle(), "R_NilValue"));
      if (car.dependence() == CONST && tag.dependence() == CONST) {
	return Output::global(GDecls(car.g_decls() + tag.g_decls() + emit_static_decl(var)),
			      GCode(car.g_code() + tag.g_code() + code),
			      Handle(var),
			      VISIBLE);
      } else {
	return Output::dependent(Decls(car.decls() + tag.decls() + emit_decl(var)),
				 Code(car.code() + tag.code()),
				 Handle(var),
				 DelText(car.del_text() + tag.del_text()),
				 VISIBLE);
      }
    }
  } else {    // two or more elements
    bool * langs; langs = (bool *)malloc(len * sizeof(bool));
    bool * tagged; tagged = (bool *)malloc(len * sizeof(bool));
    Output *cars = new Output[len];
    Output *tags = new Output[len];
    
    // collect Output from CAR and TAG of each item in the list
    SEXP tmp_e = e;
    for(i=0; i<len; i++) {
      assert(is_cons(tmp_e));
      langs[i] = (TYPEOF(tmp_e) == LANGSXP);  // whether LANGSXP or LISTSXP
      cars[i] = (literal ? op_literal(CAR(tmp_e), rho) : op_exp(tmp_e, rho, fullyEvaluatedResult));
      if (TAG(tmp_e) == R_NilValue) {
	tagged[i] = false;
	tags[i] = Output::nil;
      } else {
	tagged[i] = true;
	tags[i] = op_literal(TAG(tmp_e), rho);
      }
      tmp_e = CDR(tmp_e);
    }

    // list is constant only if everything in it is constant
    DependenceType list_dependence = CONST;
    for(i=0; i<len; i++) {
      if (cars[i].dependence() == DEPENDENT
	  || tags[i].dependence() == DEPENDENT) {
	list_dependence = DEPENDENT;
      }
    }

    // collect code for list elements
    string decls = "";
    string code = "";
    string g_decls = "";
    string g_code = "";
    string del_text = "";

    // collect list elements
    for(i=0; i<len; i++) {
      if (list_dependence == DEPENDENT) {
	decls    += cars[i].decls()    + tags[i].decls();
	code     += cars[i].code()     + tags[i].code();
	g_decls  += cars[i].g_decls()  + tags[i].g_decls();
	g_code   += cars[i].g_code()   + tags[i].g_code();
	del_text += cars[i].del_text() + tags[i].del_text();
      } else {
	// list is constant
	g_decls += cars[i].g_decls() + tags[i].g_decls();
	g_code  += cars[i].g_code()  + tags[i].g_code();
      }
    }

    // temporary scope for intermediate list elements
    CScope temp_list_elements("tmp_ls");

    // collect code for list construction inside curly braces
    string buf_decls = "";
    string buf_code = "";

    // iterate backwards to build list from element handles
    string rhs = "R_NilValue";
    string lhs = temp_list_elements.new_label();
    buf_decls += emit_decl(lhs);
    for(i=len-1; i>=0; i--) {
      string call;
      if (tagged[i]) {
	call = emit_call3("tagged_cons", cars[i].handle(), tags[i].handle(), rhs);
      } else {
	call = emit_call2((langs[i] ? "lcons" : "cons"), cars[i].handle(), rhs);
      }
      buf_code += emit_assign(lhs, call);
      rhs = lhs;
    }

    free(langs);
    free(tagged);
    delete [] cars;
    delete [] tags;

    string outside_handle = "";
    string out_assign = "";
    // connect list in buffer to other code
    if (list_dependence == DEPENDENT) {
      outside_handle = m_scope.new_label();
      out_assign = emit_prot_assign(outside_handle, lhs);
      string braces_exp = buf_decls + buf_code + out_assign;
      braces_exp = emit_in_braces(braces_exp, false /* unbalanced */);
      return Output(Decls(decls + emit_decl(outside_handle)),
		    Code(code + braces_exp),
		    GDecls(g_decls),
		    GCode(g_code),
		    Handle(outside_handle),
		    DelText(del_text + unp(outside_handle)),
		    DEPENDENT, VISIBLE);
    } else {                               // list is constant
      outside_handle = m_scope.new_label();
      out_assign = emit_prot_assign(outside_handle, lhs);
      return Output::global(GDecls(g_decls + emit_static_decl(outside_handle)),
			    GCode(g_code
				  + emit_in_braces(buf_decls + buf_code + 
						   out_assign, 
						   false /* unbalanced */)),
			    Handle(outside_handle),
			    VISIBLE);
    }
  }
}

Output CodeGen::op_string(SEXP s) {
  //  TODO: use ParseInfo::string_map
  int i, len;
  string str = "";
  len = Rf_length(s);
  for(i=0; i<len; i++) {
    str += string(CHAR(STRING_ELT(s, i)));
  }
  string out = ParseInfo::global_constants->appl1("mkString", "", 
						   quote(escape(str)));
  return Output::visible_const(Handle(out));
}

Output CodeGen::op_vector(SEXP vec) {
  int len = Rf_length(vec);
  if (len != 1) {
    rcc_error("unexpected non-scalar vector encountered");
    return Output::bogus;
  }
  int value;
  switch(TYPEOF(vec)) {
  case LGLSXP:
    {
      int value = INTEGER(vec)[0];
      if (!ParseInfo::logical_constant_exists(value)) {
	string var = ParseInfo::global_constants->new_var();
	ParseInfo::insert_logical_constant(value, var);
	Output op = Output::global(GDecls(emit_static_decl(var)),
				   GCode(emit_prot_assign(var, emit_call1("ScalarLogical", i_to_s(value)))),
				   Handle(var), VISIBLE);
	return op;
      } else {
	Output op(Decls(""), Code(""),
		  GDecls(""), GCode(""),
		  Handle(ParseInfo::get_logical_constant(value)), DelText(""), CONST, VISIBLE);
	return op;
      }
    }
    break;
  case REALSXP:
    {
      double value = REAL(vec)[0];
      if (!ParseInfo::real_constant_exists(value)) {
	string var = ParseInfo::global_constants->new_var();
	ParseInfo::insert_real_constant(value, var);
	Output op(Decls(""), Code(""),
		  GDecls(emit_static_decl(var)),
		  GCode(emit_prot_assign(var, emit_call1("ScalarReal", d_to_s(value)))),
		  Handle(var), DelText(""), CONST, VISIBLE);
	return op;
      } else {
	Output op(Decls(""), Code(""),
		  GDecls(""), GCode(""),
		  Handle(ParseInfo::get_real_constant(value)), DelText(""), CONST, VISIBLE);
	return op;
      }
    }
    break;
  default:
    rcc_error("Unhandled type in op_vector");
    return Output::bogus;
  }
}

const CScope CodeGen::m_scope("vv");

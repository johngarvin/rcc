/* Copyright (c) 2003-2005 John Garvin 
 *
 * July 11, 2003 
 *
 * Parses R code, turns into C code that uses internal R functions.
 * Attempts to output some code in regular C rather than using R
 * functions.
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#include "rcc.h"
#include "codegen.h"
#include <iostream>

// Settings to change how rcc works
static bool global_self_allocate = FALSE;
static bool output_main_program = TRUE;
static bool output_default_args = TRUE;
static bool global_c_return = FALSE;

static bool global_ok = TRUE;
static unsigned int global_temps = 0;
static map<string, string> func_map;
static map<string, string> symbol_map;
static map<double, string> sc_real_map;
static map<int, string> sc_logical_map;
static map<int, string> sc_integer_map;
static map<int, string> primsxp_map;
static map<string, bool> vis_map;
static list<string> direct_funcs;
static SubexpBuffer global_fundefs;
static SplitSubexpBuffer global_constants("c", TRUE);
static SubexpBuffer global_labels("l");
static Expression bogus_exp = Expression("BOGUS", FALSE, INVISIBLE, "");
static Expression nil_exp = Expression("R_NilValue", FALSE, INVISIBLE, "");





// Returns true if the given string represents a function specified
// for direct calling.
bool is_direct(string func) {
  list<string>::iterator i;
  for(i=direct_funcs.begin(); i!=direct_funcs.end(); i++) {
    if (*i == func) return TRUE;
  }
  return FALSE;
}

Expression SubexpBuffer::op_exp(SEXP e, string rho, bool primFuncArg) {
  string sym, var;
  Expression out, formals, body, env;
  switch(TYPEOF(e)) {
  case NILSXP:
    return nil_exp;
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
    global_ok = 0;
    return Expression("<<unimplemented vector>>", TRUE, INVISIBLE, "");
    break;
  case SYMSXP:
    if (e == R_MissingArg) {
      return Expression("R_MissingArg", FALSE, INVISIBLE, "");
    } else {
      sym = make_symbol(e);
      string v = appl2_unp("findVar", sym, rho);
      if (primFuncArg) v = appl2("eval", v, rho);
      out = Expression(v, TRUE, VISIBLE, primFuncArg ? unp(v) : "");
      return out;
    }
    break;
  case LISTSXP:
    return op_list(e, rho, FALSE);
    break;
  case CLOSXP:
    formals = op_symlist(FORMALS(e), rho);
    body = op_literal(BODY(e), rho);
    if (rho == "R_GlobalEnv" && !formals.is_dep && !body.is_dep) {
      string v = global_constants.appl3("mkCLOSXP",
					formals.var,
					body.var,
					rho);
      global_constants.del(formals);
      global_constants.del(body);
      out = Expression(v, FALSE, INVISIBLE, "");
    } else {
      string v = appl3("mkCLOSXP",
		       formals.var,
		       body.var,
		       rho);
      del(formals);
      del(body);
      out = Expression(v, TRUE, INVISIBLE, unp(v));
    }
    return out;
    break;
  case ENVSXP:
    global_ok = 0;
    return Expression("<<unexpected environment>>", TRUE, INVISIBLE, "");
    break;
  case PROMSXP:
    global_ok = 0;
    return Expression("<<unexpected promise>>", TRUE, INVISIBLE, "");
    break;
  case LANGSXP:
    out = op_lang(e, rho);
    return out;
    break;
  case CHARSXP:
    return Expression(appl1_unp("mkChar",
				quote(string(CHAR(e)))),
		      FALSE, VISIBLE, "");
    break;
  case SPECIALSXP:
  case BUILTINSXP:
    return global_constants.op_primsxp(e, rho);
  case EXPRSXP:
  case EXTPTRSXP:
  case WEAKREFSXP:
    global_ok = 0;
    return Expression("<<unimplemented type " + i_to_s(TYPEOF(e)) + ">>",
		      TRUE, INVISIBLE, "");
    break;
  default:
    err("Internal error: op_exp encountered bad type\n");
    return bogus_exp; // never reached
    break;
  }
}

Expression SubexpBuffer::op_primsxp(SEXP e, string rho) {
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
    err("Internal error: op_primsxp called on non-(special or builtin)");
    is_builtin = 0; // silence the -Wall Who Cried "Uninitialized variable."
  }
  
  // unique combination of offset and is_builtin for memoization
  int value = 2 * PRIMOFFSET(e) + is_builtin;
  map<int,string>::iterator pr = primsxp_map.find(value);
  if (pr != primsxp_map.end()) {  // primsxp already defined
    return Expression(pr->second, TRUE, INVISIBLE, "");
  } else {
    string var = new_sexp();
    const string args[] = {var, i_to_s(PRIMOFFSET(e)),
			   i_to_s(is_builtin), string(PRIMNAME(e))};
    append_defs(mac_primsxp.call(4, args));
    primsxp_map.insert(pair<int,string>(value, var));
    return Expression(var, TRUE, INVISIBLE, "");
  }
}

Expression SubexpBuffer::op_symlist(SEXP e, string rho) {
  return op_list(e, rho, TRUE);
}

Expression SubexpBuffer::op_promise(SEXP e) {
  Expression args; 
  string defs = args.var + " = forcePromise(" + args.var + ");\n" + "SET_NAMED(" + args.var + ", 2);\n";
  append_defs(defs);
  return Expression(args.var, TRUE, INVISIBLE, "");
}


Expression SubexpBuffer::op_lang(SEXP e, string rho) {
  SEXP op;
  string out;
  Expression exp;
  if (TYPEOF(CAR(e)) == SYMSXP) {
    string r_sym = CHAR(PRINTNAME(CAR(e)));

    if (is_direct(r_sym)) {
      //direct function call
      string func = make_c_id(r_sym) + "_direct";
      Expression args = op_list(CDR(e), rho, FALSE); // not local; used for env
      string call = appl1(func, args.var);
      del(args);
      return Expression(call, TRUE, VISIBLE, unp(call));
    } else { // not direct; call via closure
      op = findFunUnboundOK(CAR(e), R_GlobalEnv, TRUE);
      if (op == R_UnboundValue) {    // user-defined function
	string func = appl2("findFun",
			    make_symbol(CAR(e)),
			    rho);
	return op_clos_app(Expression(func, FALSE, INVISIBLE, unp(func)),
			   CDR(e), rho);

      } else {  // Built-in function, special function, or closure
	if (TYPEOF(op) == SPECIALSXP) {
	  return op_special(e, op, rho);
	} else if (TYPEOF(op) == BUILTINSXP) {
	  return op_builtin(e, op, rho);
	} else if (TYPEOF(op) == CLOSXP) {
	  Expression op1;
	  if (CAR(BODY(op)) == install(".Internal")) {
	    op1 = op_exp(op, "R_GlobalEnv");
	  } else {
	    op1 = op_exp(op, rho);
	  }
	  Expression out_exp = op_clos_app(op1, CDR(e), rho);
	  return out_exp;
	} else if (TYPEOF(op) == PROMSXP) {
	  // ***************** TEST ME! ******************
	  err("Hey! I don't think we should see a promis in  LANGSXP!\n");
	  return op_exp(PREXPR(op), rho);
	} else {
	  err("Internal error: LANGSXP encountered non-function op\n");
	  return bogus_exp; // never reached
	}
      }
    }
  } else {  // function is not a symbol
    Expression op1;
    op1 = op_exp(CAR(e), rho);
    Expression out_exp = op_clos_app(op1, CDR(e), rho);
    return out_exp;
    // eval.c: 395
  }
}

Expression SubexpBuffer::op_begin(SEXP exp, string rho) {
  Expression e;
  string var = new_sexp();
  while (exp != R_NilValue) {
    SubexpBuffer temp = new_sb("tmp_be_" + i_to_s(global_temps++) + "_");
    //    temp.encl_fn = this;
    e = temp.op_exp(CAR(exp), rho);
    append_defs(string("{\n") + indent(temp.output()));
    if (CDR(exp) == R_NilValue) {
      append_defs(indent(var + " = " + e.var + ";\n"));
    } else {
      del(e);
    }
    append_defs("}\n");
    exp = CDR(exp);
  }
  if (e.del_text.empty()) {
    return Expression(var, e.is_dep, e.is_visible, "");
  } else if (e.del_text == "UNPROTECT_PTR(" + e.var + ");\n") {
    return Expression(var, e.is_dep, e.is_visible, unp(var));
  } else {
    err("Encountered unhandled op_begin delete-text");
    return bogus_exp;
  }
}
  
Expression SubexpBuffer::op_if(SEXP e, string rho) {
  if (Rf_length(e) > 2) {
#if 1
    Expression cond = op_exp(CAR(e), rho);
    string out = new_sexp();
    append_defs("if (my_asLogicalNoNA(" + cond.var + ")) {\n");
    del(cond);
    Expression te = op_exp(CADR(e), rho);
    append_defs(indent("PROTECT(" + out + " = " + te.var + ");\n"));
    del(te);
    append_defs(indent(Visibility::emit_set(te.is_visible)));
    append_defs("} else {\n");
    del(cond);
    Expression fe = op_exp(CADDR(e), rho);
    append_defs(indent("PROTECT(" + out + " = " + fe.var + ");\n"));
    del(fe);
    append_defs(indent(Visibility::emit_set(fe.is_visible)));
    append_defs("}\n");
    //del(cond);
    return Expression(out, FALSE, CHECK_VISIBLE, unp(out));
#else
    Expression cond = op_exp(CAR(e), rho);
    Expression te = op_exp(CADR(e), rho);
    Expression fe = op_exp(CADDR(e), rho);
    string out = new_sexp();
    append_defs(mac_ifelse.call(6,
			    cond.var,
			    out,
			    te.text,
			    te.var,
			    fe.text,
			    fe.var));
#endif
  } else {
    Expression cond = op_exp(CAR(e), rho);
    string out = new_sexp();
    append_defs("if (my_asLogicalNoNA(" + cond.var + ")) {\n");
    del(cond);
    Expression te = op_exp(CADR(e), rho);
    append_defs(indent("PROTECT(" + out + " = " + te.var + ");\n"));
    del(te);
    append_defs(indent(Visibility::emit_set(te.is_visible)));
    append_defs("} else {\n");
    del(cond);
    append_defs(indent(Visibility::emit_set(INVISIBLE)));
    append_defs(indent("PROTECT(" + out + " = R_NilValue);\n") + "}\n");
    return Expression(out, FALSE, CHECK_VISIBLE, unp(out));
  }
}

Expression SubexpBuffer::op_for(SEXP e, string rho) {
  SEXP sym, body, val;
  sym = CAR(e);
  val = CADR(e);
  body = CADDR(e);
  if ( !isSymbol(sym) ) err("non-symbol loop variable\n");
  Expression val1 = op_exp(val, rho);
  decls += "int n;\n";
  decls += "SEXP ans, v;\n";
  decls += "PROTECT_INDEX vpi, api;\n";
  decls += "RCNTXT cntxt;\n";
  if (!has_i) {
    decls += "int i;\n";
    has_i = TRUE;
  }
  string defs;
  defs += "defineVar(" + make_symbol(sym) + ", R_NilValue, " + rho + ");\n";
  defs += "if (isList(" + val1.var + ") || isNull(" + val1.var + ")) {\n";
  defs += indent("n = length(" + val1.var + ");\n");
  defs += indent("PROTECT_WITH_INDEX(v = R_NilValue, &vpi);\n");
  defs += "} else {\n";
  defs += indent("n = LENGTH(" + val1.var + ");\n");
  defs += indent("PROTECT_WITH_INDEX(v = allocVector(TYPEOF(" + val1.var + "), 1), &vpi);\n");
  defs += "}\n";
  defs += "ans = R_NilValue;\n";
  defs += "PROTECT_WITH_INDEX(ans, &api);\n";
  defs += "begincontext(&cntxt, CTXT_LOOP, R_NilValue, " + rho
    + ", R_NilValue, R_NilValue, R_NilValue);\n";
  defs += "switch (SETJMP(cntxt.cjmpbuf)) {\n";
  string lab = global_labels.new_var();
  defs += indent("case CTXT_BREAK: goto for_break_" + lab + ";\n");
  defs += indent("case CTXT_NEXT: goto for_next_" + lab + ";\n");
  defs += "}\n";
  defs += "for (i=0; i < n; i++) {\n";
  string in_loop;
  in_loop += "switch(TYPEOF(" + val1.var + ")) {\n";
  in_loop += "case LGLSXP:\n";
  in_loop += indent("REPROTECT(v = allocVector(TYPEOF(" + val1.var + "), 1), vpi);\n");
  in_loop += indent("LOGICAL(v)[0] = LOGICAL(" + val1.var + ")[i];\n");
  in_loop += indent("setVar(" + make_symbol(sym) + ", v, " + rho + ");\n");
  in_loop += indent("break;\n");
  in_loop += "case INTSXP:\n";
  in_loop += "REPROTECT(v = allocVector(TYPEOF(" + val1.var + "), 1), vpi);\n";
  in_loop += "INTEGER(v)[0] = INTEGER(" + val1.var + ")[i];\n";
  in_loop += "setVar(" + make_symbol(sym) + ", v, " + rho + ");\n";
  in_loop += indent("break;\n");
  in_loop += "case REALSXP:\n";
  in_loop += "REPROTECT(v = allocVector(TYPEOF(" + val1.var + "), 1), vpi);\n";
  in_loop += "REAL(v)[0] = REAL(" + val1.var + ")[i];\n";
  in_loop += "setVar(" + make_symbol(sym) + ", v, " + rho + ");\n";
  in_loop += indent("break;\n");
  in_loop += "case CPLXSXP:\n";
  in_loop += "REPROTECT(v = allocVector(TYPEOF(" + val1.var + "), 1), vpi);\n";
  in_loop += "COMPLEX(v)[0] = COMPLEX(" + val1.var + ")[i];\n";
  in_loop += "setVar(" + make_symbol(sym) + ", v, " + rho + ");\n";
  in_loop += indent("break;\n");
  in_loop += "case STRSXP:\n";
  in_loop += "REPROTECT(v = allocVector(TYPEOF(" + val1.var + "), 1), vpi);\n";
  in_loop += "SET_STRING_ELT(v, 0, STRING_ELT(" + val1.var + ", i));\n";
  in_loop += "setVar(" + make_symbol(sym) + ", v, " + rho + ");\n";
  in_loop += indent("break;\n");
  in_loop += "case EXPRSXP:\n";
  in_loop += "case VECSXP:\n";
  in_loop += "setVar(" + make_symbol(sym) + ", VECTOR_ELT(" + val1.var + ", i), " + rho + ");\n";
  in_loop += indent("break;\n");
  in_loop += "case LISTSXP:\n";
  in_loop += "setVar(" + make_symbol(sym) + ", CAR(" + val1.var + "), " + rho + ");\n";
  in_loop += val1.var + " = CDR(" + val1.var + ");\n";
  in_loop += indent("break;\n");
  in_loop += "default: errorcall(R_NilValue, \"Bad for loop sequence\");\n";
  in_loop += "}\n";
  defs += indent(in_loop);
  append_defs(defs);
  Expression ans = op_exp(body, rho);
  append_defs("REPROTECT(ans = " + ans.var + ", api);\n");
  del(ans);
  defs = "for_next_" + lab + ":\n";
  defs += ";\n";
  defs += "}\n";
  defs += "for_break_" + lab + ":\n";
  defs += "endcontext(&cntxt);\n";
  append_defs(defs);
  del(val1);
  append_defs("UNPROTECT_PTR(v);\n");
  return Expression(ans.var, TRUE, INVISIBLE, unp(ans.var));
}

Expression SubexpBuffer::op_while(SEXP e, string rho) {
  return bogus_exp;
}

Expression SubexpBuffer::op_c_return(SEXP e, string rho) {
  Expression value;
  switch(Rf_length(e)) {
  case 0:
    append_defs("UNPROTECT_PTR(newenv);\n");
    append_defs("return R_NilValue;\n");
    break;
  case 1:
    value = op_exp(CAR(e), rho);
    append_defs("UNPROTECT_PTR(newenv);\n");
    del(value);
    append_defs("return " + value.var + ";\n");
    break;
  default:
    SEXP exp = e;
    // set names for multiple return
    while(exp != R_NilValue) {
      if (TYPEOF(CAR(exp)) == SYMSXP) {
	SET_TAG(exp, CAR(exp));
      }
      exp = CDR(exp);
    }
    value = op_list(e, rho, FALSE);
    string retval = appl1_unp("PairToVectorList", value.var);
    append_defs("UNPROTECT_PTR(newenv);\n");
    del(value);
    append_defs("return " + retval + ";\n");
    break;
  }
  return Expression("R_NilValue", TRUE, INVISIBLE, "");
}

Expression SubexpBuffer::op_fundef(SEXP e, string rho,
				   string opt_R_name /* = "" */) {
  bool direct = FALSE;
  if (!opt_R_name.empty() && is_direct(opt_R_name)) {
    direct = TRUE;
    // make function to be called directly
    if (func_map.find(opt_R_name) != func_map.end()) { // defined already
      string closure_name = func_map.find(opt_R_name)->second;
      return Expression(closure_name, FALSE, INVISIBLE, "");
    } else { // not yet defined
      // direct version
      if (rho != "R_GlobalEnv") {
	cerr << "Warning: function " << opt_R_name.c_str() << " is not in global scope; unable to make direct function call\n";
      } else {
	global_c_return = TRUE;
	global_fundefs.append_defs( 
	  make_fundef_argslist_c(this,
				 make_c_id(opt_R_name) + "_direct",
				 CAR(e),
				 CADR(e)));
	global_c_return = FALSE;
      }
      // in any case, continue to closure version
    }
  }
  // closure version
  string func_name = global_fundefs.new_var();
  global_fundefs.append_defs(make_fundef_argslist(this,
					      func_name,
					      CAR(e),
					      CADR(e)));
  Expression formals = op_literal(CAR(e), rho);
  if (rho == "R_GlobalEnv") {
    Expression r_args = global_constants.op_list(CAR(e), rho, TRUE);
    Expression r_code = global_constants.op_literal(CADR(e), rho);
#if 0
    string r_form = global_constants.appl3("mkCLOSXP", r_args.var, r_code.var, rho);
    Expression c_f_args = global_constants.op_arglist(CAR(e), rho);
    string c_args = global_constants.appl2("cons", rho, c_f_args.var);
    string c_call = global_constants.appl2("cons", func_sym, c_args);
    string r_call = global_constants.appl2("lcons",
			  make_symbol(install(".External")),
			  c_call);
    string c_clos = global_constants.appl3("mkCLOSXP ", formals.var,
				   func_name, rho);
#endif
    string r_form = 
      global_constants.appl4("mkRCC_CLOSXP", r_args.var, func_name, r_code.var, rho);
#if 0
    global_constants.append_defs("setAttrib(" + r_form
          + ", install(\"RCC_CompiledSymbol\"), " + c_clos + ");\n");
#endif
    global_constants.del(formals);
    if (direct) func_map.insert(pair<string,string>(opt_R_name, r_form));
    return Expression(r_form, FALSE, INVISIBLE, "");
  } else {
    Expression r_args = op_literal(CAR(e), rho);
    Expression r_code = op_literal(CADR(e), rho);
    string r_form = appl4("mkRCC_CLOSXP", r_args.var, func_name, r_code.var, rho);
    del(r_args);
    del(r_code);
#if 0
    Expression c_f_args = op_arglist(CAR(e), rho);
    string c_args = appl2("cons", rho, c_f_args.var);
    del(c_f_args);
    string c_call = appl2("cons", func_sym, c_args);
    string defs = unp(c_args);
    string r_call = appl2("lcons",
			  make_symbol(install(".External")),
			  c_call);
    defs += unp(c_call);
    string out = appl3("mkCLOSXP ", formals.var, r_call, rho);
    del(formals);
    defs += unp(r_call);
    defs += "setAttrib(" + r_form
          + ", install(\"RCC_CompiledSymbol\"), " + out + ");\n";
    defs += unp(out);
    append_defs(defs);
#endif
    return Expression(r_form, FALSE, CHECK_VISIBLE, unp(r_form));
  }
}

Expression SubexpBuffer::op_special(SEXP e, SEXP op, string rho) {
  string out;
  if (PRIMFUN(op) == (SEXP (*)())do_set) {
    return op_set(e, op, rho);
  } else if (PRIMFUN(op) == (SEXP (*)())do_internal) {
    SEXP fun = CAR(CADR(e));
    Expression args;
    if (TYPEOF(INTERNAL(fun)) == BUILTINSXP) {
      args = op_list_local(CDR(CADR(e)), rho, FALSE, TRUE);
    } else {
      args = op_exp(CDR(CADR(e)), rho);
    }
    Expression func = op_exp(INTERNAL(fun), rho, TRUE);
    out = appl4(get_name(PRIMOFFSET(INTERNAL(fun))),
		"R_NilValue", func.var, args.var, rho);
    del(func);
    del(args);
    return Expression(out, TRUE, INVISIBLE, unp(out));
  } else if (PRIMFUN(op) == (SEXP (*)())do_function) {
    return op_fundef(CDR(e), rho);
  } else if (PRIMFUN(op) == (SEXP (*)())do_begin) {
    return op_begin(CDR(e), rho);
  } else if (PRIMFUN(op) == (SEXP (*)())do_if) {
    return op_if(CDR(e), rho);
  } else if (PRIMFUN(op) == (SEXP (*)())do_for) {
    return op_for(CDR(e), rho);
    /*
     * } else if (PRIMFUN(op) == (SEXP (*)())do_while) {
     *   return op_while(CDR(e), rho);
     */
  } else if (PRIMFUN(op) == (SEXP (*)())do_return && global_c_return) {
    return op_c_return(CDR(e), rho);
  } else {
    // default case for specials: call the (call, op, args, rho) fn
    Expression op1 = op_exp(op, rho);
    Expression args1 = op_list(CDR(e), rho, TRUE, TRUE);
    string call_str = appl2("lcons", op1.var, args1.var);
    Expression call = Expression(call_str, FALSE, VISIBLE, unp(call_str));
    out = appl4(get_name(PRIMOFFSET(op)),
		call.var,
		op1.var,
		args1.var,
		rho);
    del(call);
    del(op1);
    del(args1);
    return Expression(out, TRUE, 
		      1 - PRIMPRINT(op) ? VISIBLE : INVISIBLE, 
		      unp(out));
  }
}

Expression SubexpBuffer::op_builtin(SEXP e, SEXP op, string rho) {
  string out;
  Expression op1 = op_exp(op, rho);
  SEXP args = CDR(e);
  // special case for arithmetic operations
  if (PRIMFUN(op) == (SEXP (*)())do_arith) {

    // R_unary if there's one argument and it's a non-object
    if (args != R_NilValue
	&& CDR(args) == R_NilValue
	&& !isObject(CAR(args))) { // one argument, non-object
      Expression x = op_exp(CAR(args), rho, TRUE);
      out = appl3("R_unary", "R_NilValue", op1.var, x.var);
      del(x);

    // R_binary if two non-object arguments
    } else if (CDDR(args) == R_NilValue && 
	       !isObject(CAR(args))
	       && !isObject(CADR(args))) {
      Expression x = op_exp(CAR(args), rho, TRUE);
      Expression y = op_exp(CADR(args), rho, TRUE);
      out = appl4("R_binary", "R_NilValue", op1.var, x.var, y.var);
      del(x);
      del(y);
    }
    
  } else {  // common case: call the do_ function
    Expression args1 = op_list_local(args, rho, FALSE, TRUE);
    out = appl4(get_name(PRIMOFFSET(op)),
		"R_NilValue ",
		op1.var,
		args1.var,
		rho);
    del(args1);
  }

  del(op1);
  return Expression(out, TRUE, 1 - PRIMPRINT(op) ? VISIBLE : INVISIBLE, unp(out));
}

bool isSimpleSubscript(SEXP e) {
  return (TYPEOF(e) == LANGSXP
	  && CAR(e) == install("[")
	  && TYPEOF(CADR(e)) == SYMSXP);
}
  
Expression SubexpBuffer::op_set(SEXP e, SEXP op, string rho) {
  string out;
  if (PRIMVAL(op) == 1 || PRIMVAL(op) == 3) {    /* <-, = */
    if (isString(CADR(e))) {
      SETCAR(CDR(e), install(CHAR(STRING_ELT(CADR(e), 0))));
    }
    if (isSymbol(CADR(e))) {
      string name = make_symbol(CADR(e));
      Expression body;
      // if body is a function definition, give op_fundef
      // the R name so that the f-function can be called directly later on
      if (TYPEOF(CADDR(e)) == LANGSXP
	  && TYPEOF(CAR(CADDR(e))) == SYMSXP
	  && CAR(CADDR(e)) == install("function")) {
	body = op_fundef(CDR(CADDR(e)), rho, CHAR(PRINTNAME(CADR(e))));
      } else {
	body = op_exp(CADDR(e), rho);
      }
      append_defs("defineVar(" + name + ", " 
	+ body.var + ", " + rho + ");\n");
      del(body);
      return Expression(name, TRUE, INVISIBLE, "");
    } else if (isSimpleSubscript(CADR(e))) {
      return op_subscriptset(e, rho);
    } else if (isLanguage(CADR(e))) {
      Expression func = op_exp(op, rho);
      Expression args = op_list_local(CDR(e), rho);
      out = appl4("do_set",
		  "R_NilValue",
		  func.var,
		  args.var,
		  rho);
      del(func);
      del(args);
      return Expression(out, TRUE, INVISIBLE, unp(out));
    } else {
      global_ok = 0;
      return Expression("<<assignment with unrecognized LHS>>",
			TRUE, INVISIBLE, "");
    }
  } else if (PRIMVAL(op) == 2) {  /* <<- */
    Expression op1 = op_exp(op, rho);
    Expression args1 = op_list(CDR(e), rho, TRUE);
    out = appl4(get_name(PRIMOFFSET(op)),
		"R_NilValue",
		op1.var,
		args1.var,
		rho);
    del(op1);
    del(args1);
    return Expression(out, TRUE, INVISIBLE, unp(out));
  } else {
    global_ok = 0;
    return Expression("<<Assignment of a type not yet implemented>>",
		      TRUE, INVISIBLE, "");
  }
}

Expression SubexpBuffer::op_subscriptset(SEXP e, string rho) {
  // CAR(e) = "<-", "=", etc.
  // CADR(e) = LHS
  //   CAR(CADR(e)) = "["
  //   CADR(CADR(e)) = array
  //   CADDR(CADR(e)) = subscript
  // CADDR(e) = RHS
  SEXP array = CADR(CADR(e));
  SEXP sub = CADDR(CADR(e));
  SEXP rhs = CADDR(e);
  Expression a_sym = op_literal(array, rho);
  Expression a = op_exp(array, rho);
  Expression s = op_exp(sub, rho);
  Expression r = op_exp(rhs, rho);
  string assign = appl3("rcc_subassign", a.var, s.var, r.var);
  string defs = "defineVar(" + a_sym.var + ", " + assign + ", " + rho + ");\n";
  del(a_sym); del(s); del(r); defs += unp(assign);
  append_defs(defs);
  a.is_visible = INVISIBLE;
  return a;
}

// Returns true iff the given list only contains CARs that are
// R_MissingArg and TAGs that are either symbolic or null.
bool just_sym_tags(SEXP ls) {
  if (TYPEOF(ls) != LISTSXP && TYPEOF(ls) != NILSXP) {
    err("just_sym_tags internal error: non-list given\n");
  }
  while (ls != R_NilValue) {
    if (CAR(ls) != R_MissingArg) return FALSE;
    if (TYPEOF(TAG(ls)) != SYMSXP && TYPEOF(TAG(ls)) != NILSXP) {
      return FALSE;
    }
    ls = CDR(ls);
  }
  return TRUE;
}

// Output an application of a closure to actual arguments.
Expression SubexpBuffer::op_clos_app(Expression op1, SEXP args, string rho) {
  // see eval.c:438-9
  Expression call;
  string arglist;

  // Unlike most R internal functions, applyClosure actually uses its
  // 'call' argument, so we can't just call it R_NilValue
  if (global_self_allocate) {
    call = op_list_local(args, rho, TRUE, FALSE, op1.var);
    if (call.var == "R_NilValue") {
      arglist = "R_NilValue";
    } else {
      arglist = appl2("promiseArgs", call.var + "-1", rho);
    }
  } else {
    Expression args1 = op_list(args, rho, TRUE);
    string call_str = appl2("lcons", op1.var, args1.var);
    call = Expression(call_str, FALSE, VISIBLE, unp(call_str));
    arglist = appl2("promiseArgs", args1.var, rho);
    del(args1);
  }
  string out = appl5("applyClosure ",
		     call.var,
		     op1.var,
		     arglist,
		     rho,
		     "R_NilValue");
  del(call);
  del(op1);
  if (call.var != "R_NilValue") append_defs(unp(arglist));
  //string func_name = string(CHAR(PRINTNAME(CAR(e))));
  //bool fun_visible = vis_map.find(func_name)->second;
  // Punt here; can't easily tell whether the result of the closure
  // is visible, so we just call it visible.
  // This will result in some extra expressions being printed at toplevel.
  return Expression(out, TRUE, CHECK_VISIBLE, unp(out));
}
  
// Output the argument list for an external function (generally a list
// where the CARs are R_MissingArg and the TAGs are SYMSXPs
// representing the formal arguments)
Expression SubexpBuffer::op_arglist(SEXP e, string rho) {
  int i;
  string out, tmp, tmp1;
  SEXP arg;
  int len = Rf_length(e);
  if (len == 0) return nil_exp;
  SEXP *args = new SEXP[len];
  SubexpBuffer buf = new_sb("tmp_arglist_" + i_to_s(global_temps++) + "_");
    
  arg = e;
  for(i=0; i<len; i++) {
    args[i] = arg;
    arg = CDR(arg);
  }
    
  /* Don't unprotect R_NilValue, just the conses */
  tmp = buf.appl2("cons", make_symbol(TAG(args[len-1])), "R_NilValue");
  if (len > 1) {
    for(i=len-2; i>=0; i--) {
      string sym = make_symbol(TAG(args[i]));
      tmp1 = buf.appl2("cons", sym, tmp);
      buf.append_defs(unp(tmp));
      tmp = tmp1;
    }
  }
  delete [] args;
  out = new_sexp();
  string defs;
  defs += "{\n";
  defs += indent(buf.output());
  defs += indent(out + " = " + tmp + ";\n");
  defs += "}\n";
  append_defs(defs);
  return Expression(out, TRUE, INVISIBLE, unp(out));
}


Expression SubexpBuffer::op_arglist_local(SEXP e, string rho) {
  string defs;
  if (e == R_NilValue) return nil_exp;
  int i, len;
  string list = new_sexp_unp();
  len = Rf_length(e);
  if (len == 0) return nil_exp;
#if 0
  alloc_list.add(list, defs.length(), len);
#endif
  for(i=0; i<len; i++) {
    defs += "CAR(" + list + "-" + i_to_s(i) + ") = " 
      + make_symbol(TAG(e)) + ";\n";
    e = CDR(e);
  }
  append_defs(defs);
  return Expression(list, TRUE, INVISIBLE, "");
}
  
Expression SubexpBuffer::op_literal(SEXP e, string rho) {
  Expression formals, body, env;
  string v;
  switch (TYPEOF(e)) {
  case NILSXP:
    return nil_exp;
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
    global_ok = 0;
    return Expression("<<unimplemented vector>>", TRUE, INVISIBLE, "");
    break;
  case SYMSXP:
    return Expression(make_symbol(e), FALSE, VISIBLE, "");
    break;
  case LISTSXP:
    return op_list(e, rho, TRUE);
    break;
  case CLOSXP:
    formals = op_symlist(FORMALS(e), rho);
    body = op_literal(BODY(e), rho);
    v = appl3("mkCLOSXP  ",
	      formals.var,
	      body.var,
	      rho);
    del(formals);
    del(body);
    if (rho == "R_GlobalEnv") {
      return Expression(v, FALSE, INVISIBLE, "");
    } else {
      return Expression(v, TRUE, INVISIBLE, unp(v));
    }
    break;
  case ENVSXP:
    global_ok = 0;
    return Expression("<<unexpected environment>>", TRUE, INVISIBLE, "");
    break;
  case PROMSXP:
    global_ok = 0;
    return Expression("<<unexpected promise>>", TRUE, INVISIBLE, "");
    break;
  case LANGSXP:
    return op_list(e, rho, TRUE);
    break;
  case CHARSXP:
    v = appl1_unp("mkChar", quote(string(CHAR(e))));
    return Expression(v, TRUE, VISIBLE, "");
    break;
  case SPECIALSXP:
  case BUILTINSXP:
    return global_constants.op_primsxp(e, rho);
    break;
  case EXPRSXP:
  case EXTPTRSXP:
  case WEAKREFSXP:
    global_ok = 0;
    return Expression("<<unimplemented type " + i_to_s(TYPEOF(e)) + ">>",
		      TRUE, INVISIBLE, "");
    break;
  default:
    err("Internal error: op_literal encountered bad type\n");
    return bogus_exp; // never reached
    break;
  }
}

/* Output a list using locally allocated storage instead of R's
 * allocation mechanism. The literal argument determines whether the
 * CARs are to be output literally or programatically. opt_l_car is an
 * optional string used mostly for applyClosure arguments. If it is
 * nonempty, it makes the first CONS a LANGSXP whose CAR is the given
 * string.
 */
Expression SubexpBuffer::op_list_local(SEXP e, string rho,
				       bool literal /* = TRUE */, 
				       bool primFuncArgList /* = FALSE */,
				       string opt_l_car /* = "" */) {
  if (TYPEOF(e) != LISTSXP && TYPEOF(e) != LANGSXP && TYPEOF(e) != NILSXP) {
    err("non-list found in op_list_local\n");
  }
  if (!global_self_allocate) {
    if (opt_l_car.empty()) {
      return op_list(e, rho, literal, primFuncArgList);
    } else {
      Expression cdr = op_list(e, rho, literal, primFuncArgList);
      string out = appl2("lcons", opt_l_car, cdr.var);
      del(cdr);
      return Expression(out, cdr.is_dep, VISIBLE, unp(out));
    }
  }
  bool list_dep = FALSE;
  if (e == R_NilValue) return nil_exp;
  int i, len, init_for;
  string set_list, dt;
  string list = new_sexp();
  if (opt_l_car.empty()) {
    len = Rf_length(e);
    if (len == 0) return nil_exp;
    init_for = 0;
  } else {
    len = 1 + Rf_length(e);
    init_for = 1;
    set_list += "CAR(" + list + ") = " + opt_l_car + ";\n";
    set_list += "TYPEOF(" + list + ") = LANGSXP;\n";
    dt += "TYPEOF(" + list + ") = LISTSXP;\n";
  }
  for(i=init_for; i<len; i++) {
    Expression car_exp;
    if (literal) {
      car_exp = op_literal(CAR(e), rho);
    } else {
      car_exp = op_exp(CAR(e), rho, primFuncArgList);
    }
    list_dep = list_dep || car_exp.is_dep;
    set_list += "CAR(" + list + " - " + i_to_s(i) + ") = " + car_exp.var + ";\n";
    dt += car_exp.del_text;
    if (TYPEOF(e) == LANGSXP) {
      set_list += "TYPEOF(" + list + " - " + i_to_s(i) + ") = LANGSXP;\n";
      dt += "TYPEOF(" + list + " - " + i_to_s(i) + ") = LISTSXP;\n";
    }
    if (TAG(e) != R_NilValue) {
      Expression tag_exp = op_literal(TAG(e), rho);
      set_list += "TAG(" + list + " - " + i_to_s(i) + ") = " + tag_exp.var + ";\n";
      dt += "TAG(" + list + " - " + i_to_s(i) + ") = R_NilValue;\n";
    }
    e = CDR(e);
  }
#if 0
  alloc_list.add(list, defs.length(), len);
#endif
  append_defs(set_list);
  Expression out = Expression(list, list_dep, VISIBLE, dt);
  out.is_alloc = TRUE;
  return out;
}

Expression SubexpBuffer::op_list(SEXP e, string rho, bool literal, 
				 bool primFuncArgList /* = FALSE */ ) {
  int i, len;
  len = Rf_length(e);
  string my_cons;
  if (len == 1) {
    switch (TYPEOF(e)) {
    case NILSXP:
      return nil_exp;
    case LISTSXP:
      my_cons = "cons";
      break;
    case LANGSXP:
      my_cons = "lcons";
      break;
    default:
      err("Internal error: bad call to op_list\n");
      return bogus_exp;  // never reached
    }
    Expression car = (literal ? op_literal(CAR(e), rho) : op_exp(CAR(e), rho, primFuncArgList));
    string out;
    if (car.is_dep) {
      if (TAG(e) == R_NilValue) {
	out = appl2(my_cons, car.var, "R_NilValue");
	del(car);
      } else {
	Expression tag = op_literal(TAG(e), rho);
	out = appl3("tagged_cons", car.var, tag.var, "R_NilValue");
	del(car);
	del(tag);
      }
      return Expression(out, TRUE, VISIBLE, unp(out));
    } else {  // not dep
      if (TAG(e) == R_NilValue) {
	out = global_constants.appl2(my_cons, car.var, "R_NilValue");
	global_constants.del(car);
      } else {
	Expression tag = global_constants.op_literal(TAG(e), rho);
	out = global_constants.appl3("tagged_cons", car.var, tag.var, "R_NilValue");
	global_constants.del(car);
	global_constants.del(tag);
      }
      return Expression(out, FALSE, VISIBLE, "");
    }
  } else {  // length >= 2
    string unp_cars = "";
    bool list_dep = FALSE;
    Expression *exps = new Expression[len];
    Expression *tags = new Expression[len];
    bool *langs = new bool[len];
    SEXP tmp_e = e;
    for(i=0; i<len; i++) {
      switch (TYPEOF(tmp_e)) {
      case LISTSXP:
	langs[i] = FALSE;
	break;
      case LANGSXP:
	langs[i] = TRUE;
	break;
      default:
	err("Internal error: bad call to op_list\n");
	return bogus_exp;  // never reached
      }
      exps[i] = (literal? op_literal(CAR(tmp_e), rho) : op_exp(CAR(tmp_e), rho, 
							       primFuncArgList));
      tags[i] = (TAG(tmp_e) == R_NilValue ? Expression("", FALSE, INVISIBLE, "") : op_literal(TAG(tmp_e), rho));
      if (exps[i].is_dep) list_dep = TRUE;
      tmp_e = CDR(tmp_e);
    }
    SubexpBuffer tmp_buf = new_sb("tmp_ls_" + i_to_s(global_temps++) + "_");
    string cdr = "R_NilValue";
    for(i=len-1; i>=0; i--) {
      string new_cdr;
      my_cons = (langs[i] ? "lcons" : "cons");
      if (tags[i].var.empty()) {
	cdr = tmp_buf.appl2_unp(my_cons, exps[i].var, cdr);
      } else {
	cdr = tmp_buf.appl3_unp("tagged_cons", exps[i].var, tags[i].var, cdr);
      }
      unp_cars += exps[i].del_text;
    }
    delete [] exps;
    delete [] tags;
    delete [] langs;
    if (list_dep) {
      string handle = new_sexp();
      string defs;
      defs += "{\n";
      defs += indent(tmp_buf.output());
      defs += indent("PROTECT(" + handle + " = " + cdr + ");\n");
      defs += indent(unp_cars);
      defs += "}\n";
      append_defs(defs);
      return Expression(handle, list_dep, VISIBLE, unp(handle));
    } else {
      string handle = global_constants.new_sexp();
      string defs;
      defs += "{\n";
      defs += indent(tmp_buf.output());
      defs += indent("PROTECT(" + handle + " = " + cdr + ");\n");
      defs += indent(unp_cars);
      defs += "}\n";
      global_constants.append_defs(defs);
      return Expression(handle, list_dep, VISIBLE, "");
    }
  }
}

#if 0

// Old version of op_list: recursive instead of iterative.  Changed to
// handle the constant/non-constant distinction in a more natural way.

Expression SubexpBuffer::op_list(SEXP e, string rho, bool literal = TRUE) {
  SubexpBuffer temp_f = new_sb("tmp_" + i_to_s(global_temps++) + "_");
  SubexpBuffer temp_c = new_sb("tmp_" + i_to_s(global_temps++) + "_");
  string out_const, var_c;
  Expression exp = temp_f.op_list_help(e, rho, temp_c, out_const, literal);
  if (!out_const.empty()) {
    var_c = global_constants.new_sexp();
    global_constants.defs += "{\n";
    global_constants.defs += indent(temp_c.output());
    global_constants.defs += 
      indent("PROTECT(" + var_c + " = " + out_const + ");\n");
    global_constants.defs += "}\n";
  } else {
    var_c = exp.var;
  }
  if (exp.is_dep) {
    if (temp_f.defs.empty()) {
      return exp;
    } else {
      string var_f = new_sexp();
      defs += "{\n";
      defs += indent(temp_f.output());
      defs += indent("PROTECT(" + var_f + " =" + exp.var + ");\n");
      //defs += indent(var_f + " =" + exp.var + ";\n");
      defs += "}\n";
      return Expression(var_f, TRUE, VISIBLE, unp(var_f));
    }
  } else {
    return Expression(var_c, FALSE, VISIBLE, "");
  }
}

// Note: Does not protect the result, because it is often called directly
// before a cons, where it doesn't need to be protected.
Expression SubexpBuffer::
op_list_help(SEXP e, string rho,
	     SubexpBuffer & consts,
	     string & out_const, bool literal) {
  string my_cons;
  switch (TYPEOF(e)) {
  case NILSXP:
    return nil_exp;
  case LISTSXP:
    my_cons = "cons";
    break;
  case LANGSXP:
    my_cons = "lcons";
    break;
  default:
    err("Internal error: bad call to op_list\n");
    return bogus_exp;  // never reached
  }
  if (TAG(e) == R_NilValue) {
    Expression car;
    if (literal) {
      car = op_literal(CAR(e), rho);
    } else {
      car = op_exp(CAR(e), rho);
    }
    Expression cdr = op_list_help(CDR(e), rho, consts,
				  out_const, literal);
    if (car.is_dep && cdr.is_dep) {
      string out = appl2_unp(my_cons, car.var, cdr.var);
      del(car);
      del(cdr);
      return Expression(out, TRUE, VISIBLE, "");
    } else if (!car.is_dep && !cdr.is_dep) {
      out_const = consts.appl2_unp(my_cons, car.var, cdr.var);
      consts.del(car);
      consts.del(cdr);
      return Expression(out_const, FALSE, VISIBLE, "");
      /* If this is dependent but some subexpression is constant, create
       * the bridge between the constant subexpression and the global 
       * constants. */
    } else if (car.is_dep && !cdr.is_dep) {
      string out = appl2_unp(my_cons, car.var, cdr.var);
      del(car);
      del(cdr);
      return Expression(out, TRUE, VISIBLE, "");
    } else if (!car.is_dep && cdr.is_dep) {
      string out = appl2_unp(my_cons, car.var, cdr.var);
      del(car);
      del(cdr);
      return Expression(out, TRUE, VISIBLE, "");
    }
  } else { /* It's a tagged cons */
    if (my_cons == "lcons") {
      err("Internal error: op_list encountered tagged lcons\n");
    }
    Expression car;
    if (literal) {
      car = op_literal(CAR(e), rho);
    } else {
      car = op_exp(CAR(e), rho);
    }
    Expression tag = op_literal(TAG(e), rho);
    Expression cdr = op_list_help(CDR(e), rho, consts, 
				  out_const, literal);
    if (car.is_dep && tag.is_dep && cdr.is_dep) {
      string out = appl3_unp("tagged_cons", car.var, tag.var, cdr.var);
      del(car);
      del(tag);
      del(cdr);
      return Expression(out, TRUE, VISIBLE, "");
    } else if (!car.is_dep && !tag.is_dep && !cdr.is_dep) {
      out_const = consts.appl3_unp("tagged_cons ", car.var, tag.var, cdr.var);
      consts.del(car);
      consts.del(tag);
      consts.del(cdr);
      return Expression(out_const, FALSE, VISIBLE, "");
    } else {
      global_constants.append_defs(consts.output());
      string out = appl3_unp("tagged_cons  ", car.var, tag.var, cdr.var);
      del(car);
      del(tag);
      del(cdr);
      return Expression(out, TRUE, VISIBLE, "");
    }
  }
  return bogus_exp; //never reached
}

#endif
  
Expression SubexpBuffer::op_string(SEXP s) {
  int i, len;
  string str = "";
  len = Rf_length(s);
  for(i=0; i<len; i++) {
    str += string(CHAR(STRING_ELT(s, i)));
  }
  string out = global_constants.appl1("mkString", 
				      quote(escape(str)));
  return Expression(out, FALSE, VISIBLE, "");
}
  
Expression SubexpBuffer::op_vector(SEXP vec) {
  int len = Rf_length(vec);
  switch(TYPEOF(vec)) {
  case LGLSXP:
    if (len == 1) {
      int value = INTEGER(vec)[0];
      map<int,string>::iterator pr = sc_logical_map.find(value);
      if (pr == sc_logical_map.end()) {  // not found
	string var = global_constants.appl1("ScalarLogical",
					    i_to_s(value));
	sc_logical_map.insert(pair<int,string>(value, var));
	return Expression(var, FALSE, VISIBLE, "");
      } else {
	return Expression(pr->second, FALSE, VISIBLE, "");
      }
    } else {
      global_ok = 0;
      return Expression("<<unimplemented logical vector>>",
			FALSE, INVISIBLE, "");
    }
    break;
  case INTSXP:
    if (len == 1) {
      int value = INTEGER(vec)[0];
      map<int,string>::iterator pr = sc_integer_map.find(value);
      if (pr == sc_integer_map.end()) {  // not found
	string var = global_constants.appl1("ScalarInteger",
					    i_to_s(value));
	sc_integer_map.insert(pair<int,string>(value, var));
	return Expression(var, FALSE, VISIBLE, "");
      } else {
	return Expression(pr->second, FALSE, VISIBLE, "");
      }
    } else {
      global_ok = 0;
      return Expression("<<unimplemented integer vector>>",
			FALSE, INVISIBLE, "");
    }
    break;
  case REALSXP:
    if (len == 1) {
      double value = REAL(vec)[0];
      map<double,string>::iterator pr = sc_real_map.find(value);
      if (pr == sc_real_map.end()) {  // not found
	string var = global_constants.appl1("ScalarReal",
					    d_to_s(value));
	sc_real_map.insert(pair<double,string>(value, var));
	return Expression(var, FALSE, VISIBLE, "");
      } else {
	return Expression(pr->second, FALSE, VISIBLE, "");
      }
    } else {
      global_ok = 0;
      return Expression("<<unimplemented real vector>>",
			FALSE, INVISIBLE, "");
    }
    break;
  case CPLXSXP:
    if (len == 1) {
      Rcomplex value = COMPLEX(vec)[0];
      string var = global_constants.appl1("ScalarComplex",
					  c_to_s(value));
      return Expression(var, FALSE, VISIBLE, "");
    } else {
      global_ok = 0;
      return Expression("<<unimplemented complex vector>>",
			FALSE, INVISIBLE, "");
    }
    break;
  default:
    err("Internal error: op_vector encountered bad vector type\n");
    return bogus_exp; // not reached
    break;
  }
}

string SubexpBuffer::output() {
  string out;
  output_ip();
  finalize();
  out += output_decls();
  out += output_defs();
  return out;
}

void SubexpBuffer::output_ip() {
#if 0
  int i;
  static int id;
  list<AllocListElem> ls = alloc_list.get();
  list<AllocListElem>::iterator p;
  list<VarRef>::iterator q;
  int offset = 0;
  for(p = ls.begin(), i=0; p != ls.end(); p++, i++) {
    string var = "mem" + i_to_s(i) + "_" + i_to_s(id++);
    encl_fn->decls += "SEXP " + var + ";\n";
    string alloc = var + "= alloca(" + i_to_s(p->max) + "*sizeof(SEXPREC));\n";
    alloc += "my_init_memory(" + var + ", " + i_to_s(p->max) + ");\n";
    encl_fn->defs.insert(0, alloc);
    if (encl_fn == this) offset += alloc.length();
    for(q = p->vars.begin(); q != p->vars.end(); q++) {
      string ref = q->name + " = " + var + "+" + i_to_s(q->size - 1) + ";\n";
      defs.insert(q->location + offset, ref);
      offset += ref.length();
    }
  }
#endif
}

int main(int argc, char *argv[]) {
  int i;
  SEXP *expressions;
  char *fullname_c;
  string fullname, libname, out_filename, path, exprs;
  bool in_file_exists, out_file_exists = FALSE;
  FILE *in_file;
  int n_exprs;

  // for getopt
  int c;
  extern char *optarg;
  extern int optind, opterr, optopt;

  while(1) {
    c = getopt(argc, argv, "f:lo:");
    if (c == -1) {
      break;
    }
    switch(c) {
    case 'f':
      direct_funcs.push_back(string(optarg));
      break;
    case 'a':
      output_default_args = FALSE;
      break;
    case 'm':
      output_main_program = FALSE;
      break;
    case 'l':
      global_self_allocate = TRUE;
      break;
    case 'o':
      out_file_exists = TRUE;
      out_filename = string(optarg);
      break;
    case '?':
      arg_err();
      break;
    case ':':
      arg_err();
      break;
    default:
      err("Unknown error: getopt() returned " + i_to_s(c) + "\n");
      break;
    }
  }

  if (optind < argc) {
    in_file_exists = TRUE;
    fullname_c = argv[optind++];
    if (optind < argc) {
      printf("Warning: ignoring extra arguments: ");
      while (optind < argc) {
	printf("%s ", argv[optind++]);
      }
      printf("\n");
    }
  } else {  // no filename specified
    in_file_exists = FALSE;
  }

  // Initialize R interface and parse
  init_R();

  if (in_file_exists) {
    in_file = fopen(fullname_c, "r");
    if (in_file == NULL) {
      string program = argv[0];

      string::size_type dot = program.rfind (".",program.length());
      if (dot != 0) program = program.substr(0,dot);

      string::size_type slash = program.rfind ("/",program.length());
      if (slash != 0) program = program.substr(slash+1,program.length());

      cerr << program << ": unable to open input file \"" << fullname_c << "\"" << endl;
      exit(-1);
    }
    fullname = string(fullname_c);
    int pos = filename_pos(fullname);
    path = fullname.substr(0,pos);
    string filename = fullname.substr(pos, fullname.size() - pos);
    libname = make_c_id(strip_suffix(filename));
    // Lib name must be alphanumerical to be part of R_init_<library>
    // function (explained below)
  } else {
    in_file = stdin;
    libname = "R_output";
    path = "";
  }

  parse_R(in_file, &expressions);

  if (!out_file_exists) {
    out_filename = path + libname + ".c";
  }

  SubexpBuffer *fundefs_ptr;
  fundefs_ptr = new SubexpBuffer(libname + "_f", TRUE);
  global_fundefs = *fundefs_ptr;
  delete fundefs_ptr;

  // build expressions
  global_constants.decls += "static void exec();\n";
  exprs += "\nstatic void exec() {\n";

  SEXP *e;
  e = expressions;
  n_exprs = 0;
  while (*e != NULL) {
    n_exprs++;
    e++;
  }

  for(i=0; i<n_exprs; i++) {
    exprs += indent("SEXP e" + i_to_s(i) + ";\n");
  }

  for(i=0; i<n_exprs; i++) {
    SubexpBuffer subexps;
    Expression exp = subexps.op_exp(expressions[i], "R_GlobalEnv");
    UNPROTECT_PTR(expressions[i]);
    exprs += indent("{\n");
    exprs += indent(indent(Visibility::emit_set_if_visible(exp.is_visible)));
    exprs += indent(indent("{\n"));
    exprs += indent(indent(indent(subexps.output())));
    if (exp.is_visible != INVISIBLE) {
      string evar = "e" + i_to_s(i);  
      string printexpn = evar  + " = " + exp.var + ";\n";
      string check;
      if (exp.is_visible == CHECK_VISIBLE) 
	check = Visibility::emit_check_expn();
      printexpn +=  
	emit_logical_if_stmt(check,
			     emit_call2("PrintValueEnv", 
					evar , "R_GlobalEnv"));

      exprs += indent(indent(indent(printexpn)));
    }
    exprs += indent(indent(indent(exp.del_text)));
    exprs += indent(indent("}\n"));
    exprs += indent("}\n");
    e++;
  }
  exprs += indent("UNPROTECT(" + i_to_s(global_constants.get_n_prot())
		  + "); /* c_ */\n");
  exprs += "}\n\n";
  

  
  if (!global_ok) {
    out_filename += ".bad";
    cerr << "Error: one or more problems compiling R code.\n"
	 << "Outputting best attempt to " 
	 << "\'" << out_filename << "\'.\n";
  }
  ofstream out_file(out_filename.c_str());
  if (!out_file) {
    err("Couldn't open file " + out_filename + " for output");
  }
  
  string rcc_path_prefix = string("#include \"") + RCC_INCLUDE_PATH + "/"; 
  string r_path_prefix = rcc_path_prefix + "R/";
  /* output to file */
  out_file << r_path_prefix << "IOStuff.h\"\n";
  out_file << r_path_prefix << "Parse.h\"\n";
  out_file << r_path_prefix << "Internal.h\"\n";
  out_file << r_path_prefix << "R_ext/RConverters.h\"\n";
  out_file << r_path_prefix << "main/arithmetic.h\"\n";
  out_file << rcc_path_prefix << "rcc_lib.h\"\n";
  out_file << "\n";

  global_fundefs.output_ip();
  global_fundefs.finalize();

  global_constants.output_ip();
  global_constants.finalize();

  out_file << global_fundefs.output_decls();
  out_file << global_constants.output_decls();

  string file_initializer_name = string("R_init_") + libname;
  {
    string header;
    header += "\nvoid " + file_initializer_name + "() {\n";
    /* The name R_init_<libname> causes the R dynamic loader to execute the
     * function immediately. */
    for(i=0; i<global_constants.get_n_inits(); i++) {
      header += indent(global_constants.get_init_str() + i_to_s(i) + "();\n");
    }
    header += indent("exec();\n");
    header += "}\n";
    out_file << header;
  }

  out_file << global_constants.output_defs();
  out_file << exprs;
  out_file << global_fundefs.output_defs();
  if (output_main_program) {
     string arginit = "int myargc;\n";
     string mainargs = "int argc, char **argv";
     if (output_default_args) {
       mainargs = "int argc, char **argv";
       arginit += string("char *myargv[5];\n") + "myargv[0] = argv[0];\n" + "myargv[1] = \"--gui=none\";\n" + 
	  "myargv[2] = \"--slave\";\n" + "myargv[3] = \"--vanilla\";\n" + "myargv[4] = NULL;\n" + "myargc = 4;\n";
     } else {
       mainargs = "int argc, char **argv";
       arginit += string("myargc = argc\n;") + "myargv = argv\n;";
     }
     string body = arginit + "Rf_initialize_R(myargc, myargv);\n" + "setup_Rmainloop();\n" +
       file_initializer_name + "();\nreturn 0;\n";
     out_file << "\nint main(" << mainargs << ") \n{\n" << indent(body) << "}\n"; 
  }

  if (!out_file) {
    err("Couldn't write to file " + out_filename);
  }
  if (global_ok) {
    return 0;
  } else {
    return 1;
  }
}

static void arg_err() {
  cerr << "Usage: rcc [input-file] [-l] [-o output-file] [-f function-name]*\n";
  exit(1);
}

static void set_funcs(int argc, char *argv[]) {
  int i;
  for(i=0; i<argc; i++) {
    direct_funcs.push_back(string(*argv++));
  }
}

/***
string make_fundef(string func_name, SEXP args, SEXP code) {
  int i;
  string f, header;
  SEXP temp_args = args;
  int len = Rf_length(args);
  SubexpBuffer out_subexps;
  SubexpBuffer env_subexps;
  string * old_formals;
  int old_formals_len;
  old_formals = global_formals;
  old_formals_len = global_formals_len;
  global_formals = new string[len];
  global_formals_len = len;
  for (i=0; i<len; i++) {
    global_formals[i] = string(CHAR(PRINTNAME(TAG(temp_args))));
    temp_args = CDR(temp_args);
  }
  header = "SEXP " + func_name + "(SEXP env";
  for (i=0; i<len; i++) {
    header += ", SEXP arg" + i_to_s(i);
  }
  header += ")";
  global_fundefs.decls += header + ";\n";
  f += header + " {\n";
  f += indent("SEXP newenv;\n");
  f += indent("SEXP out;\n");
  f += indent("RCNTXT context;\n");
  string formals = env_subexps.op_symlist(args, "env").var;
  string actuals = "R_NilValue";
  for (i=len-1; i>=0; i--) {
    actuals = env_subexps.appl2("cons",
				"arg" + i_to_s(i),
				actuals);
  }
  int env_nprot = env_subexps.get_n_prot() + 1;
  f += indent(env_subexps.output());
  f += indent("PROTECT(newenv =\n");
  f += indent(indent("Rf_NewEnvironment(\n"
		     + indent(formals) + ",\n"
		     + indent(actuals) + ",\n"
		     + indent("env") + "));\n"));
  f += indent("if (SETJMP(context.cjmpbuf)) {\n");
  f += indent(indent("out = R_ReturnedValue;\n"));
  f += indent("} else {\n");
  f += indent(indent("begincontext(&context, CTXT_RETURN, R_NilValue, newenv, env, R_NilValue, R_NilValue);\n"));
  Expression outblock = out_subexps.op_exp(code, "newenv");
  f += indent(indent("{\n"));
  f += indent(indent(indent(out_subexps.output())));
  f += indent(indent(indent("out = " + outblock.var + ";\n")));
  out_subexps.del(outblock); // ?????
  f += indent(indent("}\n"));
  f += indent("}\n");
  f += indent("endcontext(&context);\n");
  f += indent("UNPROTECT(" + i_to_s(env_nprot) + ");\n");
  f += indent("return out;\n");
  f += "}\n";
  delete [] global_formals;
  global_formals = old_formals;
  global_formals_len = old_formals_len;
  return f;
}

***/

/* Make a function where the arguments of the R function are packed in
 * a list to form a single f-function argument, as opposed to
 * make_fundef where the mapping is one-to-one. This version is used
 * for functions that include the "..." object.
 *
 * Actually, at the moment it's being used all the time for simplicity.
 */
string make_fundef_argslist(SubexpBuffer * this_buf, string func_name, SEXP args, SEXP code) {
  string f, header;
  SubexpBuffer out_subexps;
  SubexpBuffer env_subexps;
  header = "SEXP " + func_name + "(";
  header += "SEXP args, SEXP env)";
  global_fundefs.decls += header + ";\n";
  f += header + " {\n";
  f += indent("SEXP newenv;\n");
  f += indent("SEXP out;\n");
  f += indent("RCNTXT context;\n");
  Expression formals = env_subexps.op_symlist(args, "env");
  string actuals = "args";
  env_subexps.output_ip();
  env_subexps.finalize();
  f += indent(env_subexps.output_decls());
#if 0
  f += indent("PROTECT(env = CADR(full_args));\n");
  f += indent("PROTECT(args = CDDR(full_args));\n");
#endif
  f += indent(env_subexps.output_defs());
  f += indent("PROTECT(newenv =\n");
  f += indent(indent("Rf_NewEnvironment(\n"
		     + indent(formals.var) + ",\n"
		     + indent(actuals) + ",\n"
		     + indent("env") + "));\n"));
  f += indent("if (SETJMP(context.cjmpbuf)) {\n");
  f += indent(indent("PROTECT(out = R_ReturnedValue);\n"));
  f += indent("} else {\n");
  f += indent(indent("begincontext(&context, CTXT_RETURN, R_NilValue, newenv, env, R_NilValue, R_NilValue);\n"));
  Expression outblock = out_subexps.op_exp(code, "newenv");
  f += indent(indent("{\n"));
  f += indent(indent(indent(out_subexps.output() +
			    Visibility::emit_set(outblock.is_visible))));
  f += indent(indent(indent("PROTECT(out = " + outblock.var + ");\n")));
  f += indent(indent(indent(outblock.del_text)));
  f += indent(indent("}\n"));
  f += indent("}     \n");
  f += indent("endcontext(&context);\n");
#if 0
  f += indent(unp("env"));
  f += indent(unp("args"));
#endif
  f += indent(unp("newenv"));
  f += indent(formals.del_text);
  f += indent(unp("out"));
  f += indent("return out;\n");
  f += "}\n";
  //vis_map[func_name] = outblock.is_visible;
  return f;
}

/* Like make_fundef_arglist but for directly-called functions. */
string make_fundef_argslist_c(SubexpBuffer * this_buf, string func_name, SEXP args, SEXP code) {
  string f, header;
  SubexpBuffer out_subexps;
  SubexpBuffer env_subexps;
  header = "SEXP " + func_name + "(";
  header += "SEXP args)";
  global_fundefs.decls += header + ";\n";
  f += header + " {\n";
  f += indent("SEXP newenv;\n");
  f += indent("SEXP out;\n");
  Expression formals = env_subexps.op_symlist(args, "R_GlobalEnv");
  f += env_subexps.output();
  f += indent("PROTECT(newenv =\n");
  f += indent(indent("Rf_NewEnvironment(\n"
		     + indent(formals.var) + ",\n"
		     + indent("args") + ",\n"
		     + indent("R_GlobalEnv") + "));\n"));
  Expression outblock = out_subexps.op_exp(code, "newenv");
  f += indent("{\n");
  f += indent(indent(out_subexps.output()));
  f += indent(indent("PROTECT(out = " + outblock.var + ");\n" + 
		     Visibility::emit_set(outblock.is_visible)));
  f += indent(indent(outblock.del_text));
  f += indent("}\n");
  f += indent(unp("newenv"));
  f += indent(formals.del_text);
  f += indent(unp("out"));
  f += indent("return out;\n");
  f += "}\n";
  //vis_map[func_name] = outblock.is_visible;
  return f;
}

string make_symbol(SEXP e) {
  if (e == R_MissingArg) {
    return "R_MissingArg";
  } else if (e == R_UnboundValue) {
    return "R_UnboundValue";
  } else {
    string name = string(CHAR(PRINTNAME(e)));
    map<string,string>::iterator pr = symbol_map.find(name);
    if (pr == symbol_map.end()) {  // not found
      string var = global_constants.new_sexp_unp_name(name);
      string qname = quote(name);
      global_constants.appl(var, FALSE, "install", 1, &qname);
      symbol_map.insert(pair<string,string>(name, var));
      return var;
    } else {
      return pr->second;
    }
  }
}

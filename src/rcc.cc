/* Copyright (c) 2003 John Garvin 
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

static bool global_self_allocate = 0; // breaks on GC when 1
static bool global_ok = 1;
static unsigned int global_temps = 0;
static map<string, string> symbol_map;
static map<double, string> sc_real_map;
static map<int, string> sc_logical_map;
static map<int, string> sc_integer_map;
static map<int, string> primsxp_map;
static map<string, bool> vis_map;
static string * global_formals = NULL;
static int global_formals_len = 0;
static SubexpBuffer global_fundefs("f");
static SplitSubexpBuffer global_constants("c");
static SubexpBuffer global_labels("l");
static Expression bogus_exp = Expression("BOGUS", FALSE, FALSE, "");
static Expression nil_exp = Expression("R_NilValue", FALSE, FALSE, "");

string SubexpBuffer::new_var() {
  prot++;
  return new_var_unp();
}
string SubexpBuffer::new_var_unp() {
  return prefix + i_to_s(n++);
}
int SubexpBuffer::get_n_vars() {
  return n;
}
int SubexpBuffer::get_n_prot() {
  return prot;
}
string SubexpBuffer::new_sexp() {
  string str = new_var();
  decls += "SEXP " + str + ";\n";
  return str;
}
string SubexpBuffer::new_sexp_unp() {
  string str = new_var_unp();
  decls += "SEXP " + str + ";\n";
  return str;
}

/* Convenient macro-like things for outputting function applications */

string SubexpBuffer::appl1(string func, string arg) {
  string var = new_sexp();
  defs += "PROTECT(" + var + " = " + func + "(" + arg + "));\n";
  return var;
}

string SubexpBuffer::appl1_unp(string func, string arg) {
  string var = new_sexp_unp();
  defs += var + " = " + func + "(" + arg + ");\n";
  return var;
}
  
string SubexpBuffer::appl2(string func, string arg1, string arg2) {
  string var = new_sexp();
  defs += "PROTECT(" + var + " = " 
    + func + "(" + arg1 + ", " + arg2 + "));\n";
  return var;
}
  
string SubexpBuffer::appl2_unp(string func, string arg1, string arg2) {
  string var = new_sexp_unp();
  defs += var + " = " 
    + func + "(" + arg1 + ", " + arg2 + ");\n";
  return var;
}
  
string SubexpBuffer::appl3(string func, 
			   string arg1, string arg2, string arg3) {
  string var = new_sexp();
  defs += "PROTECT(" + var + " = " 
    + func + "(" + arg1 + ", " + arg2 + ", " + arg3 + "));\n";
  return var;
}
  
string SubexpBuffer::appl3_unp(string func,
			       string arg1, string arg2, string arg3) {
  string var = new_sexp_unp();
  defs += var + " = " 
    + func + "(" + arg1 + ", " + arg2 + ", " + arg3 + ");\n";
  return var;
}
  
string SubexpBuffer::appl4(string func, 
			   string arg1, 
			   string arg2, 
			   string arg3, 
			   string arg4) {
  string var = new_sexp();
  defs += "PROTECT(" + var + " = " 
    + func + "(" + arg1 + ", " + arg2 + ", " + arg3 + ", " + arg4 + "));\n";
  return var;
}
  
string SubexpBuffer::appl5(string func, 
			   string arg1, 
			   string arg2, 
			   string arg3, 
			   string arg4,
			   string arg5) {
  string var = new_sexp();
  defs += "PROTECT(" + var + " = " 
    + func + "(" + arg1 + ", " + arg2 + ", " + arg3 + ", " + arg4 
    + ", " + arg5 + "));\n";
  return var;
}

string SubexpBuffer::appl5_unp(string func, 
			       string arg1, 
			       string arg2, 
			       string arg3, 
			       string arg4,
			       string arg5) {
  string var = new_sexp();
  defs += var + " = " 
    + func + "(" + arg1 + ", " + arg2 + ", " + arg3 + ", " + arg4 
    + ", " + arg5 + ");\n";
  return var;
}
  
string SubexpBuffer::appl6(string func,
			   string arg1,
			   string arg2,
			   string arg3,
			   string arg4,
			   string arg5,
			   string arg6) {
  string var = new_sexp();
  defs += "PROTECT(" + var + " = " 
    + func + "(" + arg1 + ", " + arg2 + ", " + arg3 + ", " + arg4 
    + ", " + arg5 + ", " + arg6 + "));\n";
  return var;
}

void SubexpBuffer::del(Expression exp) {
  defs += exp.del_text;
  if (exp.is_alloc) {
    alloc_list.remove(exp.var);
  }
}

Expression SubexpBuffer::op_exp(SEXP e, string rho) {
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
    return Expression("<<unimplemented vector>>", TRUE, FALSE, "");
    break;
  case SYMSXP:
    if (e == R_MissingArg) {
      return Expression("R_MissingArg", FALSE, FALSE, "");
    } else {
      if (global_formals != NULL && rho != "R_GlobalEnv") {
	int i;
	for(i=0; i<global_formals_len; i++) {
	  if (string(CHAR(PRINTNAME(e))) == global_formals[i]) {
	    return Expression("arg" + i_to_s(i), TRUE, FALSE, "");
	  }
	}
      }
      sym = make_symbol(e);
      string v = appl2("findVar", sym, rho);
      out = Expression(v, TRUE, TRUE, unp(v));
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
      out = Expression(v, FALSE, FALSE, "");
    } else {
      string v = appl3("mkCLOSXP",
		       formals.var,
		       body.var,
		       rho);
      del(formals);
      del(body);
      out = Expression(v, TRUE, FALSE, unp(v));
    }
    return out;
    break;
  case ENVSXP:
    global_ok = 0;
    return Expression("<<unexpected environment>>", TRUE, FALSE, "");
    break;
  case PROMSXP:
    global_ok = 0;
    return Expression("<<unexpected promise>>", TRUE, FALSE, "");
    break;
  case LANGSXP:
    out = op_lang(e, rho);
    return out;
    break;
  case CHARSXP:
    return Expression(appl1_unp("mkChar",
				quote(string(CHAR(e)))),
		      FALSE, TRUE, "");
    break;
  case SPECIALSXP:
  case BUILTINSXP:
    return global_constants.op_primsxp(e, rho);
  case EXPRSXP:
  case EXTPTRSXP:
  case WEAKREFSXP:
    global_ok = 0;
    return Expression("<<unimplemented type " + i_to_s(TYPEOF(e)) + ">>",
		      TRUE, FALSE, "");
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
    
  int value = 2 * PRIMOFFSET(e) + is_builtin;
  map<int,string>::iterator pr = primsxp_map.find(value);
  if (pr == primsxp_map.end()) {  // not found
    string var = new_sexp();
    defs += "PROTECT(" + var
      + " = mkPRIMSXP(" + i_to_s(PRIMOFFSET(e)) 
      + "," + i_to_s(is_builtin) + "));"
      + " /* " + string(PRIMNAME(e)) + " */\n";
    primsxp_map.insert(pair<int,string>(value, var));
    return Expression(var, TRUE, FALSE, "");
  } else {
    return Expression(pr->second, TRUE, FALSE, "");
  }
}

Expression SubexpBuffer::op_symlist(SEXP e, string rho) {
  return op_list(e, rho);
}

Expression SubexpBuffer::op_lang(SEXP e, string rho) {
  SEXP op;
  string out;
  Expression exp;
  
  if (TYPEOF(CAR(e)) == SYMSXP) {
    op = findVar(CAR(e), R_GlobalEnv);
    if (op == R_UnboundValue) {  /* user-defined function */
      //Expression args = op_exp(CDR(e), rho);
      Expression args = op_list_array(CDR(e), rho, FALSE);
      string func = appl2("findFun",
			  make_symbol(CAR(e)),
			  rho);
      /* Unlike other functions,
       * applyClosure uses its 'call' argument. */
      string call = appl2("cons", func, args.var);
      // !!!!! TODO: self-allocate 'call'
      out = appl5("applyClosure",
		  call,
		  func,
		  args.var,
		  rho,
		  "R_NilValue");
      defs += unp(call);
      defs += unp(func);
      del(args);
      string func_name = string(CHAR(PRINTNAME(CAR(e))));
      //bool fun_visible = vis_map.find(func_name)->second;
      return Expression(out, TRUE, TRUE, unp(out));
      // Punt here; can't easily tell whether the result of the closure
      // is visible, so we just call it visible
    }
    /* It's a built-in function, special function, or closure */
    if (TYPEOF(op) == SPECIALSXP) {
      return op_special(e, op, rho);
    } else if (TYPEOF(op) == BUILTINSXP) {
      Expression op1 = op_exp(op, rho);
      //Expression args1 = op_exp(CDR(e), rho);
      Expression args1 = op_list_array(CDR(e), rho, FALSE);
      out = appl4(get_name(PRIMOFFSET(op)),
		  "R_NilValue ",
		  op1.var,
		  args1.var,
		  rho);
      del(op1);
      del(args1);
      return Expression(out, TRUE, 1 - PRIMPRINT(op), unp(out));
    } else if (TYPEOF(op) == CLOSXP) {
      Expression op1;
      if (CAR(BODY(op)) == install(".Internal")) {
	op1 = op_exp(op, "R_GlobalEnv");
      } else {
	op1 = op_exp(op, rho);
      }
      Expression out_exp = op_clos_app(op1, CDR(e), rho);
      return out_exp;
    } else {
      err("Internal error: LANGSXP encountered non-function op");
      return bogus_exp; // never reached
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
    SubexpBuffer temp("tmp_" + i_to_s(global_temps++) + "_");
    e = temp.op_exp(CAR(exp), rho);
    defs += "{\n";
    defs += temp.output();
    if (CDR(exp) == R_NilValue) {
      defs += indent(var + " = " + e.var + ";\n");
    } else {
      del(e);
    }
    defs += "}\n";
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
    Expression cond = op_exp(CAR(e), rho);
    string out = new_sexp();
    defs += "if (my_asLogicalNoNA(" + cond.var + ")) {\n";
    Expression te = op_exp(CADR(e), rho);
    defs += indent("PROTECT(" + out + " = " + te.var + ");\n");
    del(te);
    defs += "} else {\n";
    Expression fe = op_exp(CADDR(e), rho);
    defs += indent("PROTECT(" + out + " = " + fe.var + ");\n");
    del(fe);
    defs += "}\n";
    del(cond);
    return Expression(out, FALSE, TRUE, unp(out));
  } else {
    Expression cond = op_exp(CAR(e), rho);
    string out = new_sexp();
    defs += "if (my_asLogicalNoNA(" + cond.var + ")) {\n";
    Expression te = op_exp(CADR(e), rho);
    defs += indent("PROTECT(" + out + " = " + te.var + ");\n");
    del(te);
    defs += "} else {\n";
    defs += indent("PROTECT(" + out + " = R_NilValue);\n");
    defs += "}\n";
    del(cond);
    return Expression(out, FALSE, TRUE, unp(out));
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
  defs += "defineVar(" + make_symbol(sym) + ", R_NilValue, " + rho + ");\n";
  defs += "if (isList(" + val1.var + ") || isNull(" + val1.var + ")) {\n";
  defs += indent("n = length(" + val1.var + ");\n");
  defs += "PROTECT_WITH_INDEX(v = R_NilValue, &vpi);\n";
  defs += "} else {\n;";
  defs += indent("n = LENGTH(" + val1.var + ");\n");
  defs += "PROTECT_WITH_INDEX(v = allocVector(TYPEOF(" + val1.var + "), 1), &vpi);\n";
  defs += "}\n";
  defs += "ans = R_NilValue;\n";
  defs += "PROTECT_WITH_INDEX(ans, &api);\n";
  defs += "begincontext(&cntxt, CTXT_LOOP, R_NilValue, " + rho
    + ", R_NilValue, R_NilValue);\n";
  defs += "switch (SETJMP(cntxt.cjmpbuf)) {\n";
  string lab = global_labels.new_var();
  defs += "case CTXT_BREAK: goto for_break_" + lab + ";\n";
  defs += "case CTXT_NEXT: goto for_next_" + lab + ";\n";
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
  in_loop += "default: errorcall(R_NilValue, \"Bad for loop sequence\");\n";
  in_loop += "}\n";
  defs += indent(in_loop);
  Expression ans = op_exp(body, rho);
  defs += "REPROTECT(ans = " + ans.var + ", api);\n";
  del(ans);
  defs += "for_next_" + lab + ":\n";
  defs += ";\n";
  defs += "}\n";
  defs += "for_break_" + lab + ":\n";
  defs += "endcontext(&cntxt);\n";
  del(val1);
  defs += "UNPROTECT_PTR(v);\n";
  return Expression(ans.var, TRUE, FALSE, unp(ans.var));
}
  
Expression SubexpBuffer::op_while(SEXP e, string rho) {
  return bogus_exp;
}
  
Expression SubexpBuffer::op_fundef(SEXP e, string rho) {
  string func_name = global_fundefs.new_var();
  global_fundefs.defs += make_fundef_argslist(func_name,
					      CAR(e),
					      CADR(e));
  Expression formals = op_literal(CAR(e), rho);
  string func_sym = global_constants.appl1("mkString",
					   quote(func_name));
  Expression c_f_args = op_arglist(CAR(e), rho);
  string c_args = appl2("cons", rho, c_f_args.var);
  del(c_f_args);
  string c_call = appl2("cons", func_sym, c_args);
  defs += unp(c_args);
  string r_call = appl2("lcons",
			make_symbol(install(".External")),
			c_call);
  defs += unp(c_call);
  string out = appl3("mkCLOSXP ", formals.var, r_call, rho);
  del(formals);
  defs += unp(r_call);
  return Expression(out, TRUE, FALSE, unp(out));
}

Expression SubexpBuffer::op_special(SEXP e, SEXP op, string rho) {
  string out;
  if (PRIMFUN(op) == (SEXP (*)())do_set) {
    return op_set(e, op, rho);
  } else if (PRIMFUN(op) == (SEXP (*)())do_internal) {
    SEXP fun = CAR(CADR(e));
    Expression args;
    if (TYPEOF(INTERNAL(fun)) == BUILTINSXP) {
      Expression list = op_exp(CDR(CADR(e)), rho);
      string v = appl2("evalList", list.var, rho);
      args = Expression(v, TRUE, FALSE, unp(v));
      del(list);
    } else {
      args = op_exp(CDR(CADR(e)), rho);
    }
    Expression func = op_exp(INTERNAL(fun), rho);
    out = appl4(get_name(PRIMOFFSET(INTERNAL(fun))),
		"R_NilValue", func.var, args.var, rho);
    del(func);
    del(args);
    return Expression(out, TRUE, FALSE, unp(out));
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
  } else {
    /* default case for specials: call the (call, op, args, rho) fn */
    Expression op1 = op_exp(op, rho);
    Expression args1 = op_list(CDR(e), rho);
    out = appl4(get_name(PRIMOFFSET(op)),
		"R_NilValue",
		op1.var,
		args1.var,
		rho);
    del(op1);
    del(args1);
    return Expression(out, TRUE, 1 - PRIMPRINT(op), unp(out));
  }
}
  
Expression SubexpBuffer::op_set(SEXP e, SEXP op, string rho) {
  string out;
  if (PRIMVAL(op) == 1 || PRIMVAL(op) == 3) {    /* <-, = */
    if (isString(CADR(e))) {
      SETCAR(CDR(e), install(CHAR(STRING_ELT(CADR(e), 0))));
    }
    if (isSymbol(CADR(e))) {
      string name = make_symbol(CADR(e));
      Expression body = op_exp(CADDR(e), rho);
      defs += "defineVar(" + name + ", " 
	+ body.var + ", " + rho + ");\n";
      del(body);
      return Expression(name, TRUE, FALSE, "");
    } else if (isLanguage(CADR(e))) {
      Expression func = op_exp(op, rho);
      Expression args = op_literal(CDR(e), rho);
      out = appl4("do_set",
		  "R_NilValue",
		  func.var,
		  args.var,
		  rho);
      del(func);
      del(args);
      return Expression(out, TRUE, FALSE, unp(out));
    } else {
      global_ok = 0;
      return Expression("<<assignment with unrecognized LHS>>",
			TRUE, FALSE, "");
    }
  } else if (PRIMVAL(op) == 2) {  /* <<- */
    Expression op1 = op_exp(op, rho);
    Expression args1 = op_list(CDR(e), rho);
    out = appl4(get_name(PRIMOFFSET(op)),
		"R_NilValue",
		op1.var,
		args1.var,
		rho);
    del(op1);
    del(args1);
    return Expression(out, TRUE, FALSE, unp(out));
  } else {
    global_ok = 0;
    return Expression("<<Assignment of a type not yet implemented>>",
		      TRUE, FALSE, "");
  }
}
  
/* Output an application of a closure to arguments. */
Expression SubexpBuffer::op_clos_app(Expression op1, SEXP args, string rho) {
  /* see eval.c:438-9 */
  Expression args1 = op_literal(args, rho);
  string arglist = appl2("promiseArgs",
			 args1.var,
			 rho);
  /* Unlike most functions of its type,
   * applyClosure uses its 'call' argument. */
  string call = appl2("lcons", op1.var, args1.var);
  del(args1);
  string out = appl5("applyClosure ",
		     call,
		     op1.var,
		     arglist,
		     rho,
		     "R_NilValue");
  defs += unp(call);
  del(op1);
  defs += unp(arglist);
  return Expression(out, TRUE, op1.is_visible, unp(out));
}
  
/* Output the argument list for an external function.
   */
Expression SubexpBuffer::op_arglist(SEXP e, string rho) {
  int i;
  string out, tmp, tmp1;
  SEXP arg;
  int len = Rf_length(e);
  if (len == 0) return nil_exp;
  SEXP *args = new SEXP[len];
  SubexpBuffer buf("tmp_arglist_" + i_to_s(global_temps++) + "_");
    
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
      buf.defs += unp(tmp);
      tmp = tmp1;
    }
  }
  delete [] args;
  out = new_sexp_unp();
  defs += "{\n";
  defs += indent(buf.output());
  defs += indent(out + " = " + tmp + ";\n");
  defs += "}\n";
  return Expression(out, TRUE, FALSE, unp(out));
}


Expression SubexpBuffer::op_arglist_array(SEXP e, string rho) {
  if (e == R_NilValue) return nil_exp;
  int i, len;
  string list = new_sexp_unp();
  len = Rf_length(e);
  if (len == 0) return nil_exp;
  alloc_list.add(list, defs.length(), len);
  for(i=0; i<len; i++) {
    defs += "CAR(" + list + "-" + i_to_s(i) + ") = " 
      + make_symbol(TAG(e)) + ";\n";
    e = CDR(e);
  }
  return Expression(list, TRUE, FALSE, "");
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
    return Expression("<<unimplemented vector>>", TRUE, FALSE, "");
    break;
  case SYMSXP:
    return Expression(make_symbol(e), FALSE, TRUE, "");
    break;
  case LISTSXP:
    return op_list(e, rho);
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
      return Expression(v, FALSE, FALSE, "");
    } else {
      return Expression(v, TRUE, FALSE, unp(v));
    }
    break;
  case ENVSXP:
    global_ok = 0;
    return Expression("<<unexpected environment>>", TRUE, FALSE, "");
    break;
  case PROMSXP:
    global_ok = 0;
    return Expression("<<unexpected promise>>", TRUE, FALSE, "");
    break;
  case LANGSXP:
    return op_list(e, rho);
    break;
  case CHARSXP:
    v = appl1_unp("mkChar", quote(string(CHAR(e))));
    return Expression(v, TRUE, TRUE, "");
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
		      TRUE, FALSE, "");
    break;
  default:
    err("Internal error: op_literal encountered bad type\n");
    return bogus_exp; // never reached
    break;
  }
}

Expression SubexpBuffer::op_list_array(SEXP e, string rho, bool literal) {
  if (TYPEOF(e) != LISTSXP && TYPEOF(e) != LANGSXP && TYPEOF(e) != NILSXP) {
    err("non-list found in op_list_array\n");
  }
  if (!global_self_allocate) return op_list(e, rho, literal); 
  bool list_dep = FALSE;
  if (e == R_NilValue) return nil_exp;
  int i;
  string list = new_sexp();
  int len = Rf_length(e);
  if (len == 0) return nil_exp;
  string set_list, dt;
  for(i=0; i<len; i++) {
    Expression car_exp;
    if (literal) {
      car_exp = op_literal(CAR(e), rho);
    } else {
      car_exp = op_exp(CAR(e), rho);
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
  alloc_list.add(list, defs.length(), len);
  defs += set_list;
  Expression out = Expression(list, list_dep, TRUE, dt);
  out.is_alloc = TRUE;
  return out;
}

Expression SubexpBuffer::op_list(SEXP e, string rho, bool literal = TRUE) {
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
    Expression car = (literal ? op_literal(CAR(e), rho) : op_exp(CAR(e), rho));
    SubexpBuffer out_buf("tmp_ls_" + i_to_s(global_temps++) + "_");
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
      return Expression(out, TRUE, TRUE, unp(out));
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
      return Expression(out, FALSE, TRUE, "");
    }
  } else {  // length >= 2
    string unp_cars = "";
    bool list_dep = FALSE;
    Expression exps[len], tags[len];
    bool langs[len];
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
      exps[i] = (literal? op_literal(CAR(tmp_e), rho) : op_exp(CAR(tmp_e), rho));
      tags[i] = (TAG(tmp_e) == R_NilValue ? Expression("", FALSE, FALSE, "") : op_literal(TAG(tmp_e), rho));
      if (exps[i].is_dep) list_dep = TRUE;
      tmp_e = CDR(tmp_e);
    }
    SubexpBuffer tmp_buf("tmp_ls_" + i_to_s(global_temps++) + "_");
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
    if (list_dep) {
      string handle = new_sexp();
      defs += "{\n";
      defs += indent(tmp_buf.output());
      defs += indent("PROTECT(" + handle + " = " + cdr + ");\n");
      defs += indent(unp_cars);
      defs += "}\n";
      return Expression(handle, list_dep, TRUE, unp(handle));
    } else {
      string handle = global_constants.new_sexp();
      global_constants.defs += "{\n";
      global_constants.defs += indent(tmp_buf.output());
      global_constants.defs += indent("PROTECT(" + handle + " = " + cdr + ");\n");
      global_constants.defs += indent(unp_cars);
      global_constants.defs += "}\n";
      return Expression(handle, list_dep, TRUE, "");
    }
  }
}

/*
Expression SubexpBuffer::op_list(SEXP e, string rho, bool literal = TRUE) {
  SubexpBuffer temp_f("tmp_" + i_to_s(global_temps++) + "_");
  SubexpBuffer temp_c("tmp_" + i_to_s(global_temps++) + "_");
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
      return Expression(var_f, TRUE, TRUE, unp(var_f));
    }
  } else {
    return Expression(var_c, FALSE, TRUE, "");
  }
}
*/

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
      return Expression(out, TRUE, TRUE, "");
    } else if (!car.is_dep && !cdr.is_dep) {
      out_const = consts.appl2_unp(my_cons, car.var, cdr.var);
      consts.del(car);
      consts.del(cdr);
      return Expression(out_const, FALSE, TRUE, "");
      /* If this is dependent but some subexpression is constant, create
       * the bridge between the constant subexpression and the global 
       * constants. */
    } else if (car.is_dep && !cdr.is_dep) {
      string out = appl2_unp(my_cons, car.var, cdr.var);
      del(car);
      del(cdr);
      return Expression(out, TRUE, TRUE, "");
    } else if (!car.is_dep && cdr.is_dep) {
      string out = appl2_unp(my_cons, car.var, cdr.var);
      del(car);
      del(cdr);
      return Expression(out, TRUE, TRUE, "");
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
      return Expression(out, TRUE, TRUE, "");
    } else if (!car.is_dep && !tag.is_dep && !cdr.is_dep) {
      out_const = consts.appl3_unp("tagged_cons ", car.var, tag.var, cdr.var);
      consts.del(car);
      consts.del(tag);
      consts.del(cdr);
      return Expression(out_const, FALSE, TRUE, "");
    } else {
      global_constants.defs += consts.output();
      string out = appl3_unp("tagged_cons  ", car.var, tag.var, cdr.var);
      del(car);
      del(tag);
      del(cdr);
      return Expression(out, TRUE, TRUE, "");
    }
  }
  return bogus_exp; //never reached
}
  
Expression SubexpBuffer::op_string(SEXP s) {
  int i, len;
  string str = "";
  len = Rf_length(s);
  for(i=0; i<len; i++) {
    str += string(CHAR(STRING_ELT(s, i)));
  }
  string out = global_constants.appl1("mkString", 
				      quote(escape(str)));
  return Expression(out, FALSE, TRUE, "");
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
	return Expression(var, FALSE, TRUE, "");
      } else {
	return Expression(pr->second, FALSE, TRUE, "");
      }
    } else {
      global_ok = 0;
      return Expression("<<unimplemented logical vector>>",
			FALSE, FALSE, "");
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
	return Expression(var, FALSE, TRUE, "");
      } else {
	return Expression(pr->second, FALSE, TRUE, "");
      }
    } else {
      global_ok = 0;
      return Expression("<<unimplemented integer vector>>",
			FALSE, FALSE, "");
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
	return Expression(var, FALSE, TRUE, "");
      } else {
	return Expression(pr->second, FALSE, TRUE, "");
      }
    } else {
      global_ok = 0;
      return Expression("<<unimplemented real vector>>",
			FALSE, FALSE, "");
    }
    break;
  case CPLXSXP:
    if (len == 1) {
      Rcomplex value = COMPLEX(vec)[0];
      string var = global_constants.appl1("ScalarComplex",
					  c_to_s(value));
      return Expression(var, FALSE, TRUE, "");
    } else {
      global_ok = 0;
      return Expression("<<unimplemented complex vector>>",
			FALSE, FALSE, "");
    }
    break;
  default:
    err("Internal error: op_vector encountered bad vector type\n");
    return bogus_exp; // not reached
    break;
  }
}

/* Huge functions are hard on compilers like gcc. To generate code
 * that goes down easy, we split up the constant initialization into
 * several functions.
 */
unsigned int SplitSubexpBuffer::get_n_inits() {
  return init_fns;
}
string SplitSubexpBuffer::get_init_str() {
  return init_str;
}
string SplitSubexpBuffer::new_var() {
  prot++;
  return new_var_unp();
}  
string SplitSubexpBuffer::new_var_unp() {
  if ((n % threshold) == 0) {
    decls += "void " + init_str + i_to_s(init_fns) + "();\n";
    if (n != 0) defs += "}\n";
    defs += "void " + init_str + i_to_s(init_fns) + "() {\n";
    init_fns++;
  }
  return prefix + i_to_s(n++);
}

string SubexpBuffer::output() {
  string out;
  output_ip();
  out += decls;
  out += defs;
  return out;
}

void SubexpBuffer::output_ip() {
  int i;
  list<AllocListElem> ls = alloc_list.get();
  list<AllocListElem>::iterator p;
  list<VarRef>::iterator q;
  int offset = 0;
  for(p = ls.begin(), i=0; p != ls.end(); p++, i++) {
    string var = "mem" + i_to_s(i);
    decls += "SEXP " + var + ";\n";
    /*
      string alloc = var + " = malloc(" + i_to_s(p->max) + "*sizeof(SEXPREC));\n";
    if (p->max == 1) {
      alloc += "TYPEOF(" + var + ") = LISTSXP;\n";
      alloc += "CDR(" + var + ") = R_NilValue;\n";
      alloc += "TAG(" + var + ") = R_NilValue;\n";
      alloc += "ATTRIB(" + var + ") = R_NilValue;\n";
    } else if (p->max == 2) {
      alloc += "TYPEOF(" + var + "+1) = LISTSXP;\n";
      alloc += "CDR(" + var + "+1) = " + var + ";\n";
      alloc += "TAG(" + var + "+1) = R_NilValue;\n";
      alloc += "ATTRIB(" + var + "+1) = R_NilValue;\n";
      alloc += "TYPEOF(" + var + ") = LISTSXP;\n";
      alloc += "CDR(" + var + ") = R_NilValue;\n";
      alloc += "TAG(" + var + ") = R_NilValue;\n";
      alloc += "ATTRIB(" + var + ") = R_NilValue;\n";
    } else {
      if (!has_i) decls += "int i;\n";
      alloc += "for(i=0; i<" + i_to_s(p->max) + "; i++) {\n";
      alloc += indent("TYPEOF(" + var + "+i) = LISTSXP;\n");
      alloc += indent("CDR(" + var + "+i) = " + var + "+i-1;\n");
      alloc += indent("TAG(" + var + "+i) = R_NilValue;\n");
      alloc += indent("ATTRIB(" + var + "+i) = R_NilValue;\n");
      alloc += "}\n";
      alloc += "CDR(" + var + ") = R_NilValue;\n";
    }
    */
    string alloc = var + "= alloca(" + i_to_s(p->max) + "*sizeof(SEXPREC));\n";
    alloc += "my_init_memory(" + var + ", " + i_to_s(p->max) + ");\n";
    defs.insert(0, alloc);
    offset += alloc.length();
    for(q = p->vars.begin(); q != p->vars.end(); q++) {
      string ref = q->name + " = " + var + "+" + i_to_s(q->size - 1) + ";\n";
      defs.insert(q->location + offset, ref);
      offset += ref.length();
    }
    //defs += "UNPROTECT_PTR(" + var + "+" + i_to_s(p->max - 1) + ");\n"; // ?????
  }
}

int main(int argc, char *argv[]) {
  unsigned int i, num_exps;
  list<SEXP> e;
  string fullname, libname, out_filename, path, exprs;
  if (argc != 2 && argc != 3) {
    cerr << "Usage: rcc file [output-file]\n";
    exit(1);
  }
  if (strcmp(argv[1], "--") == 0) {
    num_exps = parse_R(e, NULL);
    libname = "R_output";
  } else {
    num_exps = parse_R(e, argv[1]);
    fullname = string(argv[1]);
    int pos = filename_pos(fullname);
    path = fullname.substr(0,pos);
    libname = strip_suffix(fullname.substr(pos, fullname.size() - pos));
  }
  if (argc == 3) {
    out_filename = string(argv[2]);
  } else {
    out_filename = path + libname + ".c";
  }

  /* build expressions */
  global_constants.decls += "void exec();\n";
  exprs += "}\n";
  exprs += "void exec() {\n";
  for(i=0; i<num_exps; i++) {
    exprs += indent("SEXP e" + i_to_s(i) + ";\n");
  }
  for(i=0; i<num_exps; i++) {
    SubexpBuffer subexps;
    SEXP sexp = e.front();
    Expression exp = subexps.op_exp(sexp, "R_GlobalEnv");
    e.pop_front();
    exprs += indent("{\n");
    exprs += indent(indent(subexps.output()));
    exprs += indent(indent("e" + i_to_s(i) + " = " + exp.var + ";\n"));
    if (exp.is_visible) {
      exprs += indent(indent("PrintValueRec(e" + i_to_s(i) + ", R_GlobalEnv);\n"));
    }
    exprs += exp.del_text;
    exprs += indent("}\n");
  }
  exprs += indent("UNPROTECT(" + i_to_s(global_constants.get_n_prot())
		  + "); /* c_ */\n");
  exprs += "}\n\n";

  string header;
  header += "void R_init_" + libname + "() {\n";
  /* The name R_init_<libname> causes the R dynamic loader to execute the
   * function immediately. */
  for(i=0; i<global_constants.get_n_inits(); i++) {
    header += indent(global_constants.get_init_str() + i_to_s(i) + "();\n");
  }
  header += indent("exec();\n");
  header += "}\n";
  
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
  
  /* output to file */
  out_file << "#include <IOStuff.h>\n";
  out_file << "#include <Parse.h>\n";
  out_file << "#include <Internal.h>\n";
  out_file << "#include <R_ext/RConverters.h>\n";
  out_file << "#include <../main/arithmetic.h>\n";
  out_file << "#include \"rcc_lib.h\"\n";
  out_file << "\n";
  global_fundefs.output_ip();
  global_constants.output_ip();
  out_file << global_fundefs.decls;
  out_file << global_constants.decls;
  out_file << header;
  out_file << indent(global_constants.defs);
  out_file << exprs;
  out_file << global_fundefs.defs;
  if (!out_file) {
    err("Couldn't write to file " + out_filename);
  }
  if (global_ok) {
    return 0;
  } else {
    return 1;
  }
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
      string var = global_constants.appl1_unp("install", quote(name));
      symbol_map.insert(pair<string,string>(name, var));
      return var;
    } else {
      return pr->second;
    }
  }
}

string make_type(int t) {
  switch(t) {
  case NILSXP: return "0 /* NILSXP */";
  case SYMSXP: return "1 /* SYMSXP */";
  case LISTSXP: return "2 /* LISTSXP */";
  case CLOSXP: return "3 /* CLOSXP */";
  case ENVSXP: return "4 /* ENVSXP */";
  case PROMSXP: return "5 /* PROMSXP */";
  case LANGSXP: return "6 /* LANGSXP */";
  case SPECIALSXP: return "7 /* SPECIALSXP */";
  case BUILTINSXP: return "8 /* BUILTINSXP */";
  case CHARSXP: return "9 /* CHARSXP */";
  case LGLSXP: return "10 /* LGLSXP */";
  case INTSXP: return "13 /* INTSXP */";
  case REALSXP: return "14 /* REALSXP */";
  case CPLXSXP: return "15 /* CPLXSXP */";
  case STRSXP: return "16 /* STRSXP */";
  case DOTSXP: return "17 /* DOTSXP */";
  case ANYSXP: return "18 /* ANYSXP */";
  case VECSXP: return "19 /* VECSXP */";
  case EXPRSXP: return "20 /* EXPRSXP */";
  case BCODESXP: return "21 /* BCODESXP */";
  case EXTPTRSXP: return "22 /* EXTPTRSXP */";
  case WEAKREFSXP: return "23 /* WEAKREFSXP */";
  case FUNSXP: return "99 /* FUNSXP */";
  default: err("make_type: invalid type"); return "BOGUS";
  }
}

string make_fundef(string func_name, SEXP args, SEXP code) {
  int i;
  string f, header;
  SEXP temp_args = args;
  int len = Rf_length(args);
  SubexpBuffer out_subexps, env_subexps;
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
  f += indent(indent("begincontext(&context, CTXT_RETURN, R_NilValue, newenv, env, R_NilValue);\n"));
  Expression outblock = out_subexps.op_exp(code, "newenv");
  f += indent(indent("{\n"));
  f += indent(indent(indent(out_subexps.output())));
  f += indent(indent(indent("out = " + outblock.var + ";\n")));
  out_subexps.del(outblock); // ?????
  f += indent(indent("}\n"));
  f += indent(indent("endcontext(&context);\n"));
  f += indent("}\n");
  f += indent("UNPROTECT(" + i_to_s(env_nprot) + ");\n");
  f += indent("return out;\n");
  f += "}\n";
  delete [] global_formals;
  global_formals = old_formals;
  global_formals_len = old_formals_len;
  return f;
}

/* Make a function where the arguments of the R function are packed in
 * a list to form a single f-function argument, as opposed to
 * make_fundef where the mapping is one-to-one. This version is used
 * for functions that include the "..." object.
 */
string make_fundef_argslist(string func_name, SEXP args, SEXP code) {
  string f, header;
  SubexpBuffer out_subexps, env_subexps;
  string * old_formals;
  int old_formals_len;
  old_formals = global_formals;
  old_formals_len = global_formals_len;
  global_formals = NULL;
  global_formals_len = 0;
  header = "SEXP " + func_name + "(";
  header += "SEXP full_args)";
  global_fundefs.decls += header + ";\n";
  f += header + " {\n";
  f += indent("SEXP newenv;\n");
  f += indent("SEXP out;\n");
  f += indent("SEXP env;\n");
  f += indent("SEXP args;\n");
  f += indent("RCNTXT context;\n");
  Expression formals = env_subexps.op_symlist(args, "env");
  string actuals = "args";
  //  int env_nprot = env_subexps.get_n_prot() + 3;
  env_subexps.output_ip();
  f += indent(env_subexps.decls);
  f += indent("PROTECT(env = CADR(full_args));\n");
  f += indent("PROTECT(args = CDDR(full_args));\n");
  f += indent(env_subexps.defs);
  f += indent("PROTECT(newenv =\n");
  f += indent(indent("Rf_NewEnvironment(\n"
		     + indent(formals.var) + ",\n"
		     + indent(actuals) + ",\n"
		     + indent("env") + "));\n"));
  f += indent("if (SETJMP(context.cjmpbuf)) {\n");
  //f += indent(indent("UNPROTECT(1);\n"));
  f += indent(indent("PROTECT(out = R_ReturnedValue);\n"));
  f += indent("} else {\n");
  f += indent(indent("begincontext(&context, CTXT_RETURN, R_NilValue, newenv, env, R_NilValue);\n"));
  Expression outblock = out_subexps.op_exp(code, "newenv");
  f += indent(indent("{\n"));
  f += indent(indent(indent(out_subexps.output())));
  f += indent(indent(indent("PROTECT(out = " + outblock.var + ");\n")));
  f += indent(indent(indent(outblock.del_text)));
  f += indent(indent("}\n"));
  f += indent(indent("endcontext(&context);\n"));
  f += indent("}\n");
  //  f += indent("UNPROTECT(" + i_to_s(env_nprot) + ");\n");
  f += indent(unp("env"));
  f += indent(unp("args"));
  f += indent(unp("newenv"));
  f += indent(formals.del_text);
  f += indent(unp("out"));
  f += indent("return out;\n");
  f += "}\n";
  //vis_map[func_name] = outblock.is_visible;
  global_formals = old_formals;
  global_formals_len = old_formals_len;
  return f;
}

const string IND_STR = "  ";

string indent(string str) {
  if (str.empty()) return "";
  string newstr = IND_STR;   /* Add indentation to beginning */
  string::iterator it;
  /* Indent after every newline (unless there's one at the end) */
  for(it = str.begin(); it != str.end(); it++) {
    if (*it == '\n' && it != str.end() - 1) {
      newstr += '\n' + IND_STR;
    } else {
      newstr += *it;
    }
  }
  return newstr;
}

/* Rrrrrgh. C++: the language that makes the hard things hard and the
 * easy things hard.
 */
string i_to_s(const int i) {
  if (i == (int)0x80000000) {
    return "0x80000000"; /* Doesn't work as a decimal constant */
  } else {
    ostringstream ss;
    ss << i;
    return ss.str();
  }
}

string d_to_s(double d) {
  if (d == HUGE_VAL) {
    return "HUGE_VAL";
  } else {
    ostringstream ss;
    ss << d;
    return ss.str();
  }
}

string c_to_s(Rcomplex c) {
  return "mk_complex(" + d_to_s(c.r) + "," + d_to_s(c.i) + ")";
}

/* Escape "'s, \'s and \n's to represent a string in C code. */
string escape(string str) {
  unsigned int i;
  string out = "";
  for(i=0; i<str.size(); i++) {
    if (str[i] == '\n') {
      out += "\\n";
    } else if (str[i] == '"') {
      out += "\\\"";
    } else if (str[i] == '\\') {
      out += "\\\\";
    } else {
      out += str[i];
    }
  }
  return out;
}

/* Simple function to add quotation marks around a string */
string quote(string str) {
  return "\"" + str + "\"";
}

string unp(string str) {
  return "UNPROTECT_PTR(" + str + ");\n";
}

string strip_suffix(string name) {
  string::size_type pos = name.rfind(".", name.size());
  if (pos == string::npos) {
    return name;
  } else {
    return name.erase(pos, name.size() - pos);
  }
}

int filename_pos(string str) {
  string::size_type pos = str.rfind("/", str.size());
  if (pos == string::npos) {
    return 0;
  } else {
    return pos + 1;
  }
}

int parse_R(list<SEXP> & e, char *filename) {
  SEXP exp;
  int status;
  int num_exps = 0;
  FILE *inFile;
  char *myargs[5];
  myargs[0] = "/home/garvin/research/tel/rcc/rcc";
  myargs[1] = "--gui=none";
  myargs[2] = "--slave";
  myargs[3] = "--vanilla";
  myargs[4] = NULL;
  Rf_initialize_R(4,myargs);
  setup_Rmainloop();

  if (!filename) {
    inFile = stdin;
  } else {
    inFile = fopen(filename, "r");
  }
  if (!inFile) {
    cerr << "Error: input file \"" << filename << "\" not found\n";
    exit(1);
  }

  do {
    /* parse each expression */
    PROTECT(exp = R_Parse1File(inFile, 1, &status));
    switch(status) {
    case PARSE_NULL:
      break;
    case PARSE_OK:
      num_exps++;
      e.push_back(exp);
      break;
    case PARSE_INCOMPLETE:
      err("parsing returned PARSE_INCOMPLETE.\n");
      break;
    case PARSE_ERROR:
      err("parsing returned PARSE_ERROR.\n");
      break;
    case PARSE_EOF:
      break;
    }
  } while (status != PARSE_EOF);
  
  return num_exps;
}

void err(string message) {
  cerr << "Error: " << message;
  exit(1);
}

void printstr(string str) {
  cerr << str << endl;
}


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

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <list>

extern "C" {

#include <stdarg.h>
#include <stdlib.h>
#include <sys/file.h>
#include <IOStuff.h>
#include <Parse.h>
#include "get_name.h"

extern int Rf_initialize_R(int argc, char **argv);
extern void setup_Rmainloop(void);

}

using namespace std;

string itos(int i);

class SubexpBuffer {
protected:
  const string prefix;
  unsigned int n;
  unsigned int prot;
public:
  string decls;
  string defs;
  virtual string new_var() {
    prot++;
    return new_var_unp();
  }
  virtual string new_var_unp() {
    return prefix + itos(n++);
  }
  int get_n_vars() {
    return n;
  }
  int get_n_prot() {
    return prot;
  }
  SubexpBuffer(string pref = "v") : prefix(pref) {
    n = prot = 0;
    decls = defs = "";
  }
  virtual ~SubexpBuffer() {};
};

/* Huge functions are hard on compilers like gcc. To generate code
 * that goes down easy, we split up the constant initialization into
 * several functions.
 */
class SplitSubexpBuffer : public SubexpBuffer {
private:
  const unsigned int threshold;
  const string init_str;
  unsigned int init_fns;
public:
  unsigned int get_n_inits() {
    return init_fns;
  }
  string get_init_str() {
    return init_str;
  }
  string new_var() {
    prot++;
    return new_var_unp();
  }  
  string new_var_unp() {
    if ((n % threshold) == 0) {
      decls += "void " + init_str + itos(init_fns) + "();\n";
      if (n != 0) defs += "}\n";
      defs += "void " + init_str + itos(init_fns) + "()\n{\n";
      init_fns++;
    }
    return prefix + itos(n++);
  }
  SplitSubexpBuffer(string pref = "v", int thr = 500, string is = "init") 
    : SubexpBuffer(pref), threshold(thr), init_str(is) {
    init_fns = 0;
  }
};

/* Expression is a struct returned by the op_ functions.
 * DEP = a variable representing a PROTECTed subexpression that
 *       depends on new_env and therefore must be placed inside the
 *       f-function
 * NOPR = an expression that must be inside the f-function but is not
 *        PROTECTed (usually a function argument)
 * CON = a variable representing a PROTECTed constant expression that
 *       can be moved outside the f-function
 * STR = a plain expression that doesn't need to be PROTECTed, such as
 * 'install("foo")'.
 * This should be redone as two booleans: whether it's PROTECTed and whether
 * it depends on the environment.
 */
typedef enum {DEP, NOPR, CON, STR} Type;
struct Expression {
  Type type;
  string var;
  Expression() {
  }
  Expression(Type t, string v) {
    type = t;
    var = v;
  }
};

Expression op_exp(SEXP e, string rho, SubexpBuffer & subexps);
Expression op_primsxp(SEXP e, string rho, SubexpBuffer & subexps);
Expression op_symlist(SEXP e, string rho, SubexpBuffer & subexps);
Expression op_lang(SEXP e, string rho, SubexpBuffer & subexps);
Expression op_begin(SEXP exp, string rho, SubexpBuffer & subexps);
Expression op_fundef(SEXP e, string rho, SubexpBuffer & subexps);
Expression op_special(SEXP e, SEXP op, string rho, SubexpBuffer & subexps);
Expression op_set(SEXP e, SEXP op, string rho, SubexpBuffer & subexps);
Expression op_clos_app(Expression op1, SEXP args, string rho, SubexpBuffer & subexps);
Expression op_arglist(SEXP e, string rho, SubexpBuffer & subexps);
Expression op_literal(SEXP e, string rho, SubexpBuffer & subexps);
Expression op_list(SEXP e, string rho, SubexpBuffer & subexps, bool literal = TRUE);
Expression op_list_help(SEXP e, string rho, 
				SubexpBuffer & subexps, SubexpBuffer & consts, 
				string & out_const, bool literal);
Expression op_string(SEXP s, SubexpBuffer & subexps);
Expression op_vector(SEXP e, SubexpBuffer & subexps);
string make_symbol(SEXP e);
string make_fundef(string func_name, SEXP args, SEXP code);
string make_fundef_argslist(string func_name, SEXP args, SEXP code);
string indent(string str);
string dtos(double d);
string ctos(Rcomplex c);
string appl1(string func, string arg, SubexpBuffer & subexps);
string appl2(string func, string arg1, string arg2, SubexpBuffer & subexps,
	     bool unp_1 = FALSE, bool unp_2 = FALSE);
string appl2_unp(string func, string arg1, string arg2, SubexpBuffer & subexps,
		 bool unp_1 = FALSE, bool unp_2 = FALSE);
string appl3(string func, string arg1, string arg2, string arg3,
		SubexpBuffer & subexps,
	     bool unp_1 = FALSE, bool unp_2 = FALSE, bool unp_3 = FALSE);
string appl3_unp(string func, string arg1, string arg2, string arg3,
		 SubexpBuffer & subexps,
		 bool unp_1 = FALSE, bool unp_2 = FALSE, bool unp_3 = FALSE);
string appl4(string func, string arg1, string arg2, string arg3, 
		string arg4, SubexpBuffer & subexps);
string appl5(string func, string arg1, string arg2, string arg3, 
		string arg4, string arg5, SubexpBuffer & subexps);
string appl6(string func, string arg1, string arg2, string arg3, 
		string arg4, string arg5, string arg6, SubexpBuffer & subexps);
string escape(string str);
string quote(string str);
string strip_suffix(string str);
int filename_pos(string str);
int parse_R(list<SEXP> & e, char *inFile);
void err(string message);
void printstr(string str);

/* Determines whether to print out each result.
 *  1 : print
 *  0 : determine dynamically; print iff R_Visible is true
 * -1 : don't print
 */
static int global_visible = 0;
static bool global_ok = 1;
static unsigned int global_temps = 0;
static SubexpBuffer global_fundefs("f");
static SplitSubexpBuffer global_constants("c");
static map<string, string> symbol_map;
static map<double, string> sc_real_map;
static map<int, string> sc_logical_map;
static map<int, string> sc_integer_map;
static map<int, string> primsxp_map;
static string * global_formals = NULL;
static int global_formals_len = 0;

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
    exprs += indent("SEXP e" + itos(i) + ";\n");
  }
  for(i=0; i<num_exps; i++) {
    SubexpBuffer subexps;
    SEXP sexp = e.front();
    Expression exp = op_exp(sexp, "R_GlobalEnv", subexps);
    e.pop_front();
    exprs += indent("{\n");
    exprs += indent(indent(subexps.decls));
    exprs += indent(indent(subexps.defs));
    exprs += indent(indent("e" + itos(i) + " = " + exp.var + ";\n"));
    if (global_visible == 1) {
      exprs += indent(indent("PrintValueRec(e" + itos(i) + ", R_GlobalEnv);\n"));
    } else if (global_visible == -1) {
      /* do nothing */
    } else {
      exprs += indent(indent("if (R_Visible) PrintValueRec(e" + itos(i)
			     + ", R_GlobalEnv);\n"));
    }
    if (exp.type == DEP) {
      exprs += indent(indent("UNPROTECT_PTR(" + exp.var + ");\n"));
    }
    exprs += indent("}\n");
  }
  exprs += indent("UNPROTECT(" + itos(global_constants.get_n_prot())
		  + "); /* c_ */\n");
  exprs += "}\n\n";

  string header;
  header += "void R_init_" + libname + "() {\n";
  /* The name R_init_<libname> causes the R dynamic loader to execute the
   * function immediately. */
  for(i=0; i<global_constants.get_n_inits(); i++) {
    header += indent(global_constants.get_init_str() + itos(i) + "();\n");
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
  out_file << global_fundefs.decls;
  out_file << global_constants.decls;
  out_file << header;
  out_file << indent(global_constants.defs);
  out_file << exprs;
  out_file << global_fundefs.defs;
  if (global_ok) {
    return 0;
  } else {
    return 1;
  }
}

Expression op_exp(SEXP e, string rho, SubexpBuffer & subexps) {
  global_visible = 0;
  string sym, var;
  Expression out, formals, body, env;
  switch(TYPEOF(e)) {
  case NILSXP:
    return Expression(STR, "R_NilValue");
    break;
  case STRSXP:
    return op_string(e, subexps);
    break;
  case LGLSXP:
  case INTSXP:
  case REALSXP:
  case CPLXSXP:
    global_visible = 1;
    return op_vector(e, subexps);
    break;
  case VECSXP:
    global_ok = 0;
    return Expression(STR, "<<unimplemented vector>>");
    break;
  case SYMSXP:
    if (e == R_MissingArg) {
      return Expression(STR, "R_MissingArg");
    } else {
      if (global_formals != NULL && rho != "R_GlobalEnv") {
	int i;
	for(i=0; i<global_formals_len; i++) {
	  if (string(CHAR(PRINTNAME(e))) == global_formals[i]) {
	    return Expression(NOPR, "arg" + itos(i));
	  }
	}
      }
      sym = make_symbol(e);
      out = Expression(DEP, appl2("findVar", sym, rho, subexps));
      global_visible = 1;
      return out;
    }
    break;
  case LISTSXP:
    global_visible = 1;
    return op_list(e, rho, subexps, FALSE);
    break;
  case CLOSXP:
    formals = op_symlist(FORMALS(e), rho, subexps);
    body = op_literal(BODY(e), rho, subexps);
    if (rho == "R_GlobalEnv") {
      out = Expression(CON, appl3("mkCLOSXP",
				  formals.var,
				  body.var,
				  rho,
				  global_constants));
    } else {
      out = Expression(DEP, appl3("mkCLOSXP",
				  formals.var,
				  body.var,
				  rho,
				  subexps));
    }
    global_visible = -1;
    return out;
    break;
  case ENVSXP:
    global_visible = -1;
    global_ok = 0;
    return Expression(STR, "<<unexpected environment>>");
    break;
  case PROMSXP:
    global_visible = -1;
    global_ok = 0;
    return Expression(STR, "<<unexpected promise>>");
    break;
  case LANGSXP:
    out = op_lang(e, rho, subexps);
    return out;
    break;
  case CHARSXP:
    return Expression(CON, appl1("mkChar",
				 quote(string(CHAR(e))),
				 subexps));
    break;
  case SPECIALSXP:
  case BUILTINSXP:
    return op_primsxp(e, rho, global_constants);
  case EXPRSXP:
  case EXTPTRSXP:
  case WEAKREFSXP:
    global_ok = 0;
    return Expression(STR, 
		      "<<unimplemented type " + itos(TYPEOF(e)) + ">>");
    break;
  default:
    err("Internal error: op_exp encountered bad type\n");
    return Expression(STR, "BOGUS"); // never reached
    break;
  }
}

Expression op_primsxp(SEXP e, string rho, SubexpBuffer & subexps) {
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
    string var = subexps.new_var();
    subexps.decls += "SEXP " + var + ";\n";
    subexps.defs += "PROTECT(" + var
      + " = mkPRIMSXP(" + itos(PRIMOFFSET(e)) 
      + "," + itos(is_builtin) + "));"
      + " /* " + string(PRIMNAME(e)) + " */\n";
    primsxp_map.insert(pair<int,string>(value, var));
    return Expression(STR, var);
  } else {
    return Expression(STR, pr->second);
  }
}

Expression op_symlist(SEXP e, string rho, SubexpBuffer & subexps) {
  return op_list(e, rho, subexps);
}

Expression op_lang(SEXP e, string rho, SubexpBuffer & subexps) {
  SEXP op;
  string out;
  Expression exp;

  if (TYPEOF(CAR(e)) == SYMSXP) {
    op = findVar(CAR(e), R_GlobalEnv);
    if (op == R_UnboundValue) {  /* user-defined function */
      Expression args = op_exp(CDR(e), rho, subexps);
      string func = appl2("findFun",
			  make_symbol(CAR(e)),
			  rho,
			  subexps);
      /* Unlike other functions,
       * applyClosure uses its 'call' argument. */
      string call = appl2("cons", func, args.var, subexps);
      out = appl5("applyClosure",
		  call,
		  func,
		  args.var,
		  rho,
		  "R_NilValue",
		  subexps);
      subexps.defs += "UNPROTECT_PTR(" + func + ");\n";
      if (args.type == DEP) {
	subexps.defs += "UNPROTECT_PTR(" + args.var + ");\n";
      }
      subexps.defs += "UNPROTECT_PTR(" + call + ");\n";
      return Expression(DEP, out);
    }
    /* It's a built-in function, special function, or closure */
    if (TYPEOF(op) == SPECIALSXP) {
      return op_special(e, op, rho, subexps);
    } else if (TYPEOF(op) == BUILTINSXP) {
      Expression op1 = op_exp(op, rho, subexps);
      Expression args1 = op_exp(CDR(e), rho, subexps);
      out = appl4(get_name(PRIMOFFSET(op)),
		  "R_NilValue ",
		  op1.var,
		  args1.var,
		  rho,
		  subexps);
      if (args1.type == DEP) {
	subexps.defs += "UNPROTECT_PTR(" + args1.var + ");\n";
      }
      global_visible = PRIMPRINT(op) ? -1 : 1;
      return Expression(DEP, out);
    } else if (TYPEOF(op) == CLOSXP) {
      Expression op1;
      if (CAR(BODY(op)) == install(".Internal")) {
	op1 = op_exp(op, "R_GlobalEnv", subexps);
      } else {
	op1 = op_exp(op, rho, subexps);
      }
      Expression out_exp = op_clos_app(op1, CDR(e), rho, subexps);
      global_visible = -1;
      return out_exp;
    } else {
      err("Internal error: LANGSXP encountered non-function op");
      return Expression(STR, "BOGUS"); // never reached
    }
  } else {  // function is not a symbol
    Expression op1;
    op1 = op_exp(CAR(e), rho, subexps);
    Expression out_exp = op_clos_app(op1, CDR(e), rho, subexps);
    global_visible = -1;
    return out_exp;
    // eval.c: 395
  }
}

Expression op_begin(SEXP exp, string rho, SubexpBuffer & subexps) {
  Expression e;
  string var = subexps.new_var();
  subexps.decls += "SEXP " + var + ";\n";
  while (exp != R_NilValue) {
    SubexpBuffer temp("tmp_" + itos(global_temps++) + "_");
    e = op_exp(CAR(exp), rho, temp);
    subexps.defs += "{\n";
    subexps.defs += indent(temp.decls);
    subexps.defs += indent(temp.defs);
    if (CDR(exp) == R_NilValue) { 
      subexps.defs += indent(var + " = " + e.var + ";\n");
    }
    subexps.defs += "}\n";
    exp = CDR(exp);
  }
  return Expression(e.type, var);
}

Expression op_fundef(SEXP e, string rho, SubexpBuffer & subexps) {
  string func_name = global_fundefs.new_var();
  global_fundefs.defs += make_fundef_argslist(func_name,
					      CAR(e),
					      CADR(e));
  Expression formals = op_literal(CAR(e), rho, subexps);
  string func_sym = appl1("mkString",
			  quote(func_name),
			  global_constants);
  Expression c_f_args = op_arglist(CAR(e), rho, subexps);
  string c_args = appl2("cons", rho, c_f_args.var, subexps,
			FALSE, (c_f_args.type == DEP));
  string c_call = appl2("cons", func_sym, c_args, subexps, FALSE, TRUE);
  string r_call = appl2("lcons",
			make_symbol(install(".External")),
			c_call,
			subexps, FALSE, TRUE);
  string out = appl3("mkCLOSXP ", formals.var, r_call, rho, subexps);
  subexps.defs += "UNPROTECT_PTR(" + r_call + ");\n";
  return Expression(DEP, out);
}

Expression op_special(SEXP e, SEXP op, string rho, SubexpBuffer & subexps) {
  string out;
  if (PRIMFUN(op) == (SEXP (*)())do_set) {
    return op_set(e, op, rho, subexps);
  } else if (PRIMFUN(op) == (SEXP (*)())do_internal) {
    SEXP fun = CAR(CADR(e));
    Expression args;
    if (TYPEOF(INTERNAL(fun)) == BUILTINSXP) {
      Expression list = op_exp(CDR(CADR(e)), rho, subexps);
      args = Expression(DEP, appl2("evalList", list.var, rho, subexps));
    } else {
      args = op_exp(CDR(CADR(e)), rho, subexps);
    }
    Expression func = op_exp(INTERNAL(fun), rho, subexps);
    global_visible = -1;
    return Expression(DEP,
		      appl4(get_name(PRIMOFFSET(INTERNAL(fun))),
			    "R_NilValue",
			    func.var,
			    args.var,
			    rho,
			    subexps));
  } else if (PRIMFUN(op) == (SEXP (*)())do_function) {
    return op_fundef(CDR(e), rho, subexps);
  } else if (PRIMFUN(op) == (SEXP (*)())do_begin) {
    return op_begin(CDR(e), rho, subexps);
  } else {
    /* default case for specials: call the (call, op, args, rho) fn */
    /* do_for currently lands here */
    Expression op1 = op_exp(op, rho, subexps);
    Expression args1 = op_list(CDR(e), rho, subexps);
    out = appl4(get_name(PRIMOFFSET(op)),
		"R_NilValue",
		op1.var,
		args1.var,
		rho,
		subexps);
    //global_visible = PRIMPRINT(op) ? -1 : 1;
    global_visible = 0;
    return Expression(DEP, out);
  }
}

Expression op_set(SEXP e, SEXP op, string rho, SubexpBuffer & subexps) {
  string out;
  if (PRIMVAL(op) == 1 || PRIMVAL(op) == 3) {    /* <-, = */
    if (isString(CADR(e))) {
      SETCAR(CDR(e), install(CHAR(STRING_ELT(CADR(e), 0))));
    }
    if (isSymbol(CADR(e))) {
      string name = make_symbol(CADR(e));
      Expression body = op_exp(CADDR(e), rho, subexps);
      subexps.defs += "defineVar(" + name + ", " 
	+ body.var + ", " + rho + ");\n";
      if (body.type == DEP) {
	subexps.defs += "UNPROTECT_PTR(" + body.var + ");\n";
      }
      global_visible = -1;
      return Expression(STR, name);
    } else if (isLanguage(CADR(e))) {
      Expression func = op_exp(op, rho, subexps);
      Expression args = op_literal(CDR(e), rho, subexps);
      out = appl4("do_set",
		  "R_NilValue",
		  func.var,
		  args.var,
		  rho,
		  subexps);
      global_visible = -1;
      return Expression(DEP, out);
    } else {
      global_ok = 0;
      return Expression(STR, "<<assignment with unrecognized LHS>>");
    }
  } else if (PRIMVAL(op) == 2) {  /* <<- */
    Expression op1 = op_exp(op, rho, subexps);
    Expression args1 = op_list(CDR(e), rho, subexps);
    out = appl4(get_name(PRIMOFFSET(op)),
		"R_NilValue",
		op1.var,
		args1.var,
		rho,
		subexps);
    global_visible = -1;
    return Expression(DEP, out);
  } else {
    global_ok = 0;
    return Expression(STR, 
		      "<<Assignment of a type not yet implemented>>");
  }
}

/* Output an application of a closure to arguments. */
Expression op_clos_app(Expression op1, SEXP args, string rho, SubexpBuffer & subexps) {
  /* see eval.c:438-9 */
  Expression args1 = op_literal(args, rho, subexps);
  string arglist = appl2("promiseArgs",
			 args1.var,
			 rho,
			 subexps);
  /* Unlike most functions of its type,
   * applyClosure uses its 'call' argument. */
  string call = appl2("lcons", op1.var, args1.var, subexps,
		      (op1.type == DEP),
		      (args1.type == DEP));
  string out = appl5("applyClosure",
		     call,
		     op1.var,
		     arglist,
		     rho,
		     "R_NilValue",
		     subexps);
  subexps.defs += "UNPROTECT_PTR(" + arglist + ");\n";
  subexps.defs += "UNPROTECT_PTR(" + call + ");\n";
  return Expression(DEP, out);
}

/* Output the argument list for an external function.
 */
Expression op_arglist(SEXP e, string rho, SubexpBuffer & subexps) {
  int i;
  string out, tmp, tmp1;
  SEXP arg;
  int len = Rf_length(e);
  if (len == 0) return Expression(STR, "R_NilValue");
  SEXP args[len];
  SubexpBuffer buf("tmp_" + itos(global_temps++) + "_");

  arg = e;
  for(i=0; i<len; i++) {
    args[i] = arg;
    arg = CDR(arg);
  }

  /* Don't unprotect R_NilValue, just the conses */
  tmp = appl2("cons", make_symbol(TAG(args[len-1])), "R_NilValue", buf);
  if (len > 1) {
    for(i=len-2; i>=0; i--) {
      string sym = make_symbol(TAG(args[i]));
      tmp = appl2("cons", sym, tmp, buf, FALSE, TRUE);
    }
  }
  out = subexps.new_var_unp();
  subexps.decls += "SEXP " + out + ";\n";
  subexps.defs += "{\n";
  subexps.defs += indent(buf.decls);
  subexps.defs += indent(buf.defs);
  subexps.defs += indent(out + " = " + tmp + ";\n");
  subexps.defs += "}\n";
  return Expression(DEP, out);
}

Expression op_literal(SEXP e, string rho, SubexpBuffer & subexps) {
  Expression formals, body, env;
  switch (TYPEOF(e)) {
  case NILSXP:
    return Expression(STR, "R_NilValue");
    break;
  case STRSXP:
    return op_string(e, subexps);
    break;
  case LGLSXP:
  case INTSXP:
  case REALSXP:
  case CPLXSXP:
    global_visible = 1;
    return op_vector(e, subexps);
    break;
  case VECSXP:
    global_ok = 0;
    return Expression(STR, "<<unimplemented vector>>");
    break;
  case SYMSXP:
    return Expression(STR, make_symbol(e));
    break;
  case LISTSXP:
    return op_list(e, rho, subexps);
    break;
  case CLOSXP:
    formals = op_symlist(FORMALS(e), rho, subexps);
    body = op_literal(BODY(e), rho, subexps);
    return Expression(DEP, appl3("mkCLOSXP  ", 
				 formals.var,
				 body.var,
				 rho,
				 subexps));
    break;
  case ENVSXP:
    global_ok = 0;
    return Expression(STR, "<<unexpected environment>>");
    break;
  case PROMSXP:
    global_ok = 0;
    return Expression(STR, "<<unexpected promise>>");
    break;
  case LANGSXP:
    return op_list(e, rho, subexps);
    break;
  case CHARSXP:
    return Expression(STR, 
		      appl1("mkChar", quote(string(CHAR(e))), subexps));
    break;
  case SPECIALSXP:
  case BUILTINSXP:
    global_visible = 1 - PRIMPRINT(e);
    return op_primsxp(e, rho, global_constants);
  case EXPRSXP:
  case EXTPTRSXP:
  case WEAKREFSXP:
    global_ok = 0;
    return Expression(STR, 
		      "<<unimplemented type " + itos(TYPEOF(e)) + ">>");
    break;
  default:
    err("Internal error: op_literal encountered bad type\n");
    return Expression(STR, "BOGUS"); // never reached
    break;
  }
}

Expression op_list(SEXP e, string rho, SubexpBuffer & subexps,
			   bool literal) {
  SubexpBuffer temp_f("tmp_" + itos(global_temps++) + "_");
  SubexpBuffer temp_c("tmp_" + itos(global_temps++) + "_");
  string out_const, var_c;
  Expression exp = op_list_help(e, rho, temp_f, temp_c, out_const, literal);
  if (out_const != "") {
    var_c = global_constants.new_var();
    global_constants.decls += "SEXP " + var_c + ";\n";
    global_constants.defs += "{\n";
    global_constants.defs += indent(temp_c.decls);
    global_constants.defs += indent(temp_c.defs);
    global_constants.defs += 
      indent("PROTECT(" + var_c + " = " + out_const + ");\n");
    global_constants.defs += "}\n";
  }
  if (exp.type == DEP) {
    string var_f = subexps.new_var();
    subexps.decls += "SEXP " + var_f + ";\n";
    subexps.defs += "{\n";
    subexps.defs += indent(temp_f.decls);
    subexps.defs += indent(temp_f.defs);
    subexps.defs += indent("PROTECT(" + var_f + " =" + exp.var + ");\n");
    subexps.defs += "}\n";
    return Expression(DEP, var_f);
  } else if (exp.type == CON) {
    return Expression(STR, var_c);
  } else if (exp.type == STR) {
    return Expression(STR, exp.var);
  }
}

// Note: Does not protect the result, because it is often called directly
// before a cons, where it doesn't need to be protected.
Expression op_list_help(SEXP e, string rho,
			SubexpBuffer & subexps, SubexpBuffer & consts, 
			string & out_const, bool literal) {
  string my_cons;
  switch (TYPEOF(e)) {
  case NILSXP:
    return Expression(STR, "R_NilValue");
  case LISTSXP:
    my_cons = "cons";
    break;
  case LANGSXP:
    my_cons = "lcons";
    break;
  default:
    err("Internal error: bad call to op_list\n");
    return Expression(STR, "BOGUS");  // never reached
  }
  if (TAG(e) == R_NilValue) {
    Expression car;
    if (literal) {
      car = op_literal(CAR(e), rho, subexps);
    } else {
      car = op_exp(CAR(e), rho, subexps);
    }
    Expression cdr = op_list_help(CDR(e), rho, subexps, consts, 
					  out_const, literal);
    if (car.type == DEP || car.type == NOPR || cdr.type == DEP) {
      /* if this is dependent but some subexpression is constant, create
       * the bridge between the constant subexpression and the global 
       * constants. */
      if (car.type == CON || cdr.type == CON) {
	string var = global_constants.new_var();
	global_constants.decls += "SEXP " + var + ";\n";
	global_constants.defs += "{\n";
	global_constants.defs += consts.decls;
	global_constants.defs += consts.defs;
	if (car.type == CON) {
	  global_constants.defs += var + " =  " + car.var + ";\n";
	  car.var = var;
	}
	if (cdr.type == CON) {
	  global_constants.defs += var + " =  " + cdr.var + ";\n";
	  cdr.var = var;
	}
	global_constants.defs += "PROTECT(" + var + ");\n";
	global_constants.defs += "}\n";
      }
      return Expression(DEP, appl2_unp(my_cons,  // _unp
				       car.var,
				       cdr.var,
				       subexps,
				       (car.type == DEP),
				       FALSE));
    } else {
      out_const = appl2_unp(my_cons,  // _unp
			    car.var,
			    cdr.var,
			    consts,
			    (car.type == CON),
			    FALSE);
      return Expression(CON, out_const);
    }
  } else {
    if (my_cons == "lcons") {
      err("Internal error: op_list encountered tagged lcons\n");
    }
    Expression car;
    if (literal) {
      car = op_literal(CAR(e), rho, subexps);
    } else {
      car = op_exp(CAR(e), rho, subexps);
    }
    Expression tag = op_literal(TAG(e), rho, subexps);
    Expression cdr = op_list_help(CDR(e), rho, subexps, consts, 
				  out_const, literal);
    if (car.type == DEP || car.type == NOPR 
	|| tag.type == DEP || cdr.type == DEP) {
      if (car.type == CON || tag.type == CON || cdr.type == CON) {
	string var = global_constants.new_var_unp();
	global_constants.decls += "SEXP " + var + ";\n";
	global_constants.defs += "{\n";
	global_constants.defs += consts.decls;
	global_constants.defs += consts.defs;
	if (car.type == CON) {
	  global_constants.defs += var + " =   " + car.var + ";\n";
	  car.var = var;
	}
	if (tag.type == CON) {
	  global_constants.defs += var + " =   " + tag.var + ";\n";
	  tag.var = var;
	}
	if (cdr.type == CON) {
	  global_constants.defs += var + " =   " + cdr.var + ";\n";
	  cdr.var = var;
	}
	global_constants.defs += "}\n";
      }
      return Expression(DEP, appl3_unp("tagged_cons",
				       car.var,
				       tag.var,
				       cdr.var,
				       subexps,
				       (car.type == DEP),
				       (tag.type == DEP),
				       FALSE));
    } else {
      out_const = appl3_unp("tagged_cons",
			    car.var,
			    tag.var,
			    cdr.var,
			    consts,
			    (car.type == CON),
			    (tag.type == CON),
			    FALSE);
      return Expression(CON, out_const);
    }
  }
}

Expression op_string(SEXP s, SubexpBuffer & subexps) {
  int i, len;
  string str = "";
  len = length(s);
  for(i=0; i<len; i++) {
    str += string(CHAR(STRING_ELT(s, i)));
  }
  global_visible = 1;
  return Expression(STR, appl1("mkString", 
			       quote(escape(str)), 
			       global_constants));
}

Expression op_vector(SEXP vec, SubexpBuffer & subexps) {
  int len = length(vec);
  switch(TYPEOF(vec)) {
  case LGLSXP:
    if (len == 1) {
      global_visible = 1;
      int value = INTEGER(vec)[0];
      map<int,string>::iterator pr = sc_logical_map.find(value);
      if (pr == sc_logical_map.end()) {  // not found
	string var = appl1("ScalarLogical",
			   itos(value),
			   global_constants);
	sc_logical_map.insert(pair<int,string>(value, var));
	return Expression(STR, var);
      } else {
	return Expression(STR, pr->second);
      }
    } else {
      global_ok = 0;
      return Expression(STR, "<<unimplemented logical vector>>");
    }
    break;
  case INTSXP:
    if (len == 1) {
      int value = INTEGER(vec)[0];
      map<int,string>::iterator pr = sc_integer_map.find(value);
      if (pr == sc_integer_map.end()) {  // not found
	string var = appl1("ScalarInteger",
			   itos(value),
			   global_constants);
	sc_integer_map.insert(pair<int,string>(value, var));
	return Expression(STR, var);
      } else {
	return Expression(STR, pr->second);
      }
    } else {
      global_ok = 0;
      return Expression(STR, "<<unimplemented integer vector>>");
    }
    break;
  case REALSXP:
    if (len == 1) {
      double value = REAL(vec)[0];
      map<double,string>::iterator pr = sc_real_map.find(value);
      if (pr == sc_real_map.end()) {  // not found
	string var = appl1("ScalarReal",
			   dtos(value),
			   global_constants);
	sc_real_map.insert(pair<double,string>(value, var));
	return Expression(STR, var);
      } else {
	return Expression(STR, pr->second);
      }
    } else {
      global_ok = 0;
      return Expression(STR, "<<unimplemented real vector>>");
    }
    break;
  case CPLXSXP:
    if (len == 1) {
      Rcomplex value = COMPLEX(vec)[0];
      string var = appl1("ScalarComplex",
			 ctos(value),
			 global_constants);
      return Expression(STR, var);
    } else {
      global_ok = 0;
      return Expression(STR, "<<unimplemented complex vector>>");
    }
    break;
  default:
    err("Internal error: op_vector encountered bad vector type\n");
    return Expression(STR, "BOGUS"); // not reached
    break;
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
      string var = global_constants.new_var_unp();
      global_constants.decls += "SEXP " + var + ";\n";
      global_constants.defs += var + " = install(" + quote(name) + ");\n";
      //      string var = appl1("install", quote(name), global_constants);
      symbol_map.insert(pair<string,string>(name, var));
      return var;
    } else {
      return pr->second;
    }
  }
}

string make_fundef(string func_name, SEXP args, SEXP code) {
  int i;
  string f, header;
  SEXP temp_args = args;
  int len = length(args);
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
    header += ", SEXP arg" + itos(i);
  }
  header += ")";
  global_fundefs.decls += header + ";\n";
  f += header + " {\n";
  f += indent("SEXP newenv;\n");
  f += indent("SEXP out;\n");
  f += indent("RCNTXT context;\n");
  string formals = op_symlist(args, "env", env_subexps).var;
  string actuals = "R_NilValue";
  for (i=len-1; i>=0; i--) {
    actuals = appl2("cons",
		    "arg" + itos(i),
		    actuals,
		    env_subexps);
  }
  int env_nprot = env_subexps.get_n_prot() + 1;
  f += indent(env_subexps.decls);
  f += indent(env_subexps.defs);
  f += indent("PROTECT(newenv =\n");
  f += indent(indent("Rf_NewEnvironment(\n"
		     + indent(formals) + ",\n"
		     + indent(actuals) + ",\n"
		     + indent("env") + "));\n"));
  f += indent("if (SETJMP(context.cjmpbuf)) {\n");
  f += indent(indent("out = R_ReturnedValue;\n"));
  f += indent("} else {\n");
  f += indent(indent("begincontext(&context, CTXT_RETURN, R_NilValue, newenv, env, R_NilValue);\n"));
  string outblock = op_exp(code, "newenv", out_subexps).var;
  f += indent(indent("{\n"));
  f += indent(indent(indent(out_subexps.decls)));
  f += indent(indent(indent(out_subexps.defs)));
  f += indent(indent(indent("out = " + outblock + ";\n")));
  f += indent(indent(indent("UNPROTECT(1);\n")));
  f += indent(indent("}\n"));
  f += indent(indent("endcontext(&context);\n"));
  f += indent("}\n");
  f += indent("UNPROTECT(" + itos(env_nprot) + ");\n");
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
  f += indent("PROTECT(env = CADR(full_args));\n");
  f += indent("PROTECT(args = CDDR(full_args));\n");
  string formals = op_symlist(args, "env", env_subexps).var;
  string actuals = "args";
  int env_nprot = env_subexps.get_n_prot() + 3;
  f += indent(env_subexps.decls);
  f += indent(env_subexps.defs);
  f += indent("PROTECT(newenv =\n");
  f += indent(indent("Rf_NewEnvironment(\n"
		     + indent(formals) + ",\n"
		     + indent(actuals) + ",\n"
		     + indent("env") + "));\n"));
  f += indent("if (SETJMP(context.cjmpbuf)) {\n");
  f += indent(indent("out = R_ReturnedValue;\n"));
  f += indent("} else {\n");
  f += indent(indent("begincontext(&context, CTXT_RETURN, R_NilValue, newenv, env, R_NilValue);\n"));
  Expression outblock = op_exp(code, "newenv", out_subexps);
  f += indent(indent("{\n"));
  f += indent(indent(indent(out_subexps.decls)));
  f += indent(indent(indent(out_subexps.defs)));
  f += indent(indent(indent("out = " + outblock.var + ";\n")));
  if (outblock.type == DEP) {
    f += indent(indent(indent("UNPROTECT(1);\n")));
  }
  f += indent(indent("}\n"));
  f += indent(indent("endcontext(&context);\n"));
  f += indent("}\n");
  f += indent("UNPROTECT(" + itos(env_nprot) + ");\n");
  f += indent("return out;\n");
  f += "}\n";
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
string itos(int i) {
  if (i == (int)0x80000000) {
    return "0x80000000"; /* -Wall complains about this as a decimal constant */
  } else {
    ostringstream ss;
    ss << i;
    return ss.str();
  }
}

string dtos(double d) {
  if (d == HUGE_VAL) {
    return "HUGE_VAL";
  } else {
    ostringstream ss;
    ss << d;
    return ss.str();
  }
}

string ctos(Rcomplex c) {
  return "mk_complex(" + dtos(c.r) + "," + dtos(c.i) + ")";
}

/* Convenient macro-like things for outputting function applications */

string appl1(string func, string arg, SubexpBuffer & subexps) {
  string var = subexps.new_var();
  subexps.decls += "SEXP " + var + ";\n";
  subexps.defs += "PROTECT(" + var + " = " + func + "(" + arg + "));\n";
  return var;
}

string appl2(string func, string arg1, string arg2, 
	     SubexpBuffer & subexps,
	     bool unp_1 /* = FALSE */, bool unp_2 /* = FALSE */) {
  string var = subexps.new_var();
  subexps.decls += "SEXP " + var + ";\n";
  subexps.defs += "PROTECT(" + var + " = " 
    + func + "(" + arg1 + ", " + arg2 + "));\n";
  if (unp_1) {
    subexps.defs += "UNPROTECT_PTR(" + arg1 + ");\n";
  }
  if (unp_2) {
    subexps.defs += "UNPROTECT_PTR(" + arg2 + ");\n";
  }
  return var;
} 

string appl2_unp(string func, string arg1, string arg2, 
		 SubexpBuffer & subexps, bool unp_1, bool unp_2) {
  string var = subexps.new_var();
  subexps.decls += "SEXP " + var + ";\n";
  subexps.defs += var + " = " 
    + func + "(" + arg1 + ", " + arg2 + ");\n";
  if (unp_1) {
    subexps.defs += "UNPROTECT_PTR(" + arg1 + ");\n";
  }
  if (unp_2) {
    subexps.defs += "UNPROTECT_PTR(" + arg2 + ");\n";
  }
  return var;
}  

string appl3(string func, string arg1, string arg2, string arg3,
	     SubexpBuffer & subexps,
	     bool unp_1, bool unp_2, bool unp_3) {
  string var = subexps.new_var();
  subexps.decls += "SEXP " + var + ";\n";
  subexps.defs += "PROTECT(" + var + " = " 
    + func + "(" + arg1 + ", " + arg2 + ", " + arg3 + "));\n";
  if (unp_1) {
    subexps.defs += "UNPROTECT_PTR(" + arg1 + ");\n";
  }
  if (unp_2) {
    subexps.defs += "UNPROTECT_PTR(" + arg2 + ");\n";
  }
  if (unp_3) {
    subexps.defs += "UNPROTECT_PTR(" + arg3 + ");\n";
  }
  return var;
}

string appl3_unp(string func, string arg1, string arg2, string arg3,
		 SubexpBuffer & subexps,
		 bool unp_1, bool unp_2, bool unp_3) {
  string var = subexps.new_var();
  subexps.decls += "SEXP " + var + ";\n";
  subexps.defs += var + " = " 
    + func + "(" + arg1 + ", " + arg2 + ", " + arg3 + ");\n";
  if (unp_1) {
    subexps.defs += "UNPROTECT_PTR(" + arg1 + ");\n";
  }
  if (unp_2) {
    subexps.defs += "UNPROTECT_PTR(" + arg2 + ");\n";
  }
  if (unp_3) {
    subexps.defs += "UNPROTECT_PTR(" + arg3 + ");\n";
  }
  return var;
}


string appl4(string func, 
	     string arg1, 
	     string arg2, 
	     string arg3, 
	     string arg4,
	     SubexpBuffer & subexps) {
  string var = subexps.new_var();
  subexps.decls += "SEXP " + var + ";\n";
  subexps.defs += "PROTECT(" + var + " = " 
    + func + "(" + arg1 + ", " + arg2 + ", " + arg3 + ", " + arg4 + "));\n";
  return var;
}

string appl5(string func, 
	     string arg1, 
	     string arg2, 
	     string arg3, 
	     string arg4,
	     string arg5,
	     SubexpBuffer & subexps) {
  string var = subexps.new_var();
  subexps.decls += "SEXP " + var + ";\n";
  subexps.defs += "PROTECT(" + var + " = " 
    + func + "(" + arg1 + ", " + arg2 + ", " + arg3 + ", " + arg4 
    + ", " + arg5 + "));\n";
  return var;
}

string appl6(string func,
	     string arg1,
	     string arg2,
	     string arg3,
	     string arg4,
	     string arg5,
	     string arg6,
	     SubexpBuffer & subexps) {
  string var = subexps.new_var();
  subexps.decls += "SEXP " + var + ";\n";
  subexps.defs += "PROTECT(" + var + " = " 
    + func + "(" + arg1 + ", " + arg2 + ", " + arg3 + ", " + arg4 
    + ", " + arg5 + ", " + arg6 + "));\n";
  return var;
}

/* Escape \'s and \n's to represent a string in C code. */
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
      //e[num_exps] = exp;
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

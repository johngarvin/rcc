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

// File: op_fundef.cc
//
// Output a fundef (function definition).
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <CheckProtect.h>
#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnalysisResults.h>
#include <analysis/CEscapeInfo.h>
#include <analysis/FormalArgInfo.h>
#include <analysis/FuncInfo.h>
#include <analysis/LexicalContext.h>
#include <analysis/Settings.h>
#include <analysis/Utils.h>
#include <analysis/VarBinding.h>

#include <support/StringUtils.h>
#include <support/RccError.h>

#include <CodeGenUtils.h>
#include <Metrics.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;
using namespace RAnnot;

string make_fundef(SubexpBuffer * this_buf, std::string func_name, 
			SEXP fndef);
string make_fundef_c(SubexpBuffer * this_buf, std::string func_name, 
			  SEXP fndef);
string output_strictness(SEXP args);

/// Output a function definition.
Expression SubexpBuffer::op_fundef(SEXP fndef, string rho,
				   Protection resultProtection)
{
  Metrics::get_instance()->inc_procedures();

  FuncInfo *fi = getProperty(FuncInfo, fndef);
  lexicalContext.Push(fi);
  SEXP e = CDR(fndef); // skip over "function" symbol
  string c_name = fi->get_c_name();

  // closure version
  ParseInfo::global_fundefs->append_defs(make_fundef(this, c_name, fndef));
  if (rho == "R_GlobalEnv") {
    Expression r_args = ParseInfo::global_constants->op_list(CAR(e),
							     rho, true, Protected);
    Expression r_code = ParseInfo::global_constants->op_literal(CADR(e), rho);
    string closure = fi->get_closure();
    ParseInfo::global_constants->appl(closure,
				      resultProtection,
				      "mkRCC_CLOSXP",
				      to_string(e),
				      4,
				      &r_args.var,
				      &c_name,
				      &r_code.var,
				      &rho);
    lexicalContext.Pop();
    return Expression(closure, CONST, INVISIBLE, "");
  } else {   // not the global environment
    Expression r_args = op_list(CAR(e), rho, true, Protected);
    Expression r_code = op_literal(CADR(e), rho);
    string closure = fi->get_closure();
    appl(closure, resultProtection, "mkRCC_CLOSXP", to_string(e), 4, &r_args.var, &c_name, &r_code.var, &rho);
    del(r_args);
    del(r_code);
    lexicalContext.Pop();
    string cleanup = (resultProtection == Protected ? unp(closure) : "");
    return Expression(closure, DEPENDENT, CHECK_VISIBLE, cleanup);
  }
}

///  Given an R function, emit a C function with two arguments. The
///  first is a list containing all the R arguments. (This is to work
///  easily with "...", default arguments, etc.) The second is the
///  environment in which the function is to be executed.
string make_fundef(SubexpBuffer * this_buf, string func_name, SEXP fndef) {
  SEXP args = fundef_args(fndef);

  string f, header;
  SubexpBuffer out_subexps;
  SubexpBuffer env_subexps;

  string strictness = comment("strictness: " + output_strictness(args));  // string of S and N: whether each formal is strict

  // whether to use escape analysis to stack allocate objects
  bool stack_alloc_obj = Settings::get_instance()->get_stack_alloc_obj();

  header = "SEXP " + func_name + "(";
  header += "SEXP args, SEXP newenv)";
  ParseInfo::global_fundefs->decls += header + ";\n";
  f += header + " { " + strictness + " \n";
#ifdef CHECK_PROTECT
  f += indent("int topval = R_PPStackTop;\n");
#endif
  f += indent("SEXP out;\n");

  FuncInfo * fi = lexicalContext.Top();
  
  if (fi->requires_context()) {
    f += indent("RCNTXT context;\n");
  }
  if (stack_alloc_obj) {
    f += indent("SEXP stack;\n");
  }

  string actuals = "args";
  env_subexps.output_ip();
  env_subexps.finalize();
  f += indent(env_subexps.output_decls());
  f += indent(env_subexps.output_defs());

  if (fi->requires_context()) {
    f += indent("if (SETJMP(context.cjmpbuf)) {\n");
    f += indent(indent("PROTECT(out = R_ReturnedValue);\n"));
    f += indent("} else {\n");
    f += indent(indent("begincontext(&context, CTXT_RETURN, R_NilValue, newenv, newenv, R_NilValue, R_NilValue);\n"));
    // TODO: 5th arg of begincontext should be old environment. Emit
    // another parameter to the current function to pass it in if we
    // require context.
  }

  // add code to get the location for each argument
  string arg_location_decls = "SEXP arg_loc_p;\n";
  string arg_location_defs = "arg_loc_p = args;\n";
  VarBinding * binding;
  for(SEXP p = args; p != R_NilValue; p = CDR(p)) {
    binding = getProperty(VarBinding, p);
    string location = binding->get_location(TAG(p), this_buf);
    arg_location_decls += "R_varloc_t " + location + ";\n";
    arg_location_defs += emit_assign(location, emit_call1("get_R_location", "arg_loc_p"));
    arg_location_defs += emit_assign("arg_loc_p", emit_call1("CDR", "arg_loc_p"));
  }

  // emit stack allocation
  //  string alloc_function = Settings::get_instance()->get_stack_debug() ? "malloc" : "alloca";
  string size = "global_alloc_stack_space_size";
  if (stack_alloc_obj) {
    //    f += indent(emit_assign("stack", "(SEXP)" + emit_call1(alloc_function, size)));
    f += indent(emit_call2("pushAllocStack", "&allocVectorStack", "&allocNodeStack") + ";\n");
  }

  // emit the function body
  Expression outblock = out_subexps.op_exp(fundef_body_c(fndef),
					   "newenv", Unprotected, true, 
					   ResultNeeded);
  f += indent(indent("{\n"));
  f += indent(indent(indent(arg_location_decls)));
  f += indent(indent(indent(arg_location_defs)));
  f += indent(indent(indent(out_subexps.output() +
			    Visibility::emit_set(outblock.visibility))));
  f += indent(indent(indent("out = " + outblock.var + ";\n")));
  f += indent(indent("}\n"));

  if (stack_alloc_obj) {
    f += indent(emit_call0("popAllocStack") + ";\n");
  }

  if (fi->requires_context()) {
    f += indent("}\n");
    f += indent("endcontext(&context);\n");
  }

#ifdef CHECK_PROTECT
  f += indent("assert(topval == R_PPStackTop);\n");
#endif
  f += indent("return out;\n");
  f += "}\n";
  return f;
}

/// Like make_fundef but for directly-called functions.
string make_fundef_c(SubexpBuffer * this_buf, string func_name, SEXP fndef) 
{
  SEXP args = fundef_args(fndef);

  string f, header;
  SubexpBuffer out_subexps;
  SubexpBuffer env_subexps;
  header = "SEXP " + func_name + "(";
  header += "SEXP args)";
  ParseInfo::global_fundefs->decls += header + ";\n";
  f += header + " {\n";
#ifdef CHECK_PROTECT
  f += indent("int topval = R_PPStackTop;\n");
#endif
  f += indent("SEXP newenv;\n");
  f += indent("SEXP out;\n");
  Expression formals = env_subexps.op_list(args, "R_GlobalEnv", true, Protected);
  f += env_subexps.output();
  f += indent("PROTECT(newenv =\n");
  f += indent(indent("Rf_NewEnvironment(\n"
		     + indent(formals.var) + ",\n"
		     + indent("args") + ",\n"
		     + indent("R_GlobalEnv") + "));\n"));
  Expression outblock = out_subexps.op_exp(fundef_body_c(fndef),
					   "newenv", Unprotected);
  f += indent("{\n");
  f += indent(indent(out_subexps.output()));
  f += Visibility::emit_set(outblock.visibility);
  f += indent(indent("out = " + outblock.var + ";\n"));
  f += indent("}\n");
  f += indent(formals.del_text);
  f += indent("UNPROTECT(1);\n");
  f += indent("return out;\n");
  f += "}\n";
  return f;
}

#if 0

// Old version of make_fundef: maps each R argument to a C argument
// instead of putting them in a list. Not used because it doesn't
// handle "...", named arguments, default arguments, etc. We could use
// this idea once we figure out compile-time argument matching.
string make_fundef(string func_name, SEXP fndef) {
  SEXP args = CAR(fundef_args_c(fndef));
  SEXP code = CAR(fundef_body_c(fndef));

  FuncInfo *fi = lexicalContext->Top();

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
    global_formals[i] = string(var_name(TAG(temp_args)));
    temp_args = CDR(temp_args);
  }
  header = "SEXP " + func_name + "(SEXP env";
  for (i=0; i<len; i++) {
    header += ", SEXP arg" + i_to_s(i);
  }
  header += ")";
  ParseInfo::global_fundefs->decls += header + ";\n";
  f += header + " {\n";
  f += indent("SEXP newenv;\n");
  f += indent("SEXP out;\n");

  if (fi->requires_context()) {
    f += indent("RCNTXT context;\n");
  }

  // <FIX ME: Doesn't this need an UNPROTECT  -- johnmc >
  string formals = env_subexps.op_list(args, "env", true, Protected).var;
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

  if (fi->requires_context()) {
    f += indent("if (SETJMP(context.cjmpbuf)) {\n");
    f += indent(indent("out = R_ReturnedValue;\n"));
    f += indent("} else {\n");
    f += indent(indent("begincontext(&context, CTXT_RETURN, R_NilValue, newenv, env, R_NilValue, R_NilValue);\n"));
  }

  Expression outblock = out_subexps.op_exp(code, "newenv");
  f += indent(indent("{\n"));
  f += indent(indent(indent(out_subexps.output())));
  f += indent(indent(indent("out = " + outblock.var + ";\n")));
  f += indent(indent("}\n"));

  if (fi->requires_context()) {
    f += indent("}\n");
    f += indent("endcontext(&context);\n");
  }

  f += indent("UNPROTECT(" + i_to_s(env_nprot) + ");\n");
  f += indent("return out;\n");
  f += "}\n";
  delete [] global_formals;
  global_formals = old_formals;
  global_formals_len = old_formals_len;
  return f;
}
#endif

string output_strictness(SEXP args) {
  string str;
  while(args != R_NilValue) {
    if (getProperty(FormalArgInfo, args)->is_strict()) {
      str += "S";
    } else {
      str += "N";
    }
    args = CDR(args);
  }
  return str;
}

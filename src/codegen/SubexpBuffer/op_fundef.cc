#include <string>

#include <CheckProtect.h>
#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/Annotation.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>
#include <support/StringUtils.h>
#include <Visibility.h>
#include <ParseInfo.h>

using namespace std;
using namespace RAnnot;

std::string make_fundef(SubexpBuffer * this_buf, std::string func_name, 
			SEXP fndef);
std::string make_fundef_c(SubexpBuffer * this_buf, std::string func_name, 
			  SEXP fndef);

//! Output a function definition.
Expression SubexpBuffer::op_fundef(SEXP fndef, string rho,
				   Protection resultProtection,
				   string opt_R_name /* = "" */)
{
  FuncInfo *fi = getProperty(FuncInfo, fndef);

  lexicalContext.Push(fi);

  SEXP e = CDR(fndef); // skip over "function" symbol

  bool direct = FALSE;
  if (!opt_R_name.empty() && ParseInfo::is_direct(opt_R_name)) {
    direct = TRUE;
    // make function to be called directly
    if (ParseInfo::func_map.find(opt_R_name) != ParseInfo::func_map.end()) { // defined already
      string closure_name = ParseInfo::func_map.find(opt_R_name)->second;
      lexicalContext.Pop();
      return Expression(closure_name, FALSE, INVISIBLE, "");
    } else { // not yet defined
      // direct version
      if (rho != "R_GlobalEnv") {
	cerr << "Warning: function " << opt_R_name.c_str() << " is not in global scope; unable to make direct function call\n";
      } else {
	ParseInfo::global_fundefs->append_defs( 
	  make_fundef_c(this,
			make_c_id(opt_R_name) + "_direct",
			fndef));
      }
      // in any case, continue to closure version
    }
  }
  // closure version
  string func_name = ParseInfo::global_fundefs->new_var();
  ParseInfo::global_fundefs->append_defs(make_fundef(this,
						     func_name,
						     fndef));
  Expression formals = op_literal(CAR(e), rho);
  if (rho == "R_GlobalEnv") {
    Expression r_args = ParseInfo::global_constants->op_list(CAR(e),
							     rho, true, Protected);
    Expression r_code = ParseInfo::global_constants->op_literal(CADR(e), rho);
#if 0
    string r_form = ParseInfo::global_constants->appl3("mkCLOSXP", r_args.var, r_code.var, rho);
    Expression c_f_args = ParseInfo::global_constants->op_arglist(CAR(e), rho);
    string c_args = ParseInfo::global_constants->appl2("cons", rho, c_f_args.var);
    string c_call = ParseInfo::global_constants->appl2("cons", func_sym, c_args);
    string r_call = ParseInfo::global_constants->appl2("lcons",
			  make_symbol(Rf_install(".External")),
			  c_call);
    string c_clos = ParseInfo::global_constants->appl3("mkCLOSXP ", formals.var,
				   func_name, rho);
#endif
    string r_form = 
      ParseInfo::global_constants->appl4("mkRCC_CLOSXP", r_args.var, func_name, r_code.var, rho);
#if 0
    ParseInfo::global_constants->append_defs("setAttrib(" + r_form
          + ", Rf_install(\"RCC_CompiledSymbol\"), " + c_clos + ");\n");
#endif
    ParseInfo::global_constants->del(formals);
    if (direct) ParseInfo::func_map.insert(pair<string,string>(opt_R_name, r_form));
    lexicalContext.Pop();
    return Expression(r_form, FALSE, INVISIBLE, "");
  } else {   // not the global environment
    Expression r_args = op_literal(CAR(e), rho);
    Expression r_code = op_literal(CADR(e), rho);
    string r_form = appl4("mkRCC_CLOSXP", r_args.var, func_name, r_code.var, rho, resultProtection);
    del(r_args);
    del(r_code);
#if 0
    Expression c_f_args = op_arglist(CAR(e), rho);
    string c_args = appl2("cons", rho, c_f_args.var);
    del(c_f_args);
    string c_call = appl2("cons", func_sym, c_args);
    string defs = unp(c_args);
    string r_call = appl2("lcons",
			  make_symbol(Rf_install(".External")),
			  c_call);
    defs += unp(c_call);
    string out = appl3("mkCLOSXP ", formals.var, r_call, rho);
    del(formals);
    defs += unp(r_call);
    defs += "setAttrib(" + r_form
          + ", Rf_install(\"RCC_CompiledSymbol\"), " + out + ");\n";
    defs += unp(out);
    append_defs(defs);
#endif
    lexicalContext.Pop();
    string cleanup = (resultProtection == Protected ? unp(r_form) : "");
    return Expression(r_form, true, CHECK_VISIBLE, cleanup);
  }
}

//!  Given an R function, emit a C function with two arguments. The
//!  first is a list containing all the R arguments. (This is to work
//!  easily with "...", default arguments, etc.) The second is the
//!  environment in which the function is to be executed.
string make_fundef(SubexpBuffer * this_buf, string func_name, SEXP fndef) {
  SEXP args = CAR(fundef_args_c(fndef));

  string f, header;
  SubexpBuffer out_subexps;
  SubexpBuffer env_subexps;
  header = "SEXP " + func_name + "(";
  header += "SEXP args, SEXP env)";
  ParseInfo::global_fundefs->decls += header + ";\n";
  f += header + " {\n";
#ifdef CHECK_PROTECT
  f += indent("int topval = R_PPStackTop;\n");
#endif
  f += indent("SEXP newenv;\n");
  f += indent("SEXP out;\n");

  FuncInfo *fi = lexicalContext.Top();

  if (fi->getRequiresContext()) {
    f += indent("RCNTXT context;\n");
  }

  Expression formals = env_subexps.op_list(args, "env", true, Protected);
  string actuals = "args";
  env_subexps.output_ip();
  env_subexps.finalize();
  f += indent(env_subexps.output_decls());
  f += indent(env_subexps.output_defs());
  f += indent("PROTECT(newenv =\n");
  f += indent(indent("Rf_NewEnvironment(\n"
		     + indent(formals.var) + ",\n"
		     + indent(actuals) + ",\n"
		     + indent("env") + "));\n"));

  if (fi->getRequiresContext()) {
    f += indent("if (SETJMP(context.cjmpbuf)) {\n");
    f += indent(indent("PROTECT(out = R_ReturnedValue);\n"));
    f += indent("} else {\n");
    f += indent(indent("begincontext(&context, CTXT_RETURN, R_NilValue, newenv, env, R_NilValue, R_NilValue);\n"));
  }

  Expression outblock = out_subexps.op_exp(fundef_body_c(fndef),
					   "newenv", Unprotected, true, 
					   ResultNeeded);
  f += indent(indent("{\n"));
  f += indent(indent(indent(out_subexps.output() +
			    Visibility::emit_set(outblock.is_visible))));
  f += indent(indent(indent("out = " + outblock.var + ";\n")));
  f += indent(indent("}\n"));

  if (fi->getRequiresContext()) {
    f += indent("}\n");
    f += indent("endcontext(&context);\n");
  }

  f += indent(formals.del_text);
  f += indent("UNPROTECT(1); /* newenv */\n");
#ifdef CHECK_PROTECT
  f += indent("assert(topval == R_PPStackTop);\n");
#endif
  f += indent("return out;\n");
  f += "}\n";
  return f;
}

// Like make_fundef but for directly-called functions.
string make_fundef_c(SubexpBuffer * this_buf, string func_name, SEXP fndef) 
{
  SEXP args = CAR(fundef_args_c(fndef));

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
  f += Visibility::emit_set(outblock.is_visible);
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
    global_formals[i] = string(CHAR(PRINTNAME(TAG(temp_args))));
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

  if (fi->getRequiresContext()) {
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

  if (fi->getRequiresContext()) {
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

  if (fi->getRequiresContext()) {
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

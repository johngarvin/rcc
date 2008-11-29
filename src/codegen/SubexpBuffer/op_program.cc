// -*- Mode: C++ -*-
//
// Copyright (c) 2008 Rice University
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

// File: op_program.cc
//
// Output an RCC-generated program.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <include/R/R_RInternals.h>

#include <support/RccError.h>
#include <support/StringUtils.h>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>

#include <CheckProtect.h>
#include <CodeGenUtils.h>
#include <ParseInfo.h>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

using namespace std;

string SubexpBuffer::op_program(SEXP e, string rho, string func_name,
				bool output_main_program, bool output_default_args)
{
  int i;
  string exec_decls, exec_defs;

  // count expressions so we can number them
  SEXP tmp_e = e;
  int n_exprs = 0;
  while (tmp_e != R_NilValue) {
    n_exprs++;
    tmp_e = CDR(tmp_e);
  }
  
  // output top-level expressions (Expression version)
  for(i=0; i<n_exprs; i++, e = CDR(e)) {
    SubexpBuffer subexps;
    Expression exp;
    if (is_rcc_assert_def(CAR(e)) || is_rcc_assertion(CAR(e))) {
      // assertions are for the analysis phase;
      // don't add them to generated code
      continue;
    }
    
    if (ParseInfo::analysis_ok()) {
      try {
	exp = subexps.op_exp(e, "R_GlobalEnv");  // op_exp takes a cell
      } catch (AnalysisException ae) {
	rcc_warn(ae.what());
	rcc_warn("analysis encountered difficulties; compiling trivially");
	clearProperties();
	ParseInfo::set_analysis_ok(false);
      }
    }
    
    if (!ParseInfo::analysis_ok()) {
      // compile trivially
      SubexpBuffer sb1;
      Expression exp1 = sb1.op_literal(CAR(e), "R_GlobalEnv");
      exp = Expression(emit_call2("Rf_eval", exp1.var, "R_GlobalEnv"),
		       DEPENDENT, CHECK_VISIBLE, exp1.del_text);
      subexps = sb1;
    }
    
    string this_exp;
    this_exp += subexps.output_decls();
    this_exp += Visibility::emit_set_if_visible(exp.visibility);
    this_exp += subexps.output_defs();
    if (exp.visibility != INVISIBLE) {
      string evar = "e" + i_to_s(i);
      exec_decls += "SEXP " + evar + ";\n";
      string pval = emit_call2("PrintValueEnv", evar, "R_GlobalEnv") + ";\n";
      string printexpn = emit_assign(evar, exp.var);
      if (exp.visibility == VISIBLE) {
	printexpn += pval;
      } else if (exp.visibility == CHECK_VISIBLE) {
	string check = Visibility::emit_check_expn();
	printexpn += emit_logical_if_stmt(check, pval);
      }
      this_exp += printexpn;
    }
    if (!exp.del_text.empty())
      this_exp += "UNPROTECT(1);\n";
    exec_defs += indent(emit_in_braces(this_exp));
  }

  // Expression code generated. Now the rest

  string finish_code;
  finish_code += "UNPROTECT(" + i_to_s(ParseInfo::global_constants->get_n_prot()) + "); /* c_ */\n";
  finish_code += "Rprintf(\"\\n\");\n";
  
  string program;

  string rcc_path_prefix = string("#include \"") + RCC_INCLUDE_PATH + "/"; 

  // output
  program += rcc_path_prefix + "rcc_generated_header.h\"\n";
  program += "\n";
#ifdef CHECK_PROTECT
  program += "#include <assert.h>\n";
#endif

  ParseInfo::global_fundefs->output_ip();
  ParseInfo::global_fundefs->finalize();

  ParseInfo::global_constants->output_ip();
  ParseInfo::global_constants->finalize();

  program += ParseInfo::global_fundefs->output_decls();
  program += ParseInfo::global_constants->output_decls();

  string header;
  header += "\nvoid " + func_name + "() {\n";
  for(i=0; i<ParseInfo::global_constants->get_n_inits(); i++) {
    header += indent(ParseInfo::global_constants->get_init_str() + i_to_s(i) + "();\n");
  }
  header += indent("exec();\n");
  header += indent("finish();\n");
  header += "}\n";
  program += header;

  program += ParseInfo::global_constants->output_defs();
  program += "static void exec() " + emit_in_braces(exec_decls + exec_defs) + "\n\n";
  program += "static void finish() " + emit_in_braces(finish_code, false) + "\n\n";
  program += ParseInfo::global_fundefs->output_defs();
  if (output_main_program) {
     string arginit = "int myargc;\n";
     string mainargs = "int argc, char **argv";
     if (output_default_args) {
       mainargs = "int argc, char **argv";
       arginit += string("char *myargv[5];\n") +
	 "myargv[0] = argv[0];\n" +
	 "myargv[1] = \"--gui=none\";\n" + 
	 "myargv[2] = \"--slave\";\n" +
	 "myargv[3] = \"--vanilla\";\n" +
	 "myargv[4] = NULL;\n" + "myargc = 4;\n";
     } else {
       mainargs = "int argc, char **argv";
       arginit += string("myargc = argc\n;") + "myargv = argv\n;";
     }
     string body = arginit + "Rf_initialize_R(myargc, myargv);\n" + "setup_Rmainloop();\n" +
       func_name + "();\nreturn 0;\n";
     program += "\nint main(" + mainargs + ") \n{\n" + indent(body) + "}\n"; 
  }
  return program;
}

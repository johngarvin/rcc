// -*- Mode: C++ -*-
//
// Copyright (c) 2003-2006 Rice University
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

// File: Main.cc
//
// Main RCC program. Processes command line arguments, initializes,
// calls the R parser, calls analysis routines, and outputs the result.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <iostream>

#include <OpenAnalysis/CallGraph/ManagerCallGraph.hpp>  // TODO: remove after testing
#include <OpenAnalysis/Alias/InterAliasMap.hpp>  // TODO: remove after testing
#include <OpenAnalysis/Alias/ManagerInterAliasMapBasic.hpp> // TODO: remove after testing
#include <OpenAnalysis/DataFlow/CallGraphDFSolver.hpp> // TODO: remove after testing
#include <OpenAnalysis/SideEffect/ManagerInterSideEffectStandard.hpp> // TODO: remove after testing

#include <CheckProtect.h>
#include <include/R/R_Internal.h>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/HandleInterface.h>
#include <analysis/SymbolTable.h>
#include <support/RccError.h>

#include <analysis/call-graph/CallGraphAnnotationMap.h>
#include <analysis/call-graph/CallGraphAnnotation.h>

#include <CodeGenUtils.h>
#include <CodeGen.h>
#include <Output.h>
#include <ParseInfo.h>
#include <LoopContext.h>
#include <CScope.h>
#include <Main.h>

using namespace std;
using namespace RAnnot;

// Settings to change how rcc works
static bool global_self_allocate = false;
static bool output_main_program = true;
static bool output_default_args = true;
static bool analysis_debug = false;
static bool cfg_dot_dump = false;

int main(int argc, char *argv[]) {
  int i;
  char *fullname_c;
  string fullname, libname, out_filename, path, exec_decls, exec_code;
  bool in_file_exists, out_file_exists = FALSE;
  FILE *in_file;
  int n_exprs;

  // for getopt
  int c;
  extern char *optarg;
  extern int optind, opterr, optopt;

  // initialize ParseInfo buffers except global_fundefs.
  // Function definitions initialized after we have the library name.
  ParseInfo::global_constants = new SplitSubexpBuffer("c", true);
  ParseInfo::global_labels = new SubexpBuffer("l");

  // get options
  while(1) {
    c = getopt(argc, argv, "acdf:lmo:");
    if (c == -1) {
      break;
    }
    switch(c) {
    case 'a':
      output_default_args = false;
      break;
    case 'c':
      // dump CFG in dot form
      cfg_dot_dump = true;
      break;
    case 'd':
      // print debugging information
      analysis_debug = true;
      break;
    case 'f':
      // argument is the name of a function that can be called directly
      ParseInfo::direct_funcs.insert(string(optarg));
      break;
    case 'l':
      // use local allocation
      global_self_allocate = true;
      break;
    case 'm':
      // don't output a main program
      output_main_program = false;
      break;
    case 'o':
      // specify output file
      out_file_exists = true;
      out_filename = string(optarg);
      break;
    case '?':
      arg_err();
      break;
    case ':':
      arg_err();
      break;
    default:
      rcc_error("Unknown error: getopt() returned " + i_to_s(c));
      break;
    }
  }

  // get filename, if it exists
  if (optind < argc) {
    in_file_exists = true;
    fullname_c = argv[optind++];
    if (optind < argc) {
      cerr << "Warning: ignoring extra arguments: ";
      while (optind < argc) {
	cerr << argv[optind++] << " ";
      }
      cerr << endl;
    }
  } else {  // no filename specified
    in_file_exists = FALSE;
  }

  // Initialize R interface
  init_R();

  // set in_file, libname, and path depending on what we're given
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
    // function
  } else {
    in_file = stdin;
    libname = "R_output";
    path = "";
  }

  // set output filename if no "-o filename" option was found
  if (!out_file_exists) {
    out_filename = path + libname + ".c";
  }

  // initialize ParseInfo::global_fundefs
  ParseInfo::global_fundefs = new SubexpBuffer(libname + "_f", TRUE);

  ParseInfo::global_constants->decls += "static void exec();\n";
  ParseInfo::global_constants->decls += "static void finish();\n";

  // FIXME: load standard assertions here (before parsing)
  // I don't think we have any except those in the code...

  // parse
  SEXP program = parse_R_as_function(in_file);

  // perform analysis
  R_Analyst * an = R_Analyst::get_instance(program);
  bool analysis_ok = an->perform_analysis();

  try {
    if (analysis_debug) {
      FuncInfo * scope_tree = an->get_scope_tree_root();
      FuncInfoIterator fii(scope_tree);
      for(FuncInfo *fi; fi = fii.Current(); ++fii) {
	cout << "New procedure:" << endl;
	fi->dump(cout);
	cout << "Dumping CFG via Analyst:" << endl;
	an->dump_cfg(cout, fi->get_defn());
	cout << "Dumping symbol table:" << endl;
	SymbolTable * st = getProperty(SymbolTable, fi->get_defn());
	st->dump(cout);
      }
      cout << "Dumping call graph:" << endl;
      CallGraphAnnotationMap::get_instance()->dump(cout);
    }
    if (cfg_dot_dump) {
      CallGraphAnnotationMap::get_instance()->dumpdot(cout);
    }
  }
  catch (AnalysisException ae) {
    // One phase of analysis rejected a program. Get rid of the
    // information in preparation to compile trivially.
    rcc_warn("analysis encountered problems; compiling trivially");
    clearProperties();
    analysis_ok = false;
  }

  // temporary: test our IRInterface by building OA call graph
  // TODO: remove
  // first build call graph
  OA::CallGraph::ManagerCallGraphStandard man(an->get_interface());
  OA::OA_ptr<OA::ProcHandleIterator> proc_iter; proc_iter = new R_ProcHandleIterator(an->get_scope_tree_root());
  OA::OA_ptr<OA::Alias::ManagerInterAliasMapBasic> alias_man; alias_man = new OA::Alias::ManagerInterAliasMapBasic(an->get_interface());
  OA::OA_ptr<OA::Alias::InterAliasInterface> alias; alias = alias_man->performAnalysis(proc_iter);
  OA::OA_ptr<OA::CallGraph::CallGraph> call_graph = man.performAnalysis(proc_iter, alias);
  // now perform call graph data flow analysis
  //  OA::SideEffect::ManagerInterSideEffectStandard solver(an->get_interface());
//   solver.performAnalysis(call_graph,
// 			 param_bindings,
// 			 alias,
// 			 intra_man);

  // We had to make our program one big function to use
  // OpenAnalysis. Now forget the function definition and assignment
  // and just look at the list of expressions.
  SEXP r_expressions = curly_body(CAR(fundef_body_c(CAR(assign_rhs_c(program)))));

  // count expressions so we can number them
  SEXP e = r_expressions;
  n_exprs = 0;
  while (e != R_NilValue) {
    n_exprs++;
    e = CDR(e);
  }

#ifdef USE_OUTPUT_CODEGEN
  // output top-level expressions (Output version)
  string g_decls, g_code, code;
  for(i=0; i<n_exprs; i++) {
    string evar = "e" + i_to_s(i);
    decls += "SEXP " + evar + ";\n";
    Output exp;
    if (analysis_ok) {
      exp = CodeGen::op_exp(r_expressions, "R_GlobalEnv");  // op_exp takes a cell
    } else {
      // compile trivially
      exp = CodeGen::op_literal(CAR(r_expressions), "R_GlobalEnv");
    }      
    string this_exp = exp.decls()
      + Visibility::emit_set_if_visible(exp.visibility())
      + exp.code();
    if (exp.visibility() != INVISIBLE) {
      string printexpn = emit_assign(evar, exp.handle());
      string check;
      if (exp.visibility() == CHECK_VISIBLE) {
	check = Visibility::emit_check_expn();
      }
      printexpn +=
	emit_logical_if_stmt(check,
			     emit_call2("PrintValueEnv",
					evar,
					"R_GlobalEnv"));
      this_exp += printexpn;
    }
    this_exp += exp.del_text();
    code += emit_in_braces(this_exp);
    r_expressions = CDR(r_expressions);
  }
  exprs = g_decls + g_code + code;
  exprs += emit_call1("UNPROTECT", "1") + "/* FIXME */";
#else  // we're using SubexpBuffer codegen
  // output top-level expressions (Expression version)
  for(i=0; i<n_exprs; i++, r_expressions = CDR(r_expressions)) {
    SubexpBuffer subexps;
    Expression exp;
    if (is_rcc_assert_def(CAR(r_expressions)) || is_rcc_assertion(CAR(r_expressions))) {
      // assertions are for the analysis phase;
      // don't add them to generated code
      continue;
    }
    if (analysis_ok) {
      exp = subexps.op_exp(r_expressions, "R_GlobalEnv");  // op_exp takes a cell
    } else {
      // compile trivially
      Expression exp1 = subexps.op_literal(CAR(r_expressions), "R_GlobalEnv");      
      exp = Expression(emit_call2("Rf_eval", exp1.var, "R_GlobalEnv"),
		       DEPENDENT, CHECK_VISIBLE, exp1.del_text);
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
    exec_code += indent(emit_in_braces(this_exp));
  }
#endif

  string finish_code = "UNPROTECT(" + i_to_s(ParseInfo::global_constants->get_n_prot()) +
    "); /* c_ */\n";
  
  if (ParseInfo::get_problem_flag()) {
    out_filename += ".bad";
    cerr << "Error: one or more problems compiling R code.\n"
	 << "Outputting best attempt to " 
	 << "\'" << out_filename << "\'.\n";
  }
  ofstream out_file(out_filename.c_str());
  if (!out_file) {
    rcc_error("Couldn't open file " + out_filename + " for output");
  }
  
  string rcc_path_prefix = string("#include \"") + RCC_INCLUDE_PATH + "/"; 

  // output to file
  out_file << rcc_path_prefix << "rcc_generated_header.h\"\n";
  out_file << "\n";
#ifdef CHECK_PROTECT
  out_file << "#include <assert.h>\n";
#endif

  ParseInfo::global_fundefs->output_ip();
  ParseInfo::global_fundefs->finalize();

  ParseInfo::global_constants->output_ip();
  ParseInfo::global_constants->finalize();

  out_file << ParseInfo::global_fundefs->output_decls();
  out_file << ParseInfo::global_constants->output_decls();

  // The name R_init_<libname> is a signal to the R dynamic loader
  // telling it to execute the function immediately upon loading.
  string file_initializer_name = string("R_init_") + libname;

  string header;
  header += "\nvoid " + file_initializer_name + "() {\n";
  for(i=0; i<ParseInfo::global_constants->get_n_inits(); i++) {
    header += indent(ParseInfo::global_constants->get_init_str() + i_to_s(i) + "();\n");
  }
  header += indent("exec();\n");
  header += indent("finish();\n");
  header += "}\n";
  out_file << header;

  out_file << ParseInfo::global_constants->output_defs();
  out_file << "static void exec() " + emit_in_braces(exec_decls + exec_code) + "\n\n";
  out_file << "static void finish() " + emit_in_braces(finish_code, false) + "\n\n";
  out_file << ParseInfo::global_fundefs->output_defs();
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
    rcc_error("Couldn't write to file " + out_filename);
  }
  if (ParseInfo::get_problem_flag()) {
    return 1;
  } else {
    return 0;
  }
}

static void arg_err() {
  cerr << "Usage: rcc [input-file] [-a] [-c] [-d] [-l] [-m] [-o output-file] [-f function-name]*\n";
  exit(1);
}

// initialize statics in SubexpBuffer
unsigned int SubexpBuffer::n = 0;
unsigned int SubexpBuffer::global_temps = 0;

/// Convert an Output into an Expression. Will go away as soon as
/// everything uses Output instead of Expression.
const Expression SubexpBuffer::output_to_expression(const Output op) {
  append_decls(op.decls());
  append_defs(op.code());
  ParseInfo::global_constants->append_decls(op.g_decls());
  ParseInfo::global_constants->append_defs(op.g_code());
  return Expression(op.handle(),
		    op.dependence(),
		    op.visibility(),
		    op.del_text());
}

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

#include <OpenAnalysis/CallGraph/ManagerCallGraph.hpp>
#include <OpenAnalysis/DataFlow/CallGraphDFSolver.hpp>
#include <OpenAnalysis/DataFlow/DGraphSolverDFP.hpp>
#include <OpenAnalysis/DataFlow/ManagerParamBindings.hpp>
#include <OpenAnalysis/SideEffect/ManagerInterSideEffectStandard.hpp>
#include <OpenAnalysis/Utils/OutputBuilderDOT.hpp>

#include <CheckProtect.h>
#include <include/R/R_Internal.h>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/HandleInterface.h>
#include <analysis/OACallGraphAnnotationMap.h>
#include <analysis/SymbolTable.h>

#include <support/Debug.h>
#include <support/RccError.h>
#include <support/StringUtils.h>

// we are using OpenAnalysis call graphs instead
// #include <analysis/call-graph/RccCallGraphAnnotationMap.h>
// #include <analysis/call-graph/RccCallGraphAnnotation.h>

#include <CodeGenUtils.h>
#include <CodeGen.h>
#include <CommandLineArgs.h>
#include <CScope.h>
#include <LoopContext.h>
#include <Main.h>
#include <Output.h>
#include <ParseInfo.h>

using namespace std;
using namespace RAnnot;

// Settings to change how rcc works
static bool output_main_program = true;
static bool output_default_args = true;
static bool analysis_debug;

int main(int argc, char * argv[]) {
  RCC_DEBUG("RCC_Main", analysis_debug);

  int i;
  string libname, path, exec_decls, exec_code, fullname, out_filename;
  FILE *in_file;
  int n_exprs;

  CommandLineArgs * args = new CommandLineArgs(argc, argv);
  ParseInfo::set_command_line_args(args);

  // initialize ParseInfo buffers except global_fundefs.
  // Function definitions initialized after we have the library name.
  ParseInfo::global_constants = new SplitSubexpBuffer("c", true);
  ParseInfo::global_labels = new SubexpBuffer("l");

  // Initialize R interface
  init_R();

  fullname = args->get_fullname();
  // set in_file, libname, and path depending on what we're given
  if (args->get_in_file_exists()) {
    in_file = fopen(fullname.c_str(), "r");
    if (in_file == NULL) {
      string program = argv[0];

      string::size_type dot = program.rfind(".", program.length());
      if (dot != 0) program = program.substr(0, dot);

      string::size_type slash = program.rfind("/", program.length());
      if (slash != 0) program = program.substr(slash+1, program.length());

      // TODO: use rcc_error instead
      cerr << program << ": unable to open input file \"" << fullname << "\"" << endl;
      exit(-1);
    }
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
  if (!args->get_out_file_exists()) {
    out_filename = path + libname + ".c";
  } else {
    out_filename = args->get_out_filename();
  }

  // The name R_init_<libname> is a signal to the R dynamic loader
  // telling it to execute the function immediately upon loading.
  string file_initializer_name = "R_init_" + libname;

  // initialize ParseInfo::global_fundefs
  ParseInfo::global_fundefs = new SubexpBuffer(libname + "_f", TRUE);

  ParseInfo::global_constants->decls += "static void exec();\n";
  ParseInfo::global_constants->decls += "static void finish();\n";

  // parse
  SEXP program = parse_R_as_function(in_file);

  // perform analysis
  try {
    R_Analyst * an = R_Analyst::get_instance(program);
    an->perform_analysis();

    if (analysis_debug) {
      FuncInfo * fi;
      FOR_EACH_PROC(fi) {
	cout << "New procedure:" << endl;
	fi->dump(cout);
	cout << "Dumping CFG via Analyst:" << endl;
	an->dump_cfg(cout, fi->get_sexp());
	cout << "Dumping symbol table:" << endl;
	SymbolTable * st = fi->get_scope()->get_symbol_table();
	st->dump(cout);
      }
    }
    ParseInfo::set_analysis_ok(true);
  }
  catch (AnalysisException ae) {
    // One phase of analysis rejected a program. Get rid of the
    // information in preparation to compile trivially.
    rcc_warn(ae.what());
    rcc_warn("analysis encountered difficulties; compiling trivially");
    clearProperties();
    ParseInfo::set_analysis_ok(false);
  }

  // We had to make our program one big function to use
  // OpenAnalysis. Now forget the function definition and assignment
  // and just look at the list of expressions.
  SEXP r_expressions = curly_body(CAR(fundef_body_c(CAR(assign_rhs_c(program)))));

  SubexpBuffer sb;
  string program_str = sb.op_program(r_expressions, "R_GlobalEnv", file_initializer_name,
				     output_main_program, output_default_args);

  if (analysis_debug && ParseInfo::analysis_ok()) {
    // output call graph in DOT form
    cout << "Dumping call graph in DOT form:" << endl;
    OACallGraphAnnotationMap::get_instance()->dumpdot(cout);
    
    // output data flow results
    cout << "Dumping call graph DF analysis:" << endl;
    OACallGraphAnnotationMap::get_instance()->dump(cout);
  }

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

  out_file << program_str;
  
  if (!out_file) {
    rcc_error("Couldn't write to file " + out_filename);
  }
  return (ParseInfo::get_problem_flag() ? 1 : 0);
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

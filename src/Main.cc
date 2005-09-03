// Copyright (c) 2003-2005 John Garvin 
//
// July 11, 2003 
//
// Parses R code, turns into C code that uses internal R functions.
// Attempts to output some code in regular C rather than using R
// functions.
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
///

#include <iostream>

#include <include/R/R_Internal.h>

#include <analysis/AnalysisResults.h>
#include <analysis/IRInterface.h>
#include <analysis/UseDefSolver.h>
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

int main(int argc, char *argv[]) {
  int i;
  char *fullname_c;
  string fullname, libname, out_filename, path, exprs;
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
    c = getopt(argc, argv, "adf:lmo:");
    if (c == -1) {
      break;
    }
    switch(c) {
    case 'a':
      output_default_args = false;
      break;
    case 'd':
      analysis_debug = true;
      break;
    case 'f':
      ParseInfo::direct_funcs.push_back(string(optarg));
      break;
    case 'l':
      global_self_allocate = true;
      break;
    case 'm':
      output_main_program = false;
      break;
    case 'o':
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
      err("Unknown error: getopt() returned " + i_to_s(c) + "\n");
      break;
    }
  }

  // get filename, if it exists
  if (optind < argc) {
    in_file_exists = true;
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
  exprs += "\nstatic void exec() {\n";

  // FIXME: load standard assertions here (before parsing)

  SEXP program = parse_R_as_function(in_file);

  R_Analyst an(program);
  if (analysis_debug) {
    FuncInfo *scope_tree = an.get_scope_tree_root();
    FuncInfoIterator fii(scope_tree);
    for(FuncInfo *fi; fi = fii.Current(); fii++) {
      cout << "New procedure:" << endl;
      fi->dump(cout);
      an.dump_cfg(cout, fi->get_defn());
    }
  }

  // We had to make our program one big function to use
  // OpenAnalysis. Now forget the function definition and assignment
  // and just look at the list of expressions.
  SEXP expressions = curly_body(CAR(fundef_body_c(CAR(assign_rhs_c(program)))));

  // count expressions so we can number them
  SEXP e = expressions;
  n_exprs = 0;
  while (e != R_NilValue) {
    n_exprs++;
    e = CDR(e);
  }

  for(i=0; i<n_exprs; i++) {
    exprs += indent("SEXP e" + i_to_s(i) + ";\n");
  }

  // output expressions
  for(i=0; i<n_exprs; i++) {
    SubexpBuffer subexps;
    Expression exp = subexps.op_exp(CAR(expressions), "R_GlobalEnv");
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
    expressions = CDR(expressions);
  }
  exprs += indent("UNPROTECT(" + i_to_s(ParseInfo::global_constants->get_n_prot())
		  + "); /* c_ */\n");
  exprs += "}\n\n";
  
  if (ParseInfo::get_problem_flag()) {
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

  // output to file
  out_file << rcc_path_prefix << "rcc_generated_header.h\"\n";
  out_file << "\n";

  ParseInfo::global_fundefs->output_ip();
  ParseInfo::global_fundefs->finalize();

  ParseInfo::global_constants->output_ip();
  ParseInfo::global_constants->finalize();

  out_file << ParseInfo::global_fundefs->output_decls();
  out_file << ParseInfo::global_constants->output_decls();

  string file_initializer_name = string("R_init_") + libname;

  string header;
  header += "\nvoid " + file_initializer_name + "() {\n";
  // The name R_init_<libname> is a signal to the R dynamic loader
  // telling it to execute the function immediately upon loading.
  for(i=0; i<ParseInfo::global_constants->get_n_inits(); i++) {
    header += indent(ParseInfo::global_constants->get_init_str() + i_to_s(i) + "();\n");
  }
  header += indent("exec();\n");
  header += "}\n";
  out_file << header;

  out_file << ParseInfo::global_constants->output_defs();
  out_file << exprs;
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
    err("Couldn't write to file " + out_filename);
  }
  if (ParseInfo::get_problem_flag()) {
    return 1;
  } else {
    return 0;
  }
}

static void arg_err() {
  cerr << "Usage: rcc [input-file] [-a] [-d] [-l] [-m] [-o output-file] [-f function-name]*\n";
  exit(1);
}

static void set_funcs(int argc, char *argv[]) {
  int i;
  for(i=0; i<argc; i++) {
    ParseInfo::direct_funcs.push_back(string(*argv++));
  }
}

string make_symbol(SEXP e) {
  if (e == R_MissingArg) {
    return "R_MissingArg";
  } else if (e == R_UnboundValue) {
    return "R_UnboundValue";
  } else {
    string name = string(CHAR(PRINTNAME(e)));
    map<string,string>::iterator pr = ParseInfo::symbol_map.find(name);
    if (pr == ParseInfo::symbol_map.end()) {  // not found
      string var = ParseInfo::global_constants->new_sexp_unp_name(name);
      string qname = quote(name);
      ParseInfo::global_constants->appl(var, FALSE, "Rf_install", 1, &qname);
      ParseInfo::symbol_map.insert(pair<string,string>(name, var));
      return var;
    } else {
      return pr->second;
    }
  }
}

// initialize statics in SubexpBuffer
unsigned int SubexpBuffer::n = 0;
unsigned int SubexpBuffer::global_temps = 0;

//! Convert an Output into an Expression. Will go away as soon as
//! everything uses Output instead of Expression.
const Expression SubexpBuffer::output_to_expression(const Output op) {
  append_decls(op.get_decls());
  append_defs(op.get_code());
  ParseInfo::global_constants->append_decls(op.get_g_decls());
  ParseInfo::global_constants->append_defs(op.get_g_code());
  return Expression(op.get_handle(),
		    (op.get_dependence() == DEPENDENT),
		    op.get_visibility(),
		    op.get_del_text());
}

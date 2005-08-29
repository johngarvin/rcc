/* mode: -*-C++-*-
 * Copyright (c) 2003-2005 John Garvin 
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

#ifndef MAIN_H
#define MAIN_H

#define __USE_STD_IOSTREAM

#include <fstream>
#include <list>
#include <string>

extern "C" {

#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>

} //extern "C"

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/CFG/Interface.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>

#include <annotations/AnnotationSet.hpp>
#include <support/StringUtils.h>
#include <support/Parser.h>
#include <analysis/Analyst.h>
#include <analysis/Utils.h>

#include <GetName.h>
#include <Visibility.h>
#include <Macro.h>

bool is_special(std::string func);

//!  Expression is a struct returned by the op_ functions representing a
//!  subexpression in the output.
//!  var = the name of the variable storing the expression
//!  is_dep = whether the expression depends on the current
//!           environment. If false, the expression can be hoisted out
//!           of an f-function.
//!  is_visible = whether the expression should be printed if it
//!               appears at top level in R.
//!  is_alloc = whether the expression is locally allocated.
//!  del_text = code to clean up after the final use of the
//!             expression. Most commonly a call to UNPROTECT_PTR.
struct Expression {
  std::string var;
  bool is_dep;
  visibility is_visible;
  bool is_alloc;
  std::string del_text;
  Expression() {}
  Expression(std::string v, bool d, visibility vis, std::string dt) {
    var = v;
    is_dep = d;
    is_visible = vis;
    is_alloc = FALSE;
    del_text = dt;
  }
};

class SubexpBuffer {
protected:
  const std::string prefix;
  static unsigned int n;  // see also definition immediately
			  // following class definition
  unsigned int prot;
  //  AllocList alloc_list;
  std::string edefs;
public:
  SubexpBuffer::SubexpBuffer(std::string pref = "v", bool is_c = FALSE)
    : prefix(pref), is_const(is_c) {
    has_i = FALSE;
    prot = 0;
    decls = edefs = "";
    encl_fn = this;
  }
  virtual ~SubexpBuffer();
  SubexpBuffer &operator=(SubexpBuffer &sb);

  std::string decls;
  SubexpBuffer * encl_fn;
  bool has_i;
  const bool is_const;

  virtual void finalize();
  const std::string &output_decls();
  const std::string &output_defs();
  virtual void append_decls(std::string s);
  virtual void append_defs(std::string s);
  virtual std::string new_var();
  virtual std::string new_var_unp();
  virtual std::string new_var_unp_name(std::string name);
  int get_n_vars();
  int get_n_prot();
  std::string new_sexp();
  std::string new_sexp_unp();
  std::string new_sexp_unp_name(std::string name);
  std::string protect_str(std::string str);
  void del(Expression exp);
  SubexpBuffer new_sb();
  SubexpBuffer new_sb(std::string pref);
  std::string output();
  void output_ip();

  void appl(std::string var, bool do_protect, std::string func, int argc, ...);
  std::string SubexpBuffer::appl1(std::string func, std::string arg);
  std::string SubexpBuffer::appl1_unp(std::string func, std::string arg);
  std::string SubexpBuffer::appl2(std::string func, std::string arg1, std::string arg2);
  std::string SubexpBuffer::appl2_unp(std::string func, std::string arg1, std::string arg2);
  std::string SubexpBuffer::appl3(std::string func, std::string arg1, std::string arg2, std::string arg3);
  std::string SubexpBuffer::appl3_unp(std::string func, std::string arg1, std::string arg2, std::string arg3);
  std::string SubexpBuffer::appl4(std::string func,
				  std::string arg1, 
				  std::string arg2, 
				  std::string arg3, 
				  std::string arg4);
  std::string SubexpBuffer::appl5(std::string func,
				  std::string arg1, 
				  std::string arg2, 
				  std::string arg3, 
				  std::string arg4,
				  std::string arg5);
  std::string SubexpBuffer::appl5_unp(std::string func, 
				      std::string arg1, 
				      std::string arg2, 
				      std::string arg3, 
				      std::string arg4,
				      std::string arg5);
  std::string SubexpBuffer::appl6(std::string func,
				  std::string arg1,
				  std::string arg2,
				  std::string arg3,
				  std::string arg4,
				  std::string arg5,
				  std::string arg6);

  Expression op_exp(SEXP e, std::string rho, bool primFuncArg = FALSE);
  Expression op_primsxp(SEXP e, std::string rho);
  Expression op_symlist(SEXP e, std::string rho);
  Expression op_lang(SEXP e, std::string rho);
  Expression op_promise(SEXP e);
  Expression op_begin(SEXP exp, std::string rho);
  Expression op_if(SEXP e, std::string rho);
  Expression op_for(SEXP e, std::string rho);
  Expression op_while(SEXP e, std::string rho);
  Expression op_c_return(SEXP e, std::string rho);
  Expression op_fundef(SEXP e, std::string rho, std::string opt_R_name = "");
  Expression op_special(SEXP e, SEXP op, std::string rho);
  Expression op_builtin(SEXP e, SEXP op, std::string rho);
  Expression op_set(SEXP e, SEXP op, std::string rho);
  Expression op_subscriptset(SEXP e, std::string rho);
  Expression op_clos_app(Expression op1, SEXP args, std::string rho);
  Expression op_arglist(SEXP e, std::string rho);
  Expression op_arglist_local(SEXP e, std::string rho);
  Expression op_literal(SEXP e, std::string rho);
  Expression op_list_local(SEXP e, std::string rho, bool literal = TRUE, 
			   bool primFuncArgList = FALSE, std::string opt_l_car = "");
  Expression op_list(SEXP e, std::string rho, bool literal, bool primFuncArgList = FALSE);
  Expression op_list_help(SEXP e, std::string rho, 
			  SubexpBuffer & consts, 
			  std::string & out_const, bool literal);
  Expression op_string(SEXP s);
  Expression op_vector(SEXP e);
};

//! Huge functions are hard on compilers like gcc. To generate code
//! that goes down easy, we split up the constant initialization into
//! several functions.
class SplitSubexpBuffer : public SubexpBuffer {
public:
  SplitSubexpBuffer(std::string pref = "v", bool is_c = FALSE, int thr = 300, std::string is = "init")
    : SubexpBuffer(pref, is_c), threshold(thr), init_str(is) {
    init_fns = 0;
  }

  static SplitSubexpBuffer global_constants;

  virtual void finalize();
  void virtual append_defs(std::string d);
  int virtual defs_location();
  void virtual insert_def(int loc, std::string d);
  unsigned int get_n_inits();
  std::string get_init_str();
  virtual std::string new_var();
  virtual std::string new_var_unp();
  virtual std::string new_var_unp_name(std::string name);

private:
  const unsigned int threshold;
  const std::string init_str;
  unsigned int init_fns;
  std::string split_defs;
  void flush_defs();
};

//! ProgramInfo:
//! collection of global (whole-program) information collected during code generation
// TODO: encapsulate
class ProgramInfo {
 public:
  static std::map<std::string, std::string> func_map;
  static std::map<std::string, std::string> symbol_map;
  static std::map<std::string, std::string> string_map;
  static std::map<double, std::string> sc_real_map;
  static std::map<int, std::string> sc_logical_map;
  static std::map<int, std::string> sc_integer_map;
  static std::map<int, std::string> primsxp_map;
  static std::list<std::string> direct_funcs;
  static SubexpBuffer global_fundefs;
  static SplitSubexpBuffer global_constants;
  static SubexpBuffer global_labels;
};

static void arg_err();
static void set_funcs(int argc, char *argv[]);
std::string make_fundef(SubexpBuffer * this_buf, std::string func_name, SEXP args, SEXP code);
std::string make_fundef_c(SubexpBuffer * this_buf, std::string func_name, SEXP args, SEXP code);
std::string make_symbol(SEXP e);

#endif

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

// File: SubexpBuffer.h
//
// A buffer containing C code, including declarations and definitions.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef SUBEXP_BUFFER_H
#define SUBEXP_BUFFER_H

#include <string>

#include <Output.h>

#include <analysis/EagerLazy.h>

#include <include/Protection.h>
#include <include/ResultStatus.h>
#include <include/R/R_RInternals.h>

#include <codegen/SubexpBuffer/Expression.h>

namespace RAnnot {
  class FuncInfo;
}

class SubexpBuffer {
public:
  explicit SubexpBuffer(std::string pref = "v", bool is_c = false);
  virtual ~SubexpBuffer();
  SubexpBuffer &operator=(SubexpBuffer &sb);

  std::string decls;
  SubexpBuffer * encl_fn;
  bool has_i;             // int iterator
  bool has_di;            // double iterator
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
  SubexpBuffer new_sb(std::string pref);
  std::string output();
  void output_ip();

  void appl(std::string var, 
	    Protection do_protect, 
	    std::string func,
	    std::string cmnt,
	    int argc, ...);

  std::string appl1(std::string func,
		    std::string cmnt,
		    std::string arg,
		    Protection resultProtection = Protected);

  std::string appl2(std::string func,
		    std::string cmnt,
		    std::string arg1, 
		    std::string arg2, 
		    Protection resultProtection = Protected);

  std::string appl3(std::string func,
		    std::string cmnt,
		    std::string arg1, 
		    std::string arg2, 
		    std::string arg3,
		    Protection resultProtection = Protected);

  std::string appl4(std::string func,
		    std::string cmnt,
		    std::string arg1, 
		    std::string arg2, 
		    std::string arg3, 
		    std::string arg4, 
		    Protection resultProtection = Protected);

  std::string appl5(std::string func,
		    std::string cmnt,
		    std::string arg1, 
		    std::string arg2, 
		    std::string arg3, 
		    std::string arg4,
		    std::string arg5, 
		    Protection resultProtection = Protected);

  std::string appl6(std::string func,
		    std::string cmnt,
		    std::string arg1,
		    std::string arg2,
		    std::string arg3,
		    std::string arg4,
		    std::string arg5,
		    std::string arg6, 
		    Protection resultProtection = Protected);

  std::string appl7(std::string func,
		    std::string cmnt,
		    std::string arg1,
		    std::string arg2,
		    std::string arg3,
		    std::string arg4,
		    std::string arg5,
		    std::string arg6, 
		    std::string arg7, 
		    Protection resultProtection = Protected);

  std::string op_program(SEXP e, std::string rho, std::string func_name,
			 bool output_main_program, bool output_default_args);
  Expression op_exp(SEXP cell, std::string rho, 
		    Protection resultProtection = Protected, 
		    bool fullyEvaluatedResult = false,
		    ResultStatus resultStatus = ResultNeeded);
  Expression op_var_use(SEXP e, std::string rho,
			Protection resultProtection = Protected,
			bool fullyEvaluatedResult = false);
  Expression op_fun_use(SEXP e, std::string rho,
			Protection resultProtection = Protected,
			bool fullyEvaluatedResult = false);
  Expression op_var_def(SEXP cell, std::string rhs, std::string rho);
  Expression op_primsxp(SEXP e, std::string rho);
  Expression op_symlist(SEXP e, std::string rho);
  Expression op_lang(SEXP cell, std::string rho, Protection resultProtection,
		    ResultStatus resultStatus = ResultNeeded); 
  Expression op_promise(SEXP e);
  Expression op_begin(SEXP exp, std::string rho,
		      Protection resultProtection,
		      ResultStatus resultStatus = ResultNeeded); 
  Expression op_if(SEXP e, std::string rho,
		   ResultStatus resultStatus = ResultNeeded); 
  Expression op_for(SEXP e, std::string rho,
		   ResultStatus resultStatus = ResultNeeded); 
  Expression op_for_colon(SEXP e, std::string rho,
			  ResultStatus resultStatus = ResultNeeded);
  Expression op_while(SEXP e, std::string rho);
  Expression op_return(SEXP e, std::string rho);
  Expression op_struct_field(SEXP e, SEXP op, std::string rho, Protection resultProtection);
  Expression op_subscript(SEXP e, SEXP op, std::string rho, Protection resultProtection);
  Expression op_break(SEXP e, std::string rho);
  Expression op_fundef(SEXP e, std::string rho, Protection resultProtection);
  Expression op_special(SEXP cell, SEXP op, std::string rho, 
			Protection resultProtection,
			ResultStatus resultStatus = ResultNeeded); 
  Expression op_builtin(SEXP e, SEXP op, std::string rho, 
			Protection resultProtection);
  Expression op_set(SEXP cell, SEXP op, std::string rho,
		    Protection resultProtection);
  Expression op_subscriptset(SEXP cell, std::string rho, 
			     Protection resultProtection);
  Expression op_clos_app(RAnnot::FuncInfo * fi_if_known,
			 Expression op1,
			 SEXP args,
			 std::string rho,
			 Protection resultProtection,
			 EagerLazyT laziness = LAZY);
  Expression op_closure(SEXP e, std::string rho, Protection resultProtection);
  Expression op_literal(SEXP e, std::string rho);
  Expression op_list_local(SEXP e, std::string rho, bool literal = TRUE, 
			   bool fullyEvaluatedResult = FALSE, std::string opt_l_car = "");
  Expression op_list(SEXP e, std::string rho, bool literal, 
		     Protection protectResult, 
		     bool fullyEvaluatedResult = FALSE);
  Expression op_list_help(SEXP e, std::string rho, 
			  SubexpBuffer & consts, 
			  std::string & out_const, bool literal);
  Expression op_string(SEXP s);
  Expression op_vector(SEXP e);
  std::string new_location();

  /// Convert an Output into an Expression. Will go away as soon as
  /// everything uses Output instead of Expression.
  const Expression output_to_expression(const Output op);
  const std::string get_prefix() { return prefix; }

protected:
  const std::string prefix;
  static unsigned int n;
  unsigned int prot;
  std::string edefs;
  static unsigned int global_temps;
};

#endif

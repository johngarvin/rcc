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

// File: CodeGen.h
//
// A class with methods that output C code given R objects. Intended
// as a replacement for the ugliness in SubexpBuffer, but conversion
// is not yet complete.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef CODE_GEN_H
#define CODE_GEN_H

#include <string>

#include <include/Protection.h>
#include <include/R/R_RInternals.h>

#include <Output.h>
#include <CScope.h>

class CodeGen {
public:
  static Output op_exp(SEXP e, std::string rho,
		       bool fullyEvaluatedResult = false);
  static Output op_var_use(SEXP cell, std::string rho, 
			   Protection resultProtected, 
			   bool fullyEvaluatedResult);
  static Output op_closure(SEXP e, std::string rho);
  static Output op_primsxp(SEXP e, std::string rho);
  static Output op_lang(SEXP e, std::string rho);
  static Output op_promise(SEXP e);
  static Output op_begin(SEXP exp, std::string rho);
  static Output op_if(SEXP e, std::string rho);
  static Output op_for(SEXP e, std::string rho);
  static Output op_while(SEXP e, std::string rho);
  static Output op_return(SEXP e, std::string rho);
  static Output op_break(SEXP e, std::string rho);
  static Output op_fundef(SEXP e, std::string rho, std::string opt_R_name = "");
  static Output op_special(SEXP e, SEXP op, std::string rho);
  static Output op_builtin(SEXP e, SEXP op, std::string rho);
  static Output op_set(SEXP e, SEXP op, std::string rho);
  static Output op_subscriptset(SEXP e, std::string rho);
  static Output op_clos_app(std::string clos_h, SEXP args, std::string rho);
  static Output op_literal(SEXP e, std::string rho);
  static Output op_list(SEXP e,
			std::string rho,
			bool literal,
			bool promFuncArgList = false);
  static Output op_string(SEXP e);
  static Output op_vector(SEXP vec);

private:
  static const CScope s_scope;
};

#endif

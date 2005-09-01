// -*- Mode: C++ -*-
#ifndef CODE_GEN_H
#define CODE_GEN_H

#include <string>

#include <include/R/R_RInternals.h>

#include <Output.h>

class CScope;

class CodeGen {
public:
  static Output op_exp(SEXP e, std::string rho, bool primFuncArg);
  static Output op_primsxp(SEXP e, std::string rho);
  static Output op_literal(CScope scope, SEXP e, std::string rho);
  static Output op_list(CScope scope,
			SEXP e,
			std::string rho,
			bool literal,
			bool promFuncArgList = false);
  static Output op_string(SEXP e);
  static Output op_vector(SEXP vec);

  static const std::string new_label(const std::string prefix);
private:
  static unsigned int last_buffer_num;
};

#endif

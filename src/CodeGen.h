#ifndef CODE_GEN_H
#define CODE_GEN_H

#include <include/R/R_RInternals.h>

#include <Output.h>

class CodeGen {
 public:
  static Output op_exp(SEXP e, std::string rho, bool primFuncArg);
  static Output op_literal(SEXP e, std::string rho);
  static Output op_list(SubexpBuffer & buf,
			SEXP e,
			std::string rho,
			bool literal,
			bool promFuncArgList /* = false */);
  static Output op_string(SEXP e);
  static Output op_vector(SEXP vec);
};

#endif

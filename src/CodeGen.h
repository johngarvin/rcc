#ifndef CODE_GEN_H
#define CODE_GEN_H

#include <include/R/R_RInternals.h>

#include <Output.h>

class CodeGen {
 public:
  static Output op_vector(SEXP vec);
};

#endif

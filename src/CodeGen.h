#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "MyRInternals.h"
#include "Output.h"

class CodeGen {
 public:
  static Output op_vector(SEXP vec);
};

#endif

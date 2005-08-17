#include "R_IRInterface.h"

void opt_matmul(OA::OA_ptr<OA::CFG::CFGIRInterface> rir, SEXP exp);
extern "C" {
  SEXP opt_matmul_plain(SEXP e);
}

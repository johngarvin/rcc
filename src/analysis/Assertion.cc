#include <analysis/Utils.h>
#include "Assertion.h"

namespace RAnnot {

void process_assert(SEXP assertion, FuncInfo* fi) {
  if (is_rcc_assert(assertion)) {
    // rcc_assert
    SEXP list = CDR(assertion);
    for (; list != R_NilValue; list = CDR(list)) {
      SEXP e = CAR(list);
      if (CAR(e) == Rf_install("value")) {
      } else if (CAR(e) == Rf_install("prom")) {
      }
    }
  } else if (is_rcc_assert_exp(assertion)) {
    // rcc_assert_expression
  }
}

}

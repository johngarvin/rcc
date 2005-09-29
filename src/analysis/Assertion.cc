#include <analysis/Utils.h>
#include <analysis/AnalysisResults.h>
#include "Assertion.h"

namespace RAnnot {

void process_assert(SEXP assertion, FuncInfo* fi) {
  if (is_rcc_assert(assertion)) {
    // rcc_assert
    SEXP list = CDR(assertion);
    for (; list != R_NilValue; list = CDR(list)) {
      SEXP e = CAR(list);
      SEXP v = CADR(e);
      char* vname = CHAR(PRINTNAME(v));
      int position = fi->findArgPosition(vname);
      SEXP arg = fi->getArg(position);
      FormalArgInfo* fargInfo = getProperty(FormalArgInfo, arg);
      if (CAR(e) == Rf_install("value")) {
	fargInfo->setIsValue(true);
      } else if (CAR(e) == Rf_install("prom")) {
	fargInfo->setIsValue(false);
      }
    }
  } else if (is_rcc_assert_exp(assertion)) {
    // rcc_assert_expression
  }
}

}

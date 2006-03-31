#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnalysisResults.h>

#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_while(SEXP e, string rho) {
  return Expression::bogus_exp;
}


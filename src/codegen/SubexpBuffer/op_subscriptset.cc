#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>

#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_subscriptset(SEXP e, string rho) {
  // CAR(e) = "<-", "=", etc.
  // CADR(e) = LHS
  //   CAR(CADR(e)) = "["
  //   CADR(CADR(e)) = array
  //   CADDR(CADR(e)) = subscript
  // CADDR(e) = RHS
  SEXP array = CADR(CADR(e));
  SEXP sub = CADDR(CADR(e));
  SEXP rhs = CADDR(e);
  Expression a_sym = op_literal(array, rho);
  Expression a = op_exp(array, rho);
  Expression s = op_exp(sub, rho);
  Expression r = op_exp(rhs, rho);
  string assign = appl3("rcc_subassign", a.var, s.var, r.var);
  string defs = "defineVar(" + a_sym.var + ", " + assign + ", " + rho + ");\n";
  del(a_sym); del(s); del(r); defs += unp(assign);
  append_defs(defs);
  a.is_visible = INVISIBLE;
  return a;
}

#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>
#include <support/StringUtils.h>

#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_subscriptset(SEXP e, string rho, 
					 Protection resultProtection) {
  // CAR(e) = "<-", "=", etc.
  // CADR(e) = LHS
  //   CAR(CADR(e)) = "["
  //   CADR(CADR(e)) = array
  //   CADDR(CADR(e)) = subscript
  // CADDR(e) = RHS
#if 0
  SEXP lhs = CAR(assign_lhs_c(e));
  SEXP array_c = subscript_lhs_c(lhs);
  SEXP sub_c = subscript_rhs_c(lhs);
  SEXP rhs_c = assign_rhs_c(e);
  Expression a_sym = op_literal(CAR(array_c), rho);
  Expression a = op_exp(array_c, rho, Protected, true);
  Expression s = op_exp(sub_c, rho);
  Expression r = op_exp(rhs_c, rho);
  string assign = appl3("rcc_subassign", a.var, s.var, r.var);
  string defs = "defineVar(" + a_sym.var + ", " + assign + ", " + rho + ");\n";
  del(a_sym);
  del(s);
  del(r);
  defs += unp(assign);
  append_defs(defs);
  if (resultProtection == Unprotected) {
    del(a);
    a.del_text = "";
  }
  a.is_visible = INVISIBLE;

  return a;
#else
  Expression args = op_list(CDR(e), rho, false, Protected, true);
  string assign = appl4("do_subassign", "R_NilValue", "R_NilValue",
					 args.var, rho,
					 resultProtection);
  string deleteText;
  if (resultProtection == Protected) deleteText = unp(assign);
  return Expression(assign, TRUE, INVISIBLE, deleteText);
#endif
}

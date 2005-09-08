#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <Visibility.h>

using namespace std;

//!  Output a list using locally allocated storage instead of R's
//!  allocation mechanism. The literal argument determines whether the
//!  CARs are to be output literally or programatically. opt_l_car is an
//!  optional string used mostly for applyClosure arguments. If it is
//!  nonempty, it makes the first CONS a LANGSXP whose CAR is the given
//!  string.
Expression SubexpBuffer::op_list_local(SEXP e, string rho,
				       bool literal /* = TRUE */, 
				       bool fullyEvaluatedResult /* = FALSE */,
				       string opt_l_car /* = "" */) {
  if (TYPEOF(e) != LISTSXP && TYPEOF(e) != LANGSXP && TYPEOF(e) != NILSXP) {
    err("non-list found in op_list_local\n");
  }
  
  if (opt_l_car.empty()) {
    //      return output_to_expression(CodeGen::op_list(CScope(prefix + "_" + i_to_s(n)), e, rho, literal, fullyEvaluatedResult));
    return op_list(e, rho, literal, fullyEvaluatedResult);
  } else {
    //      Expression cdr = output_to_expression(CodeGen::op_list(CScope(prefix + "_" + i_to_s(n)), e, rho, literal, fullyEvaluatedResult));
    Expression cdr = op_list(e, rho, literal, fullyEvaluatedResult);
    string out = appl2("lcons", opt_l_car, cdr.var);
    del(cdr);
    return Expression(out, cdr.is_dep, VISIBLE, unp(out));
  }

#if 0
  bool list_dep = FALSE;
  if (e == R_NilValue) return nil_exp;
  int i, len, init_for;
  string set_list, dt;
  string list = new_sexp();
  if (opt_l_car.empty()) {
    len = Rf_length(e);
    if (len == 0) return nil_exp;
    init_for = 0;
  } else {
    len = 1 + Rf_length(e);
    init_for = 1;
    set_list += "CAR(" + list + ") = " + opt_l_car + ";\n";
    set_list += "TYPEOF(" + list + ") = LANGSXP;\n";
    dt += "TYPEOF(" + list + ") = LISTSXP;\n";
  }
  for(i=init_for; i<len; i++) {
    Expression car_exp;
    if (literal) {
      car_exp = op_literal(CAR(e), rho);
    } else {
      car_exp = op_exp(CAR(e), rho, fullyEvaluatedResult);
    }
    list_dep = list_dep || car_exp.is_dep;
    set_list += "CAR(" + list + " - " + i_to_s(i) + ") = " + car_exp.var + ";\n";
    dt += car_exp.del_text;
    if (TYPEOF(e) == LANGSXP) {
      set_list += "TYPEOF(" + list + " - " + i_to_s(i) + ") = LANGSXP;\n";
      dt += "TYPEOF(" + list + " - " + i_to_s(i) + ") = LISTSXP;\n";
    }
    if (TAG(e) != R_NilValue) {
      Expression tag_exp = op_literal(TAG(e), rho);
      set_list += "TAG(" + list + " - " + i_to_s(i) + ") = " + tag_exp.var + ";\n";
      dt += "TAG(" + list + " - " + i_to_s(i) + ") = R_NilValue;\n";
    }
    e = CDR(e);
  }
#if 0
  alloc_list.add(list, defs.length(), len);
#endif
  append_defs(set_list);
  Expression out = Expression(list, list_dep, VISIBLE, dt);
  out.is_alloc = TRUE;
  return out;
#endif
}

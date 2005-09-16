#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <CodeGenUtils.h>
#include <Visibility.h>


using namespace std;

//! Output a sequence of statements
Expression SubexpBuffer::op_begin(SEXP exp, string rho) {
  Expression e;
  string var = new_sexp();
  while (exp != R_NilValue) {
    SubexpBuffer temp = new_sb("tmp_be");
    //    temp.encl_fn = this;
    e = temp.op_exp(exp, rho);

    string code;
    code += temp.output();
    if (CDR(exp) == R_NilValue) {
      code += emit_assign(var, e.var);
    } else {
      code += e.del_text;
    }
    append_defs(emit_in_braces(code));
    exp = CDR(exp);
  }
  if (e.del_text.empty()) {
    return Expression(var, e.is_dep, e.is_visible, "");
  } else if (e.del_text == "UNPROTECT_PTR(" + e.var + ");\n") {
    return Expression(var, e.is_dep, e.is_visible, unp(var));
  } else {
    err("Encountered unhandled op_begin delete-text");
    return Expression::bogus_exp;
  }
}
  

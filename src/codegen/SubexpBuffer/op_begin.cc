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
Expression SubexpBuffer::op_begin(SEXP exp, string rho,
				  Protection resultProtection,
				  ResultStatus resultStatus)
{
  Expression e;
  string var;
  string cleanup;
  while (exp != R_NilValue) {
    SEXP next = CDR(exp);
    SubexpBuffer temp = new_sb("tmp_be");
    //    temp.encl_fn = this;
    ResultStatus rs = (next == R_NilValue) ? resultStatus : NoResultNeeded;

    e = temp.op_exp(exp, rho, Unprotected, false, rs);
    //    assert(e.del_text.empty());  // given Unprotected; should have no unprotect code
    string code = temp.output();

    if (next == R_NilValue) {
      if (resultStatus == ResultNeeded) {
	var = new_sexp();
	string assign = emit_assign(var, e.var);
	if (resultProtection == Protected) {
	  assign += protect_str(var) + ";\n";
	  cleanup = unp(var);
	}
	code += assign;
      }
    }
    append_defs(emit_in_braces(code, resultProtection == Unprotected));
    exp = next;
  }
  return Expression(var, e.is_dep, e.is_visible, cleanup);
}
  

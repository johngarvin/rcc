#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <Visibility.h>
#include <CodeGen.h>

using namespace std;

//! Output an application of a closure to actual arguments.
Expression SubexpBuffer::op_clos_app(Expression op1, SEXP args,
				     string rho,
				     Protection resultProtection) {
  int unprotcnt = 0;
  string arglist;

  // Unlike most R internal functions, applyClosure actually uses its
  // 'call' argument, so we can't just call it R_NilValue.

#ifdef USE_OUTPUT_CODEGEN
  Expression args1 = output_to_expression(CodeGen::op_list(args, rho, true));
#else
  Expression args1 = op_list(args, rho, true, Protected);
#endif
  string call_str = appl2("lcons", op1.var, args1.var);
  unprotcnt++;  // call_str
  string del_args = "";
  if (args1.var == "R_NilValue") {
    arglist = "R_NilValue";
  } else {
    arglist = appl2("promiseArgs", args1.var, rho);
    unprotcnt++;
  }
  string out = appl5("applyClosure ",
		     call_str,
		     op1.var,
		     arglist,
		     rho,
		     "R_NilValue", Unprotected);
  if (!op1.del_text.empty()) unprotcnt++;
  if (!args1.del_text.empty()) unprotcnt++;
  if (unprotcnt > 0)
    append_defs("UNPROTECT(" + i_to_s(unprotcnt) + ");\n");
  string cleanup;
  if (resultProtection == Protected) {
    if (unprotcnt > 0)
      append_defs("SAFE_PROTECT(" + out + ");\n");
    else {
      append_defs("PROTECT(" + out + ");\n");
    }
    cleanup = unp(out);
  }
  return Expression(out, TRUE, CHECK_VISIBLE, cleanup);
}


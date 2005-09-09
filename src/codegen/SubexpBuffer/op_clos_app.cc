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
Expression SubexpBuffer::op_clos_app(Expression op1, SEXP args, string rho) {
  // see eval.c:438-9
  Expression call;
  string arglist;

  // Unlike most R internal functions, applyClosure actually uses its
  // 'call' argument, so we can't just call it R_NilValue.

  //Expression args1 = output_to_expression(CodeGen::op_list(CScope(prefix + "_" + i_to_s(n)), args, rho, TRUE));
  Expression args1 = op_list(args, rho, true);
  string call_str = appl2("lcons", op1.var, args1.var);
  call = Expression(call_str, FALSE, VISIBLE, unp(call_str));
  arglist = appl2("promiseArgs", args1.var, rho);
  del(args1);
  string out = appl5("applyClosure ",
		     call.var,
		     op1.var,
		     arglist,
		     rho,
		     "R_NilValue");
  del(call);
  del(op1);
  if (call.var != "R_NilValue") append_defs(unp(arglist));
  return Expression(out, TRUE, CHECK_VISIBLE, unp(out));
}


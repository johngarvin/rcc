#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_Defn.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <CodeGen.h>
#include <GetName.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

// e                 the expression to be translated. It is an application; its CAR is 'op'.
// op                the operation. On entry we know its R type is SPECIALSXP.
// rho               a string representing the environment
// resultProtection  whether the generated expression is protected
// resultStatus      whether the result is needed by later computation
Expression SubexpBuffer::op_special(SEXP e, SEXP op, string rho,
			Protection resultProtection,
			ResultStatus resultStatus)
{
  string out;
  if (PRIMFUN(op) == (CCODE)do_set) {
    return op_set(e, op, rho, resultProtection);
  } else if (PRIMFUN(op) == (CCODE)do_internal) {
    // ".Internal" call
    SEXP internal_call = CADR(e);
    SEXP fun = CAR(internal_call);
    Expression args;
    if (TYPEOF(INTERNAL(fun)) == BUILTINSXP) {
#ifdef USE_OUTPUT_CODEGEN
      args = output_to_expression(CodeGen::op_list(CDR(internal_call), rho, false, true));
#else
      args = op_list(CDR(internal_call), rho, false, Protected, true);
#endif
    } else {
      args = op_exp(internal_call, rho);
    }
#ifdef USE_OUTPUT_CODEGEN
    Expression func = output_to_expression(CodeGen::op_primsxp(INTERNAL(fun), rho));
#else
    Expression func = ParseInfo::global_constants->op_primsxp(INTERNAL(fun), rho);
#endif
    out = appl4(get_name(PRIMOFFSET(INTERNAL(fun))),
		"R_NilValue", func.var, args.var, rho);
    del(func);
    del(args);
    return Expression(out, TRUE, INVISIBLE, unp(out));
  } else if (PRIMFUN(op) == (CCODE)do_function) {
    return op_fundef(e, rho, resultProtection);
  } else if (PRIMFUN(op) == (CCODE)do_begin) {
    return op_begin(CDR(e), rho, resultProtection, resultStatus);
  } else if (PRIMFUN(op) == (CCODE)do_if) {
#ifdef USE_OUTPUT_CODEGEN
    return output_to_expression(CodeGen::op_if(e, rho));
#else
    return op_if(e, rho, resultStatus);
#endif
  } else if (PRIMFUN(op) == (CCODE)do_for) {
    return op_for(e, rho, resultStatus);
    //
    // } else if (PRIMFUN(op) == (CCODE)do_while) {
    //   return op_while(e, rho);
    //
  } else if (PRIMFUN(op) == (CCODE)do_break) {
    return op_break(CAR(e), rho);
  } else if (PRIMFUN(op) == (CCODE)do_return) {
    return op_return(CDR(e), rho);
  } else {
    // default case for specials: call the (call, op, args, rho) fn
#ifdef USE_OUTPUT_CODEGEN
    Expression op1 = output_to_expression(CodeGen::op_primsxp(op, rho));
    Expression args1 = output_to_expression(CodeGen::op_list(CDR(e), rho, true, true));
#else
    Expression op1 = ParseInfo::global_constants->op_primsxp(op, rho);
    Expression args1 = op_list(CDR(e), rho, true, Protected, true);
#endif
    string call_str = appl2("lcons", op1.var, args1.var);
    Expression call = Expression(call_str, FALSE, VISIBLE, unp(call_str));
    out = appl4(get_name(PRIMOFFSET(op)),
		call.var,
		op1.var,
		args1.var,
		rho, resultProtection);
    string cleanup;	
    if (resultProtection == Protected) cleanup = unp(out);
    del(call);
    del(op1);
    del(args1);
    return Expression(out, TRUE, 
		      1 - PRIMPRINT(op) ? VISIBLE : INVISIBLE, 
		      cleanup);
  }
}

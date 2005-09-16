#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_Defn.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <CodeGen.h>
#include <GetName.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_special(SEXP e, SEXP op, string rho) {
  string out;
  if (PRIMFUN(op) == (SEXP (*)())do_set) {
    return op_set(e, op, rho);
  } else if (PRIMFUN(op) == (SEXP (*)())do_internal) {
    // ".Internal" call
    SEXP fun = CAR(CADR(e));
    Expression args;
    if (TYPEOF(INTERNAL(fun)) == BUILTINSXP) {
      args = op_list(CDR(CADR(e)), rho, false, true);
    } else {
      args = op_exp(CADR(e), rho);
    }
    Expression func = output_to_expression(CodeGen::op_primsxp(INTERNAL(fun), rho));
    out = appl4(get_name(PRIMOFFSET(INTERNAL(fun))),
		"R_NilValue", func.var, args.var, rho);
    del(func);
    del(args);
    return Expression(out, TRUE, INVISIBLE, unp(out));
  } else if (PRIMFUN(op) == (SEXP (*)())do_function) {
    return op_fundef(e, rho);
  } else if (PRIMFUN(op) == (SEXP (*)())do_begin) {
    return op_begin(CDR(e), rho);
  } else if (PRIMFUN(op) == (SEXP (*)())do_if) {
#if 0
    return output_to_expression(CodeGen::op_if(e, rho));
#else
    return op_if(e, rho);
#endif
  } else if (PRIMFUN(op) == (SEXP (*)())do_for) {
    return op_for(e, rho);
    //
    // } else if (PRIMFUN(op) == (SEXP (*)())do_while) {
    //   return op_while(e, rho);
    //
  } else if (PRIMFUN(op) == (SEXP (*)())do_break) {
    return op_break(CAR(e), rho);
  } else if (PRIMFUN(op) == (SEXP (*)())do_return) {
    return op_return(CDR(e), rho);
  } else {
    // default case for specials: call the (call, op, args, rho) fn
    Expression op1 = output_to_expression(CodeGen::op_primsxp(op, rho));
#if 0
    Expression args1 = output_to_expression(CodeGen::op_list(CDR(e), rho, TRUE, TRUE));
#else
    Expression args1 = op_list(CDR(e), rho, true, true);
#endif
    string call_str = appl2("lcons", op1.var, args1.var);
    Expression call = Expression(call_str, FALSE, VISIBLE, unp(call_str));
    out = appl4(get_name(PRIMOFFSET(op)),
		call.var,
		op1.var,
		args1.var,
		rho);
    del(call);
    del(op1);
    del(args1);
    return Expression(out, TRUE, 
		      1 - PRIMPRINT(op) ? VISIBLE : INVISIBLE, 
		      unp(out));
  }
}

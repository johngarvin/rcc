#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_Defn.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <GetName.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_builtin(SEXP e, SEXP op, string rho) {
  string out;
  Expression op1 = ParseInfo::global_constants->op_primsxp(op, rho);
  SEXP args = CDR(e);
  // special case for arithmetic operations
  if (PRIMFUN(op) == (SEXP (*)())do_arith) {

    // R_unary if there's one argument and it's a non-object
    if (args != R_NilValue
	&& CDR(args) == R_NilValue
	&& !Rf_isObject(CAR(args))) { // one argument, non-object
      Expression x = op_exp(args, rho, true);
      out = appl3("R_unary", "R_NilValue", op1.var, x.var);
      del(x);

    // R_binary if two non-object arguments
    } else if (CDDR(args) == R_NilValue && 
	       !Rf_isObject(CAR(args))
	       && !Rf_isObject(CADR(args))) {
      Expression x = op_exp(args, rho, true);
      Expression y = op_exp(CDR(args), rho, true);
      out = appl4("R_binary", "R_NilValue", op1.var, x.var, y.var);
      del(x);
      del(y);
    }
    
  } else {  // common case: call the do_ function
    Expression args1 = op_list(args, rho, false, true);
    out = appl4(get_name(PRIMOFFSET(op)),
		"R_NilValue ",
		op1.var,
		args1.var,
		rho);
    del(args1);
  }

  del(op1);
  return Expression(out, TRUE, 1 - PRIMPRINT(op) ? VISIBLE : INVISIBLE, unp(out));
}

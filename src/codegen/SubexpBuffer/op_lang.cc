#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <Visibility.h>
#include <ParseInfo.h>

using namespace std;

Expression SubexpBuffer::op_lang(SEXP e, string rho) {
  SEXP op;
  string out;
  Expression exp;
  if (TYPEOF(CAR(e)) == SYMSXP) {
    string r_sym = CHAR(PRINTNAME(CAR(e)));

    if (ParseInfo::is_direct(r_sym)) {
      //direct function call
      string func = make_c_id(r_sym) + "_direct";
      //      Expression args = output_to_expression(CodeGen::op_list(CScope(prefix + "_" + i_to_s(n)), CDR(e), rho, FALSE)); // not local; used for env
      Expression args = op_list(CDR(e), rho, false);
      string call = appl1(func, args.var);
      del(args);
      return Expression(call, TRUE, VISIBLE, unp(call));
    } else { // not direct; call via closure
      op = Rf_findFunUnboundOK(CAR(e), R_GlobalEnv, TRUE);
      if (op == R_UnboundValue) {    // user-defined function
	string func = appl2("findFun",
			    make_symbol(CAR(e)),
			    rho);
	return op_clos_app(Expression(func, FALSE, INVISIBLE, unp(func)),
			   CDR(e), rho);

      } else {  // Built-in function, special function, or closure
	if (TYPEOF(op) == SPECIALSXP) {
	  return op_special(e, op, rho);
	} else if (TYPEOF(op) == BUILTINSXP) {
	  return op_builtin(e, op, rho);
	} else if (TYPEOF(op) == CLOSXP) {
	  Expression op1;
	  if (CAR(BODY(op)) == Rf_install(".Internal")) {
	    op1 = op_exp(op, "R_GlobalEnv");
	  } else {
	    op1 = op_exp(op, rho);
	  }
	  Expression out_exp = op_clos_app(op1, CDR(e), rho);
	  return out_exp;
	} else if (TYPEOF(op) == PROMSXP) {
	  // ***************** TEST ME! ******************
	  err("Hey! I don't think we should see a promise in LANGSXP!\n");
	  return op_exp(PREXPR(op), rho);
	} else {
	  err("Internal error: LANGSXP encountered non-function op\n");
	  return Expression::bogus_exp; // never reached
	}
      }
    }
  } else {  // function is not a symbol
    Expression op1;
    op1 = op_exp(CAR(e), rho);
    Expression out_exp = op_clos_app(op1, CDR(e), rho);
    return out_exp;
    // eval.c: 395
  }
}

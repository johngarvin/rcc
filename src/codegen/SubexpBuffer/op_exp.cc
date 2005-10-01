#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>
#include <support/StringUtils.h>
#include <CodeGen.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

#if 0
<<<<<<< op_exp.cc
Expression SubexpBuffer::op_exp(SEXP e, string rho, Protection
				resultProtection, bool fullyEvaluatedResult, 
				ResultStatus resultStatus) {
=======
// op_exp
//
// outputs a representation of the CAR of the given SEXP. Note that
// op_exp takes as the argument the CONS cell containing the
// expression instead of the expression itself; this allows op_var to
// use annotations. (Otherwise, we wouldn't be able to distinguish
// different mentions of the same variable.)

Expression SubexpBuffer::op_exp(SEXP cell, string rho, 
				bool fullyEvaluatedResult) {
  assert(is_cons(cell));
  SEXP e = CAR(cell);
>>>>>>> 1.5
#endif

// op_exp
//
// outputs a representation of the CAR of the given SEXP. Note that
// op_exp takes as the argument the CONS cell containing the
// expression instead of the expression itself; this allows op_var to
// use annotations. (Otherwise, we wouldn't be able to distinguish
// different mentions of the same variable.)

Expression SubexpBuffer::op_exp(SEXP cell, string rho, Protection
				resultProtection, bool fullyEvaluatedResult, 
				ResultStatus resultStatus) {

  assert(is_cons(cell));
  SEXP e = CAR(cell);

  Expression out, formals, body, env;
  switch(TYPEOF(e)) {
  case NILSXP:
    return Expression::nil_exp;
    break;
  case STRSXP:
    return op_string(e);
    break;
  case LGLSXP:
  case INTSXP:
  case REALSXP:
  case CPLXSXP:
      return output_to_expression(CodeGen::op_vector(e));
      //    return op_vector(e);
    break;
  case VECSXP:
    ParseInfo::flag_problem();
    return Expression("<<unimplemented vector>>", TRUE, INVISIBLE, "");
    break;
  case SYMSXP:
#if 0
    if (e == R_MissingArg) {
      return Expression("R_MissingArg", FALSE, INVISIBLE, "");
    } else {
      string sym = make_symbol(e);
      string v = appl2("findVar", sym, rho, Unprotected);
      string cleanup;
      if (fullyEvaluatedResult) {
	v = appl2("eval", v, rho, resultProtection);
	if (resultProtection == Protected) cleanup = unp(v);
      }
      out = Expression(v, TRUE, VISIBLE, cleanup);
      return out;
    }
#else
    return output_to_expression(CodeGen::op_var(cell, rho,
						resultProtection, 
						fullyEvaluatedResult));
#endif
    break;
  case LISTSXP:
#if 0
<<<<<<< op_exp.cc
    return output_to_expression(CodeGen::op_list(CScope(prefix + "_" + i_to_s(n)), e, rho, false, false));
#if 0
    return op_list(e, rho, false, resultPprotection, false);
#endif
=======
    return output_to_expression(CodeGen::op_list(e, rho, false, false));
    //    return op_list(e, rho, false, false);
>>>>>>> 1.5
#endif
    return output_to_expression(CodeGen::op_list(e, rho, false, false));
    break;
  case CLOSXP:
    // johnmc - commented out in merge
    // formals = op_list(FORMALS(e), rho, true, Protected);
#if 0
    formals = op_list(FORMALS(e), rho, true);
    body = op_literal(BODY(e), rho);
    if (rho == "R_GlobalEnv" && !formals.is_dep && !body.is_dep) {
      string v = ParseInfo::global_constants->appl3("mkCLOSXP",
						    formals.var,
						    body.var,
						    rho);
      ParseInfo::global_constants->del(formals);
      ParseInfo::global_constants->del(body);
      out = Expression(v, FALSE, INVISIBLE, "");
    } else {
      string v = appl3("mkCLOSXP",
		       formals.var,
		       body.var,
		       rho);
      del(formals);
      del(body);
      out = Expression(v, TRUE, INVISIBLE, unp(v));
    }
    return out;
#else
    return output_to_expression(CodeGen::op_closure(e, rho));
#endif
    break;
  case ENVSXP:
    ParseInfo::flag_problem();
    return Expression("<<unexpected environment>>", TRUE, INVISIBLE, "");
    break;
  case PROMSXP:
    ParseInfo::flag_problem();
    return Expression("<<unexpected promise>>", TRUE, INVISIBLE, "");
    break;
  case LANGSXP:
    out = op_lang(e, rho, resultProtection, resultStatus);
    return out;
    break;
  case CHARSXP:
    return Expression(appl1("mkChar",
				quote(string(CHAR(e))), Unprotected),
		      FALSE, VISIBLE, "");
    break;
  case SPECIALSXP:
  case BUILTINSXP:
    return ParseInfo::global_constants->op_primsxp(e, rho);
    break;
  case EXPRSXP:
  case EXTPTRSXP:
  case WEAKREFSXP:
    ParseInfo::flag_problem();
    return Expression("<<unimplemented type " + i_to_s(TYPEOF(e)) + ">>",
		      TRUE, INVISIBLE, "");
    break;
  default:
    err("Internal error: op_exp encountered bad type\n");
    return Expression::bogus_exp; // never reached
    break;
  }
}

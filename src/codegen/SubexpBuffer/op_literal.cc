#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_literal(SEXP e, string rho) {
  Expression formals, body, env;
  string v;
  switch (TYPEOF(e)) {
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
    return op_vector(e);
    break;
  case VECSXP:
    ParseInfo::flag_problem();
    return Expression("<<unimplemented vector>>", TRUE, INVISIBLE, "");
    break;
  case SYMSXP:
    return Expression(make_symbol(e), FALSE, VISIBLE, "");
    break;
  case LISTSXP:
    //    return output_to_expression(CodeGen::op_list(CScope(prefix + "_" + i_to_s(n)), e, rho, TRUE));
    return op_list(e, rho, true);
    break;
  case CLOSXP:
    formals = op_symlist(FORMALS(e), rho);
    body = op_literal(BODY(e), rho);
    v = appl3("mkCLOSXP  ",
	      formals.var,
	      body.var,
	      rho);
    del(formals);
    del(body);
    if (rho == "R_GlobalEnv") {
      return Expression(v, FALSE, INVISIBLE, "");
    } else {
      return Expression(v, TRUE, INVISIBLE, unp(v));
    }
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
    //    return output_to_expression(CodeGen::op_list(CScope(prefix + "_" + i_to_s(n)), e, rho, TRUE));
    return op_list(e, rho, true);
    break;
  case CHARSXP:
    v = appl1_unp("mkChar", quote(string(CHAR(e))));
    return Expression(v, TRUE, VISIBLE, "");
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
    err("Internal error: op_literal encountered bad type\n");
    return Expression::bogus_exp; // never reached
    break;
  }
}

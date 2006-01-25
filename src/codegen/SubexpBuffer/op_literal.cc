#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <support/RccError.h>
#include <ParseInfo.h>
#include <Visibility.h>
#include <CodeGen.h>

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
#ifdef USE_OUTPUT_CODEGEN
    return output_to_expression(CodeGen::op_vector(e));
#else
    return op_vector(e);
#endif
    break;
  case VECSXP:
    ParseInfo::flag_problem();
    return Expression("<<unimplemented vector>>", TRUE, INVISIBLE, "");
    break;
  case SYMSXP:
    return Expression(make_symbol(e), FALSE, VISIBLE, "");
    break;
  case LISTSXP:
  case LANGSXP:
#ifdef USE_OUTPUT_CODEGEN
    return output_to_expression(CodeGen::op_list(e, rho, true));
#else
    return op_list(e, rho, true, Protected);
#endif
    break;
  case CLOSXP:
#ifdef USE_OUTPUT_CODEGEN
    return output_to_expression(CodeGen::op_closure(e, rho));
#else
    return op_closure(e, rho, Protected);
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
  case CHARSXP:
    v = appl1("mkChar", quote(string(CHAR(e))), Unprotected);
    return Expression(v, TRUE, VISIBLE, "");
    break;
  case SPECIALSXP:
  case BUILTINSXP:
#ifdef USE_OUTPUT_CODEGEN
    return output_to_expression(CodeGen::op_primsxp(e, rho));
#else
    return ParseInfo::global_constants->op_primsxp(e, rho);
#endif
    break;
  case EXPRSXP:
  case EXTPTRSXP:
  case WEAKREFSXP:
    ParseInfo::flag_problem();
    return Expression("<<unimplemented type " + i_to_s(TYPEOF(e)) + ">>",
		      TRUE, INVISIBLE, "");
    break;
  default:
    rcc_error("Internal error: op_literal encountered bad type");
    return Expression::bogus_exp; // never reached
    break;
  }
}

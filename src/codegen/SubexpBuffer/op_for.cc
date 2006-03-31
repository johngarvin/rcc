#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>
#include <support/StringUtils.h>
#include <support/RccError.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>

#include <LoopContext.h>
#include <ParseInfo.h>
#include <Visibility.h>
#include <CodeGenUtils.h>

using namespace std;

//! Output a for loop
Expression SubexpBuffer::op_for(SEXP e, string rho,
				ResultStatus resultStatus) {
  SEXP sym, body, val;
  sym = CAR(for_iv_c(e));
  if ( !Rf_isSymbol(sym) ) rcc_error("non-symbol loop variable found in for loop");
  Expression range = op_exp(for_range_c(e), rho);
  decls += "int n;\n";
  decls += "int rangetype;\n";
  decls += "SEXP v;\n";
  decls += "PROTECT_INDEX vpi;\n";
  if (resultStatus == ResultNeeded) {
    decls += "SEXP ans;\n";
    decls += "PROTECT_INDEX api;\n";
  }
  LoopContext thisLoop;
  if (!has_i) {
    decls += "int i;\n";
    has_i = TRUE;
  }
  string defs;
  defs += "defineVar(" + make_symbol(sym) + ", R_NilValue, " + rho + ");\n";
  defs += "if (isList(" + range.var + ") || isNull(" + range.var + ")) {\n";
  defs += indent("n = length(" + range.var + ");\n");
  defs += indent("PROTECT_WITH_INDEX(v = R_NilValue, &vpi);\n");
  defs += "} else {\n";
  defs += indent("n = LENGTH(" + range.var + ");\n");
  defs += indent("PROTECT_WITH_INDEX(v = allocVector(TYPEOF(" + range.var + "), 1), &vpi);\n");
  defs += "}\n";
  if (resultStatus == ResultNeeded) {
    defs += "ans = R_NilValue;\n";
    defs += "PROTECT_WITH_INDEX(ans, &api);\n";
  }
  defs += "rangetype = TYPEOF(" + range.var + ");\n";
  defs += "for (i=0; i < n; i++) {\n";
  string in_loop;
  in_loop += "switch(rangetype) {\n";
  in_loop += "case LGLSXP:\n";
  in_loop += indent("REPROTECT(v = allocVector(LGLSXP, 1), vpi);\n");
  in_loop += indent("LOGICAL(v)[0] = LOGICAL(" + range.var + ")[i];\n");
  in_loop += indent("setVar(" + make_symbol(sym) + ", v, " + rho + ");\n");
  in_loop += indent("break;\n");
  in_loop += "case INTSXP:\n";
  in_loop += "REPROTECT(v = allocVector(INTSXP, 1), vpi);\n";
  in_loop += "INTEGER(v)[0] = INTEGER(" + range.var + ")[i];\n";
  in_loop += "setVar(" + make_symbol(sym) + ", v, " + rho + ");\n";
  in_loop += indent("break;\n");
  in_loop += "case REALSXP:\n";
  in_loop += "REPROTECT(v = allocVector(REALSXP, 1), vpi);\n";
  in_loop += "REAL(v)[0] = REAL(" + range.var + ")[i];\n";
  in_loop += "setVar(" + make_symbol(sym) + ", v, " + rho + ");\n";
  in_loop += indent("break;\n");
  in_loop += "case CPLXSXP:\n";
  in_loop += "REPROTECT(v = allocVector(CPLXSXP, 1), vpi);\n";
  in_loop += "COMPLEX(v)[0] = COMPLEX(" + range.var + ")[i];\n";
  in_loop += "setVar(" + make_symbol(sym) + ", v, " + rho + ");\n";
  in_loop += indent("break;\n");
  in_loop += "case STRSXP:\n";
  in_loop += "REPROTECT(v = allocVector(STRSXP, 1), vpi);\n";
  in_loop += "SET_STRING_ELT(v, 0, STRING_ELT(" + range.var + ", i));\n";
  in_loop += "setVar(" + make_symbol(sym) + ", v, " + rho + ");\n";
  in_loop += indent("break;\n");
  in_loop += "case EXPRSXP:\n";
  in_loop += "case VECSXP:\n";
  in_loop += "setVar(" + make_symbol(sym) + ", VECTOR_ELT(" + range.var + ", i), " + rho + ");\n";
  in_loop += indent("break;\n");
  in_loop += "case LISTSXP:\n";
  in_loop += "setVar(" + make_symbol(sym) + ", CAR(" + range.var + "), " + rho + ");\n";
  in_loop += range.var + " = CDR(" + range.var + ");\n";
  in_loop += indent("break;\n");
  in_loop += "default: errorcall(R_NilValue, \"Bad for loop sequence\");\n";
  in_loop += "}\n";
  defs += indent(in_loop);
  append_defs(defs);
  Expression ans = op_exp(for_body_c(e), rho, Unprotected, false, resultStatus);
  string cleanup;
  if (resultStatus == ResultNeeded) {
    append_defs("REPROTECT(ans = " + ans.var + ", api);\n");
  }
  append_defs("}\n");
  append_defs(thisLoop.breakLabel() + ":;\n");
  if (resultStatus == ResultNeeded) {
    append_defs(emit_unprotect("ans"));
  }
  append_defs("UNPROTECT_PTR(v);\n");
  del(range);
  return Expression(ans.var, TRUE, INVISIBLE, "");
}

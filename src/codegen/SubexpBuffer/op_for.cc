#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>
#include <support/StringUtils.h>
#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>

#include <LoopContext.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

//! Output a for loop
Expression SubexpBuffer::op_for(SEXP e, string rho) {
  SEXP sym, body, val;
  sym = CAR(e);
  val = CADR(e);
  body = CADDR(e);
  if ( !Rf_isSymbol(sym) ) err("non-symbol loop variable\n");
  Expression val1 = op_exp(val, rho);
  decls += "int n;\n";
  decls += "SEXP ans, v;\n";
  decls += "PROTECT_INDEX vpi, api;\n";
  LoopContext thisLoop;
  if (!has_i) {
    decls += "int i;\n";
    has_i = TRUE;
  }
  string defs;
  defs += "defineVar(" + make_symbol(sym) + ", R_NilValue, " + rho + ");\n";
  defs += "if (isList(" + val1.var + ") || isNull(" + val1.var + ")) {\n";
  defs += indent("n = length(" + val1.var + ");\n");
  defs += indent("PROTECT_WITH_INDEX(v = R_NilValue, &vpi);\n");
  defs += "} else {\n";
  defs += indent("n = LENGTH(" + val1.var + ");\n");
  defs += indent("PROTECT_WITH_INDEX(v = allocVector(TYPEOF(" + val1.var + "), 1), &vpi);\n");
  defs += "}\n";
  defs += "ans = R_NilValue;\n";
  defs += "PROTECT_WITH_INDEX(ans, &api);\n";
  defs += "for (i=0; i < n; i++) {\n";
  string in_loop;
  in_loop += "switch(TYPEOF(" + val1.var + ")) {\n";
  in_loop += "case LGLSXP:\n";
  in_loop += indent("REPROTECT(v = allocVector(TYPEOF(" + val1.var + "), 1), vpi);\n");
  in_loop += indent("LOGICAL(v)[0] = LOGICAL(" + val1.var + ")[i];\n");
  in_loop += indent("setVar(" + make_symbol(sym) + ", v, " + rho + ");\n");
  in_loop += indent("break;\n");
  in_loop += "case INTSXP:\n";
  in_loop += "REPROTECT(v = allocVector(TYPEOF(" + val1.var + "), 1), vpi);\n";
  in_loop += "INTEGER(v)[0] = INTEGER(" + val1.var + ")[i];\n";
  in_loop += "setVar(" + make_symbol(sym) + ", v, " + rho + ");\n";
  in_loop += indent("break;\n");
  in_loop += "case REALSXP:\n";
  in_loop += "REPROTECT(v = allocVector(TYPEOF(" + val1.var + "), 1), vpi);\n";
  in_loop += "REAL(v)[0] = REAL(" + val1.var + ")[i];\n";
  in_loop += "setVar(" + make_symbol(sym) + ", v, " + rho + ");\n";
  in_loop += indent("break;\n");
  in_loop += "case CPLXSXP:\n";
  in_loop += "REPROTECT(v = allocVector(TYPEOF(" + val1.var + "), 1), vpi);\n";
  in_loop += "COMPLEX(v)[0] = COMPLEX(" + val1.var + ")[i];\n";
  in_loop += "setVar(" + make_symbol(sym) + ", v, " + rho + ");\n";
  in_loop += indent("break;\n");
  in_loop += "case STRSXP:\n";
  in_loop += "REPROTECT(v = allocVector(TYPEOF(" + val1.var + "), 1), vpi);\n";
  in_loop += "SET_STRING_ELT(v, 0, STRING_ELT(" + val1.var + ", i));\n";
  in_loop += "setVar(" + make_symbol(sym) + ", v, " + rho + ");\n";
  in_loop += indent("break;\n");
  in_loop += "case EXPRSXP:\n";
  in_loop += "case VECSXP:\n";
  in_loop += "setVar(" + make_symbol(sym) + ", VECTOR_ELT(" + val1.var + ", i), " + rho + ");\n";
  in_loop += indent("break;\n");
  in_loop += "case LISTSXP:\n";
  in_loop += "setVar(" + make_symbol(sym) + ", CAR(" + val1.var + "), " + rho + ");\n";
  in_loop += val1.var + " = CDR(" + val1.var + ");\n";
  in_loop += indent("break;\n");
  in_loop += "default: errorcall(R_NilValue, \"Bad for loop sequence\");\n";
  in_loop += "}\n";
  defs += indent(in_loop);
  append_defs(defs);
  Expression ans = op_exp(body, rho);
  append_defs("REPROTECT(ans = " + ans.var + ", api);\n");
  del(ans);
  append_defs("}\n");
  append_defs(thisLoop.breakLabel() + ":;\n");
  del(val1);
  append_defs("UNPROTECT_PTR(v);\n");
  return Expression(ans.var, TRUE, INVISIBLE, unp(ans.var));
}

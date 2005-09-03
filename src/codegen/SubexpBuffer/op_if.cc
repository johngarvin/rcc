#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_if(SEXP e, string rho) {
  if (Rf_length(e) > 2) {                            // both true and false clauses present
#if 1
    Expression cond = op_exp(CAR(e), rho);
    string out = new_sexp();
    append_defs("if (my_asLogicalNoNA(" + cond.var + ")) {\n");
    del(cond);
    Expression te = op_exp(CADR(e), rho);
    append_defs(indent("PROTECT(" + out + " = " + te.var + ");\n"));
    del(te);
    append_defs(indent(Visibility::emit_set(te.is_visible)));
    append_defs("} else {\n");
    del(cond);
    Expression fe = op_exp(CADDR(e), rho);
    append_defs(indent("PROTECT(" + out + " = " + fe.var + ");\n"));
    del(fe);
    append_defs(indent(Visibility::emit_set(fe.is_visible)));
    append_defs("}\n");
    //del(cond);
    return Expression(out, FALSE, CHECK_VISIBLE, unp(out));
#else
    // Macro version. Waiting on better code generation.
    Expression cond = op_exp(CAR(e), rho);
    Expression te = op_exp(CADR(e), rho);
    Expression fe = op_exp(CADDR(e), rho);
    string out = new_sexp();
    append_defs(mac_ifelse.call(6,
			    cond.var,
			    out,
			    te.text,
			    te.var,
			    fe.text,
			    fe.var));
#endif
  } else {                                        // just the one clause, no else
    Expression cond = op_exp(CAR(e), rho);
    string out = new_sexp();
    append_defs("if (my_asLogicalNoNA(" + cond.var + ")) {\n");
    del(cond);
    Expression te = op_exp(CADR(e), rho);
    append_defs(indent("PROTECT(" + out + " = " + te.var + ");\n"));
    del(te);
    append_defs(indent(Visibility::emit_set(te.is_visible)));
    append_defs("} else {\n");
    del(cond);
    append_defs(indent(Visibility::emit_set(INVISIBLE)));
    append_defs(indent("PROTECT(" + out + " = R_NilValue);\n") + "}\n");
    return Expression(out, FALSE, CHECK_VISIBLE, unp(out));
  }
}

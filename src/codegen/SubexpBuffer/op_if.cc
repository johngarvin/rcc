#include <assert.h>

#include <string>
#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>
#include <support/StringUtils.h>
#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_if(SEXP e, string rho, 
			       ResultStatus resultStatus) 
{
  if (Rf_length(e) == 4) {                            // both true and false clauses present
#if 1
    //----------------------------------------------------------
    // optimization note:
    //  my_asLogicalNoNA is safe to call with an unprotected argument.
    //  it does not allocate memory.
    //  14 September 2005 - John Mellor-Crummey
    //----------------------------------------------------------
    Expression cond = op_exp(if_cond_c(e), rho, Unprotected);

    string out;
    append_defs("if (my_asLogicalNoNA(" + cond.var + ")) {\n");
    del(cond);

    //----------------------------------------------------------
    // optimization note:
    //   leave the result unprotected since it will be protected as it is
    //   immediately assigned to a name that is then protected.
    //
    // 14 September 2005 - John Mellor-Crummey
    //----------------------------------------------------------
    Expression te = op_exp(if_truebody_c(e), rho, Unprotected, false, 
			   resultStatus);
    if (resultStatus == ResultNeeded) {
      out = new_sexp();
      append_defs(indent(out + " = " + te.var + ";\n"));
      append_defs(indent(Visibility::emit_set(te.is_visible)));
    }
    append_defs("} else {\n");
    del(cond);

    //----------------------------------------------------------
    // optimization note:
    //   leave the result unprotected since it will be protected as it is
    //   immediately assigned to a name that is then protected.
    //
    // 14 September 2005 - John Mellor-Crummey
    //----------------------------------------------------------
    Expression fe = op_exp(if_falsebody_c(e), rho, Unprotected, false,
			   resultStatus);
    if (resultStatus == ResultNeeded) {
      append_defs(indent(out + " = " + fe.var + ";\n"));

      append_defs(indent(Visibility::emit_set(fe.is_visible)));
    }
    append_defs("}\n");
    return Expression(out, FALSE, CHECK_VISIBLE, "");
#else
    // Macro version. Waiting on better code generation.
    Expression cond = op_exp(if_cond_c(e), rho);
    Expression te = op_exp(if_truebody_c(e), rho);
    Expression fe = op_exp(if_falsebody_c(e), rho);
    string out = new_sexp();
    append_defs(mac_ifelse.call(6,
			    cond.var,
			    out,
			    te.text,
			    te.var,
			    fe.text,
			    fe.var));
#endif
  } else if (Rf_length(e) == 3) {         // just the one clause, no else
    Expression cond = op_exp(if_cond_c(e), rho, Unprotected);
    string out;
    assert(cond.del_text.empty());
    append_defs("if (my_asLogicalNoNA(" + cond.var + ")) {\n");
    del(cond);
    Expression te = op_exp(if_truebody_c(e), rho, Unprotected, false,
			   resultStatus);
    if (resultStatus == ResultNeeded) {
      out = new_sexp();
      append_defs(indent(out + " = " + te.var +";\n"));
      del(te);
      append_defs(indent(Visibility::emit_set(te.is_visible)));
      append_defs("} else {\n");
      del(cond);
      append_defs(indent(Visibility::emit_set(INVISIBLE)));
      append_defs(indent(out + " = R_NilValue;\n"));
    }
    append_defs("}\n");
    return Expression(out, FALSE, CHECK_VISIBLE, "");
  } else {
    err("Badly formed if expression\n");
    return Expression::bogus_exp;
  }
}

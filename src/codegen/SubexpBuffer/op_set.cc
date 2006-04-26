#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_Defn.h>
#include <support/StringUtils.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>

#include <CodeGen.h>
#include <CodeGenUtils.h>
#include <GetName.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

/// Output an assignment statement
Expression SubexpBuffer::op_set(SEXP e, SEXP op, string rho, 
				Protection resultProtection)
{
  string out;
  assert(is_assign_prim(op));
  SEXP lhs = CAR(assign_lhs_c(e));
  if (is_string(lhs)) {
    SETCAR(CDR(e), Rf_install(CHAR(STRING_ELT(lhs, 0))));
    lhs = CAR(assign_lhs_c(e));
  }
  if (is_var(lhs)) {
    string name = make_symbol(lhs);
    SEXP rhs_c = assign_rhs_c(e);
    SEXP rhs = CAR(assign_rhs_c(e));
    Expression body;
    // if body is a function definition, give op_fundef
    // the R name so that the f-function can be called directly later on
    if (is_fundef(rhs)) {
      body = op_fundef(rhs, rho, resultProtection, var_name(lhs));
    } else {
      // Right side of an assignment is always evaluated.
      // Pass 'true' to tell op_exp to evaluate the result
      body = op_exp(rhs_c, rho, Protected, true);
    }

    // whether the assignment is local or free
    // determines which environment we should use
    string target_env;
    if (is_local_assign_prim(op) || rho == "R_GlobalEnv") {
      target_env = rho;
    } else {
      target_env = emit_call1("ENCLOS", rho);
    }

    Expression out = op_var_def(assign_lhs_c(e), body.var, target_env);
    if (!body.del_text.empty())
      append_defs("UNPROTECT(1);\n");
    return out;
  } else if (is_simple_subscript(lhs)) {
    return op_subscriptset(e, rho, resultProtection);
  } else if (Rf_isLanguage(lhs)) {
#ifdef USE_OUTPUT_CODEGEN
    Expression func = output_to_expression(CodeGen::op_primsxp(op, rho));
    Expression args = output_to_expression(CodeGen::op_list(CDR(e), rho, true, Protected));
#else
    Expression func = ParseInfo::global_constants->op_primsxp(op, rho);
    Expression args = op_list(CDR(e), rho, true, Protected);
#endif
    out = appl4("do_set",
		"R_NilValue",
		func.var,
		args.var,
		rho, resultProtection);
    string cleanup;
    if (resultProtection == Protected) cleanup = unp(out);
    del(func);
    del(args);
    return Expression(out, true, INVISIBLE, cleanup);
  } else {
    ParseInfo::flag_problem();
    return Expression("<<assignment with unrecognized LHS>>",
		      TRUE, INVISIBLE, "");
  }
}

#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_Defn.h>
#include <support/StringUtils.h>
#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>

#include <CodeGen.h>
#include <GetName.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

//! Output an assignment statement
Expression SubexpBuffer::op_set(SEXP e, SEXP op, string rho) {
  string out;
  if (PRIMVAL(op) == 1 || PRIMVAL(op) == 3) {    //    <-, =
    SEXP lhs = CAR(assign_lhs_c(e));
    if (is_string(lhs)) {
      SETCAR(CDR(e), Rf_install(CHAR(STRING_ELT(lhs, 0))));
    }
    if (is_var(lhs)) {
      string name = make_symbol(lhs);
      SEXP rhs_c = assign_rhs_c(e);
      SEXP rhs = CAR(assign_rhs_c(e));
      Expression body;
      // if body is a function definition, give op_fundef
      // the R name so that the f-function can be called directly later on
      if (is_fundef(rhs)) {
	body = op_fundef(rhs, rho, var_name(lhs));
      } else {
	body = op_exp(rhs_c, rho);
      }
      append_defs("defineVar(" + name + ", " 
	+ body.var + ", " + rho + ");\n");
      del(body);
      return Expression(name, TRUE, INVISIBLE, "");
    } else if (is_simple_subscript(lhs)) {
      return op_subscriptset(e, rho);
    } else if (Rf_isLanguage(lhs)) {
      Expression func = output_to_expression(CodeGen::op_primsxp(op, rho));
      Expression args = op_list(CDR(e), rho, true);
      out = appl4("do_set",
		  "R_NilValue",
		  func.var,
		  args.var,
		  rho);
      del(func);
      del(args);
      return Expression(out, TRUE, INVISIBLE, unp(out));
    } else {
      ParseInfo::flag_problem();
      return Expression("<<assignment with unrecognized LHS>>",
			TRUE, INVISIBLE, "");
    }
  } else if (PRIMVAL(op) == 2) {  //     <<-
    Expression op1 = output_to_expression(CodeGen::op_primsxp(op, rho));
    Expression args1 = output_to_expression(CodeGen::op_list(CDR(e), rho, TRUE));
    out = appl4(get_name(PRIMOFFSET(op)),
		"R_NilValue",
		op1.var,
		args1.var,
		rho);
    del(op1);
    del(args1);
    return Expression(out, TRUE, INVISIBLE, unp(out));
  } else {
    ParseInfo::flag_problem();
    return Expression("<<Assignment of a type not yet implemented>>",
		      TRUE, INVISIBLE, "");
  }
}


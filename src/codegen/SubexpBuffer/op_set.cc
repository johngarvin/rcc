#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_Defn.h>
#include <support/StringUtils.h>
#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <GetName.h>
#include <ParseInfo.h>
#include <Visibility.h>

bool isSimpleSubscript(SEXP e);

using namespace std;

//! Output an assignment statement
Expression SubexpBuffer::op_set(SEXP e, SEXP op, string rho) {
  string out;
  if (PRIMVAL(op) == 1 || PRIMVAL(op) == 3) {    //    <-, =
    SEXP lhs = CADR(e);
    if (Rf_isString(lhs)) {
      SETCAR(CDR(e), Rf_install(CHAR(STRING_ELT(lhs, 0))));
    }
    if (Rf_isSymbol(lhs)) {
      string name = make_symbol(lhs);
      SEXP rhs = CADDR(e);
      Expression body;
      // if body is a function definition, give op_fundef
      // the R name so that the f-function can be called directly later on
      if (TYPEOF(rhs) == LANGSXP
	  && TYPEOF(CAR(rhs)) == SYMSXP
	  && CAR(rhs) == Rf_install("function")) {
	body = op_fundef(rhs, rho, CHAR(PRINTNAME(lhs)));
      } else {
	body = op_exp(rhs, rho);
      }
      append_defs("defineVar(" + name + ", " 
	+ body.var + ", " + rho + ");\n");
      del(body);
      return Expression(name, TRUE, INVISIBLE, "");
    } else if (isSimpleSubscript(lhs )) {
      return op_subscriptset(e, rho);
    } else if (Rf_isLanguage(lhs)) {
      Expression func = op_exp(op, rho);
      Expression args = op_list_local(CDR(e), rho);
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
    Expression op1 = op_exp(op, rho);
    //Expression args1 = output_to_expression(CodeGen::op_list(CScope(prefix + "_" + i_to_s(n)), CDR(e), rho, TRUE));
    Expression args1 = op_list(CDR(e), rho, true);
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



bool isSimpleSubscript(SEXP e) {
  return (TYPEOF(e) == LANGSXP
	  && CAR(e) == Rf_install("[")
	  && TYPEOF(CADR(e)) == SYMSXP);
}

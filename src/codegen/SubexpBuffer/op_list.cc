#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <CodeGenUtils.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_list(SEXP e, string rho, bool literal, 
				 bool fullyEvaluatedResult /* = FALSE */ ) {
  int i, len;
  len = Rf_length(e);
  string my_cons;
  if (len == 1) {
    switch (TYPEOF(e)) {
    case NILSXP:
      return Expression::nil_exp;
    case LISTSXP:
      my_cons = "cons";
      break;
    case LANGSXP:
      my_cons = "lcons";
      break;
    default:
      err("Internal error: bad call to op_list\n");
      return Expression::bogus_exp;  // never reached
    }
    Expression car = (literal ? op_literal(CAR(e), rho) : op_exp(e, rho, fullyEvaluatedResult));
    string out;
    if (car.is_dep) {
      if (TAG(e) == R_NilValue) {
	out = appl2(my_cons, car.var, "R_NilValue");
	del(car);
      } else {
	Expression tag = op_literal(TAG(e), rho);
	out = appl3("tagged_cons", car.var, tag.var, "R_NilValue");
	del(car);
	del(tag);
      }
      return Expression(out, TRUE, VISIBLE, unp(out));
    } else {  // not dep
      if (TAG(e) == R_NilValue) {
	out = ParseInfo::global_constants->appl2(my_cons, car.var, "R_NilValue");
	ParseInfo::global_constants->del(car);
      } else {
	Expression tag = ParseInfo::global_constants->op_literal(TAG(e), rho);
	out = ParseInfo::global_constants->appl3("tagged_cons", car.var, tag.var, "R_NilValue");
	ParseInfo::global_constants->del(car);
	ParseInfo::global_constants->del(tag);
      }
      return Expression(out, FALSE, VISIBLE, "");
    }
  } else {  // length >= 2
    string unp_cars = "";
    bool list_dep = FALSE;
    Expression *exps = new Expression[len];
    Expression *tags = new Expression[len];
    bool *langs = new bool[len];
    SEXP tmp_e = e;
    for(i=0; i<len; i++) {
      switch (TYPEOF(tmp_e)) {
      case LISTSXP:
	langs[i] = FALSE;
	break;
      case LANGSXP:
	langs[i] = TRUE;
	break;
      default:
	err("Internal error: bad call to op_list\n");
	return Expression::bogus_exp;  // never reached
      }
      exps[i] = (literal? op_literal(CAR(tmp_e), rho) : op_exp(tmp_e, rho, 
							       fullyEvaluatedResult));
      tags[i] = (TAG(tmp_e) == R_NilValue ? Expression("", FALSE, INVISIBLE, "") : op_literal(TAG(tmp_e), rho));
      if (exps[i].is_dep) list_dep = TRUE;
      tmp_e = CDR(tmp_e);
    }
    SubexpBuffer tmp_buf = new_sb("tmp_list");
    string cdr = "R_NilValue";
    for(i=len-1; i>=0; i--) {
      my_cons = (langs[i] ? "lcons" : "cons");
      if (tags[i].var.empty()) {
	cdr = tmp_buf.appl2_unp(my_cons, exps[i].var, cdr);
      } else {
	cdr = tmp_buf.appl3_unp("tagged_cons", exps[i].var, tags[i].var, cdr);
      }
      unp_cars += exps[i].del_text;
    }
    delete [] exps;
    delete [] tags;
    delete [] langs;
    if (list_dep) {
      string handle = new_sexp();
      string defs;
      defs += tmp_buf.output();
      defs += "PROTECT(" + handle + " = " + cdr + ");\n";
      defs += unp_cars;
      append_defs(emit_in_braces(defs));
      return Expression(handle, list_dep, VISIBLE, unp(handle));
    } else {
      string handle = ParseInfo::global_constants->new_sexp();
      string defs;
      defs += tmp_buf.output();
      defs += "PROTECT(" + handle + " = " + cdr + ");\n";
      defs += unp_cars;
      ParseInfo::global_constants->append_defs(emit_in_braces(defs));
      return Expression(handle, list_dep, VISIBLE, "");
    }
  }
}

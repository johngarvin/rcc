#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>
#include <support/StringUtils.h>
#include <CodeGen.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_closure(SEXP e, string rho, Protection resultProtection) {
  Expression formals = op_list(FORMALS(e), rho, true, Protected);
  Expression body = op_literal(BODY(e), rho);
  if (rho == "R_GlobalEnv" && !formals.is_dep && !body.is_dep) {
    string v = ParseInfo::global_constants->appl3("mkCLOSXP",
						  formals.var,
						  body.var,
						  rho,
						  resultProtection);
    ParseInfo::global_constants->del(formals);
    ParseInfo::global_constants->del(body);
    return Expression(v, false, INVISIBLE, "");
  } else {
    string v = appl3("mkCLOSXP",
		     formals.var,
		     body.var,
		     rho,
		     resultProtection);
    del(formals);
    del(body);
    string del_text = (resultProtection == Protected ? unp(v) : "");
    return Expression(v, true, INVISIBLE, del_text);
  }
}

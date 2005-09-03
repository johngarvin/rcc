#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>
#include <support/StringUtils.h>
#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>

#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;
using namespace RAnnot;

Expression SubexpBuffer::op_return(SEXP e, string rho) {
  FuncInfo *fi = 
    lexicalContext.IsEmpty() ? NULL : lexicalContext.Top();

  if (fi && fi->getRequiresContext()) {
    append_defs("endcontext(&context);\n");
  }

  Expression value;
  string v;
  switch(Rf_length(e)) {
  case 0:
    append_defs("UNPROTECT_PTR(newenv);\n");
    append_defs("return R_NilValue;\n");
    break;
  case 1:
    value = op_exp(CAR(e), rho);
    append_defs("PROTECT(" + value.var + ");\n");
    v = appl2_unp("eval", value.var, rho);
    append_defs("UNPROTECT_PTR(" + value.var + ");\n");
    append_defs("UNPROTECT_PTR(newenv);\n");
    del(value);
    append_defs("return " + v + ";\n");
    break;
  default:
    SEXP exp = e;
    // set names for multiple return
    while(exp != R_NilValue) {
      if (TYPEOF(CAR(exp)) == SYMSXP) {
	SET_TAG(exp, CAR(exp));
      }
      exp = CDR(exp);
    }
    //    value = output_to_expression(CodeGen::op_list(CScope(prefix + "_" + i_to_s(n)), e, rho, false));
    value = op_list(e, rho, false);
    string retval = appl1_unp("PairToVectorList", value.var);
    append_defs("UNPROTECT_PTR(newenv);\n");
    del(value);
    append_defs("return " + retval + ";\n");
    break;
  }
  return Expression("R_NilValue", TRUE, INVISIBLE, "");
}

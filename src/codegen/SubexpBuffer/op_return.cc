#include <string>

#include <CheckProtect.h>
#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>
#include <analysis/AnalysisResults.h>

#include <ParseInfo.h>
#include <Visibility.h>
#include <CodeGen.h>

using namespace std;
using namespace RAnnot;

Expression SubexpBuffer::op_return(SEXP e, string rho) {
  FuncInfo *fi = 
    lexicalContext.IsEmpty() ? NULL : lexicalContext.Top();


  Expression value;
  string v;
  switch(Rf_length(e)) {
  case 0:
    v = "R_NilValue";
    break;
  case 1:
    // pass true as fourth argument to yield fully evaluated result
    value = op_exp(e, rho, Unprotected, true);
#if 0
    if (value.is_dep) {
      append_defs("PROTECT(" + value.var + ");\n");
      v = appl2("eval", value.var, rho, Unprotected);
      append_defs("UNPROTECT_PTR(" + value.var + ");\n");
    } else {
      v = value.var;
    }
#else
    v = value.var;
#endif
    del(value);
    break;
  default:
    SEXP exp = e;
    // set names for multiple return
    while(exp != R_NilValue) {
      SEXP tag = TAG(exp);
      if (tag == R_NilValue && TYPEOF(CAR(exp)) == SYMSXP) {
	SET_TAG(exp, CAR(exp));
      }
      exp = CDR(exp);
    }
#ifdef USE_OUTPUT_CODEGEN
    value = output_to_expression(CodeGen::op_list(e, rho, false, true));
#else
    value = op_list(e, rho, false, Protected, true);
#endif
    v = appl1("PairToVectorList", value.var, Unprotected);
    del(value);
    break;
  }

  //---------------------------
  // tear down context, if any
  //---------------------------
  if (fi && fi->getRequiresContext())  
    append_defs("endcontext(&context);\n"); 

  //---------------------------
  // unprotect newenv
  //---------------------------
  append_defs("UNPROTECT(1); /* newenv */\n");
#ifdef CHECK_PROTECT
  append_defs("assert(topval == R_PPStackTop);\n");
#endif

  //---------------------------
  // return v
  //---------------------------
  append_defs("return " + v + ";\n");

  return Expression("R_NilValue", TRUE, INVISIBLE, "");
}

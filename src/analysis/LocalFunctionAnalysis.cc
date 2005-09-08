#include <analysis/Utils.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Annotation.h>
#include "LocalFunctionAnalysis.h"

using namespace RAnnot;

//! Get local information about a function. Adds a variable annotation
//! for each argument and updates the function annotation with
//! local information about its arguments.
void LocalFunctionAnalysis::perform_analysis(const SEXP fundef) {
  assert(is_fundef(fundef));
  SEXP args = CAR(fundef_args_c(fundef));
  const SEXP ddd = Rf_install("...");
  bool has_var_args = false;
  int n_args = 0;
  for(SEXP e = args; e != R_NilValue; e = CDR(e)) {
    ++n_args;
    DefVar * annot = new DefVar();
    annot->setMention(e);
    annot->setSourceType(DefVar::DefVar_FORMAL);
    annot->setMayMustType(Var::Var_MUST);
    annot->setLocalityType(Locality_LOCAL);
    putProperty(Var, e, annot, true);
    if (TAG(e) == ddd) {
      has_var_args = true;
    }
  }
  FuncInfo * f_annot = getProperty(FuncInfo, fundef);
  assert(f_annot != 0);
  f_annot->setNumArgs(n_args);
  f_annot->setHasVarArgs(has_var_args);
}

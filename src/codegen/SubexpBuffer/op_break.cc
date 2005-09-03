#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>
#include <support/StringUtils.h>
#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>

#include <LoopContext.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_break(SEXP e, string rho) {
  LoopContext *loop = LoopContext::Top(); 
  if (Rf_install("next") == e)
    append_defs("continue;\n");
  else
    append_defs(loop->doBreak() + ";\n");
  return Expression("R_NilValue", TRUE, INVISIBLE, "");
}

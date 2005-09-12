#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>

#include <support/StringUtils.h>

#include <CodeGen.h>
#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_symlist(SEXP e, string rho) {
  return output_to_expression(CodeGen::op_list(CScope(prefix + "_" + i_to_s(n)), e, rho, true));
  //return op_list(e, rho, true);
}

#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>

#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_promise(SEXP e) {
  Expression args; 
  string defs = args.var + " = forcePromise(" + args.var + ");\n" + "SET_NAMED(" + args.var + ", 2);\n";
  append_defs(defs);
  return Expression(args.var, TRUE, INVISIBLE, "");
}


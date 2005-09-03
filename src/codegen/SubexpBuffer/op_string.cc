#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_string(SEXP s) {
  int i, len;
  string str = "";
  len = Rf_length(s);
  for(i=0; i<len; i++) {
    str += string(CHAR(STRING_ELT(s, i)));
  }
  string out = ParseInfo::global_constants->appl1("mkString", 
				      quote(escape(str)));
  return Expression(out, FALSE, VISIBLE, "");
}

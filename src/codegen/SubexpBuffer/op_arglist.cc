#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>

#include <support/StringUtils.h>

#include <Visibility.h>

using namespace std;

//! Output the argument list for an external function (generally a list
//! where the CARs are R_MissingArg and the TAGs are SYMSXPs
//! representing the formal arguments)
Expression SubexpBuffer::op_arglist(SEXP e, string rho) {
  int i;
  string out, tmp, tmp1;
  SEXP arg;
  int len = Rf_length(e);
  if (len == 0) return Expression::nil_exp;
  SEXP *args = new SEXP[len];
  SubexpBuffer buf = new_sb("tmp_arglist");
    
  arg = e;
  for(i=0; i<len; i++) {
    args[i] = arg;
    arg = CDR(arg);
  }
    
  // Don't unprotect R_NilValue, just the conses
  tmp = buf.appl2("cons", make_symbol(TAG(args[len-1])), "R_NilValue");
  if (len > 1) {
    for(i=len-2; i>=0; i--) {
      string sym = make_symbol(TAG(args[i]));
      tmp1 = buf.appl2("cons", sym, tmp);
      buf.append_defs(unp(tmp));
      tmp = tmp1;
    }
  }
  delete [] args;
  out = new_sexp();
  string defs;
  defs += "{\n";
  defs += indent(buf.output());
  defs += indent(out + " = " + tmp + ";\n");
  defs += "}\n";
  append_defs(defs);
  return Expression(out, TRUE, INVISIBLE, unp(out));
}

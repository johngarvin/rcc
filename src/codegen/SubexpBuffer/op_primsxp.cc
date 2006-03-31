#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_Defn.h>

#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <support/RccError.h>
#include <Macro.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_primsxp(SEXP e, string rho) {
  string var;
  int is_builtin;
  switch (TYPEOF(e)) {
  case SPECIALSXP:
    is_builtin = 0;
    break;
  case BUILTINSXP:
    is_builtin = 1;
    break;
  default:
    rcc_error("Internal error: op_primsxp called on non-(special or builtin)");
    is_builtin = 0; // silence the -Wall Who Cried "Uninitialized variable."
  }
  
  // unique combination of offset and is_builtin for memoization
  int value = 2 * PRIMOFFSET(e) + is_builtin;
  map<int,string>::iterator pr = ParseInfo::primsxp_map.find(value);
  if (pr != ParseInfo::primsxp_map.end()) {  // primsxp already defined
    return Expression(pr->second, TRUE, INVISIBLE, "");
  } else {
    string var = new_sexp();
    const string args[] = {var, i_to_s(PRIMOFFSET(e)),
			   i_to_s(is_builtin), string(PRIMNAME(e))};
    append_defs(mac_primsxp.call(4, args));
    ParseInfo::primsxp_map.insert(pair<int,string>(value, var));
    return Expression(var, TRUE, INVISIBLE, "");
  }
}

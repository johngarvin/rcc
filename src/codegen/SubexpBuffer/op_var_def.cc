#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_Defn.h>
#include <support/StringUtils.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>
#include <analysis/VarBinding.h>

#include <CodeGen.h>
#include <CodeGenUtils.h>
#include <GetName.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;
using namespace RAnnot;

// forward declaration for file-static function

static Expression op_caching_lookup(SubexpBuffer * sb, string name, string symbol, string rhs, string rho);

// interface function

Expression SubexpBuffer::op_var_def(SEXP cell, string rhs, string rho) {
  string name = CHAR(PRINTNAME(CAR(cell)));
  string symbol = make_symbol(CAR(cell));
  VarBinding * annot = getProperty(VarBinding, cell);
  if (annot->is_single()) {
    string location = annot->get_location(CAR(cell), this);
    append_defs(emit_assign(location, emit_call3("defineVarReturnLoc", symbol, rhs, rho)));
    return Expression(rhs, true, INVISIBLE, "");
  } else {   // no unique location; emit lookup
    append_defs(emit_call3("defineVar", symbol, rhs, rho) + ";\n");
    return Expression(rhs, true, INVISIBLE, "");
  }
}

// TODO: if we can prove the variable is already declared by another def
//      append_defs(emit_call2("R_SetVarLocValue", location, rhs) + ";\n");

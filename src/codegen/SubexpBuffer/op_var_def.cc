#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_Defn.h>
#include <support/StringUtils.h>
#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>

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
  Var * annot = getProperty(Var, cell);
  if (annot->getScopeType() == Var::Var_GLOBAL) {
    map<string, string>::iterator loc;
    loc = ParseInfo::loc_map.find(name);
    if (loc == ParseInfo::loc_map.end()) {
      // not found; emit code to define name and cache location
      return op_caching_lookup(this, name, symbol, rhs, rho);
    } else {
      // location exists, having been set by a previous use or def.
      // TODO: do something different if there's a unique binding
      return op_caching_lookup(this, name, symbol, rhs, rho);
    }
  } else {   // not global scope
    append_defs(emit_call3("defineVar", symbol, rhs, rho) + ";\n");
    return Expression(rhs, true, INVISIBLE, "");
  }
}

// if unique binding:
//      append_defs(emit_call2("R_SetVarLocValue", loc->second, rhs) + ";\n");

// file static function

static Expression op_caching_lookup(SubexpBuffer * sb, string name, string symbol, string rhs, string rho) {
  string loc_h = sb->new_var_unp();
  string decl = "static R_varloc_t " + loc_h + ";\n";
  ParseInfo::global_constants->append_decls(decl);
  string call = emit_call3("defineVarReturnLoc", symbol, rhs, rho);
  sb->append_defs(emit_assign(loc_h, call));
  ParseInfo::loc_map.insert(pair<string,string>(name, loc_h));
  return Expression(rhs, true, INVISIBLE, "");
}

#include "CodeGen.h"

#include <string>
#include <map>

#include "Main.h"
#include <StringUtils.h>
#include "CodeGenUtils.h"

using namespace std;

Output CodeGen::op_vector(SEXP vec) {
  int len = Rf_length(vec);
  if (len != 1) {
    err("unexpected non-scalar vector encountered");
    return Output::bogus;
  }
  int value;
  switch(TYPEOF(vec)) {
  case LGLSXP:
    {
      int value = INTEGER(vec)[0];
      map<int,string>::iterator pr = ProgramInfo::sc_logical_map.find(value);
      if (pr == ProgramInfo::sc_logical_map.end()) {  // not found
	string var = ProgramInfo::global_constants.new_var();
	ProgramInfo::sc_logical_map.insert(pair<int,string>(value, var));
	Output op = Output::global(GDecls(emit_static_decl(var)),
				   GCode(emit_prot_assign(var, emit_call1("ScalarLogical", i_to_s(value)))),
				   Handle(var), VISIBLE);
	return op;
      } else {
	Output op(Decls(""), Code(""),
		  GDecls(""), GCode(""),
		  Handle(pr->second), DelText(""), CONST, VISIBLE);
	return op;
      }
    }
    break;
  case REALSXP:
    {
      double value = REAL(vec)[0];
      map<double,string>::iterator pr = ProgramInfo::sc_real_map.find(value);
      if (pr == ProgramInfo::sc_real_map.end()) {  // not found
	string var = ProgramInfo::global_constants.new_var();
	ProgramInfo::sc_real_map.insert(pair<double,string>(value, var));
	Output op(Decls(""), Code(""),
		  GDecls(emit_static_decl(var)),
		  GCode(emit_prot_assign(var, emit_call1("ScalarReal", d_to_s(value)))),
		  Handle(var), DelText(""), CONST, VISIBLE);
	return op;
      } else {
	Output op(Decls(""), Code(""),
		  GDecls(""), GCode(""),
		  Handle(pr->second), DelText(""), CONST, VISIBLE);
	return op;
      }
    }
    break;
  default:
    err("Unhandled type in op_vector");
    return Output::bogus;
  }
}

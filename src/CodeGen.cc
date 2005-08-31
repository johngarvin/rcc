#include "CodeGen.h"

#include <string>
#include <map>

#include <Main.h>
#include <support/StringUtils.h>
#include <CodeGenUtils.h>
#include <ParseInfo.h>

using namespace std;

Output CodeGen::op_exp(SEXP e, string rho, bool primFuncArg) {
  return Output::bogus;
}

Output CodeGen::op_literal(SEXP e, string rho) {
  return Output::bogus;
}

Output CodeGen::op_list(SubexpBuffer & buf,
			SEXP e,
			string rho,
			bool literal,
			bool promFuncArgList /* = false */) {
  int i, len;
  if (e == R_NilValue) return Output::nil;
  len = Rf_length(e);
  string my_cons;
  if (len == 1) {
    // one element
    switch (TYPEOF(e)) {
    case LISTSXP:
      my_cons = "cons";
      break;
    case LANGSXP:
      my_cons = "lcons";
      break;
    default:
      err("Internal error: non-list passed to op_list\n");
      return Output::bogus;
    }

    Output car = (literal ? op_literal(CAR(e), rho) : op_exp(CAR(e), rho, promFuncArgList));
    string code;
    string var = buf.new_var_unp();
    if (TAG(e) == R_NilValue) {
      // No tag; regular cons or lcons
      code = emit_prot_assign(var, emit_call2(my_cons, car.get_handle(), "R_NilValue"));
      if (car.get_dependence() == CONST) {
	return Output::global(GDecls(car.get_g_decls()),
			      GCode(car.get_g_code() + code),
			      Handle(var),
			      VISIBLE);
      } else {
	return Output::dependent(Decls(car.get_decls()),
				 Code(car.get_code() + code),
				 Handle(var),
				 DelText(car.get_del_text() + unp(var)),
				 VISIBLE);
      }
    } else {
      // Tag exists; need car, cdr, and tag
      Output tag = CodeGen::op_literal(TAG(e), rho);
      code = emit_prot_assign(var, emit_call3("tagged_cons", car.get_handle(), tag.get_handle(), "R_NilValue"));
      if (car.get_dependence() == CONST && tag.get_dependence() == CONST) {
	return Output::global(GDecls(car.get_g_decls() + tag.get_g_decls()),
			      GCode(car.get_g_code() + tag.get_g_code() + code),
			      Handle(var),
			      VISIBLE);
      } else {
	return Output::dependent(Decls(car.get_decls() + tag.get_decls()),
				 Code(car.get_code() + tag.get_code()),
				 Handle(var),
				 DelText(car.get_del_text() + tag.get_del_text()),
				 VISIBLE);
      }
    }
  } else {    // length >= 2
    /// HERE
  }
}

Output CodeGen::op_string(SEXP s) {
  //  TODO: use ParseInfo::string_map
  int i, len;
  string str = "";
  len = Rf_length(s);
  for(i=0; i<len; i++) {
    str += string(CHAR(STRING_ELT(s, i)));
  }
  string out = ParseInfo::global_constants->appl1("mkString", 
						   quote(escape(str)));
  return Output::text_const(Handle(out));
}

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
      map<int,string>::iterator pr = ParseInfo::sc_logical_map.find(value);
      if (pr == ParseInfo::sc_logical_map.end()) {  // not found
	string var = ParseInfo::global_constants->new_var();
	ParseInfo::sc_logical_map.insert(pair<int,string>(value, var));
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
      map<double,string>::iterator pr = ParseInfo::sc_real_map.find(value);
      if (pr == ParseInfo::sc_real_map.end()) {  // not found
	string var = ParseInfo::global_constants->new_var();
	ParseInfo::sc_real_map.insert(pair<double,string>(value, var));
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

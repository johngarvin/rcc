#include "CodeGen.h"

#include <string>
#include <map>

#include <include/R/R_Defn.h>

#include <Main.h>
#include <support/StringUtils.h>
#include <CodeGenUtils.h>
#include <ParseInfo.h>
#include <CScope.h>

using namespace std;

Output CodeGen::op_exp(SEXP e, string rho, bool promFuncArg) {
  err("CodeGen::op_exp not yet implemented\n");
  return Output::bogus;
}

Output CodeGen::op_primsxp(SEXP e, string rho) {
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
    err("Internal error: op_primsxp called on non-(special or builtin)");
    is_builtin = 0; // silence the -Wall Who Cried "Uninitialized variable."
  }
  
  // unique combination of offset and is_builtin for memoization
  int value = 2 * PRIMOFFSET(e) + is_builtin;
  map<int,string>::iterator pr = ParseInfo::primsxp_map.find(value);
  if (pr != ParseInfo::primsxp_map.end()) {  // primsxp already defined
    return Output::invisible_const(Handle(pr->second));
  } else {
    string var = ParseInfo::global_constants->new_var();
    const string args[] = {var,
			   i_to_s(PRIMOFFSET(e)),
			   i_to_s(is_builtin),
			   string(PRIMNAME(e))};
    string primsxp_def = mac_primsxp.call(4, args);
    ParseInfo::primsxp_map.insert(pair<int,string>(value, var));
    return Output::global(GDecls(emit_decl(var)),
			  GCode(primsxp_def),
			  Handle(var),
			  INVISIBLE);
  }
  
}

Output CodeGen::op_literal(CScope scope, SEXP e, string rho) {
#if 0
  err("CodeGen::op_literal not yet implemented\n");
  return Output::bogus;
#endif
#if 1
  string v;
  switch(TYPEOF(e)) {
  case NILSXP:
    return Output::nil;
    break;
  case STRSXP:
    return op_string(e);
    break;
  case LGLSXP: case REALSXP:
    return op_vector(e);
    break;
  case SYMSXP:
    return Output::visible_const(Handle(make_symbol(e)));
    break;
  case LISTSXP: case LANGSXP:
    return op_list(scope, e, rho, true);
    break;
  case CHARSXP:
    return Output::visible_const(Handle(emit_call1("mkChar", quote(string(CHAR(e))))));
    break;
  case SPECIALSXP: case BUILTINSXP:
    return op_primsxp(e, rho);
    break;
  default:
    ParseInfo::flag_problem();
    return Output::visible_const(Handle("<<unimplemented type " + i_to_s(TYPEOF(e)) + ">>"));
    break;
  }
#endif
}

Output CodeGen::op_list(CScope scope,
			SEXP e,
			string rho,
			bool literal,
			bool promFuncArgList /* = false */)
{
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

    Output car = (literal ? op_literal(scope, CAR(e), rho) : op_exp(CAR(e), rho, promFuncArgList));
    string code;
    string var = scope.new_label();
    if (TAG(e) == R_NilValue) {
      // No tag; regular cons or lcons
      code = emit_prot_assign(var, emit_call2(my_cons, car.get_handle(), "R_NilValue"));
      if (car.get_dependence() == CONST) {
	return Output::global(GDecls(car.get_g_decls() + emit_decl(var)),
			      GCode(car.get_g_code() + code),
			      Handle(var),
			      VISIBLE);
      } else {
	return Output::dependent(Decls(car.get_decls() + emit_decl(var)),
				 Code(car.get_code() + code),
				 Handle(var),
				 DelText(car.get_del_text() + unp(var)),
				 VISIBLE);
      }
    } else {
      // Tag exists; need car, cdr, and tag
      Output tag = op_literal(scope, TAG(e), rho);
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
    int types[len];
    bool langs[len];
    Output *cars = new Output[len];
    Output *tags = new Output[len];
    bool list_uses_tags = false;
    
    // collect Output from CAR and TAG of each item in the list
    SEXP tmp_e = e;
    for(i=0; i<len; i++) {
      assert(TYPEOF(tmp_e) == LANGSXP || TYPEOF(tmp_e) == LISTSXP);
      langs[i] = (TYPEOF(tmp_e) == LANGSXP);  // whether LANGSXP or LISTSXP
      cars[i] = (literal ? op_literal(scope, CAR(tmp_e), rho) : op_exp(CAR(tmp_e), rho, promFuncArgList));
      if (TAG(tmp_e) == R_NilValue) {
	tags[i] = Output::nil;
      } else {
	list_uses_tags = true;
	tags[i] = op_literal(scope, TAG(tmp_e), rho);
      }
      tmp_e = CDR(tmp_e);
    }

    // temporary scope for intermediate list elements
    CScope temp_list_elements("tmp_ls");

    // list is constant only if everything in it is constant
    DependenceType list_dependence = CONST;
    for(i=0; i<len; i++) {
      if (cars[i].get_dependence() == DEPENDENT
	  || tags[i].get_dependence() == DEPENDENT) {
	list_dependence = DEPENDENT;
      }
    }

    // collect code for outside objects
    string decls = "";
    string code = "";
    string g_decls = "";
    string g_code = "";
    string del_text = "";

    // collect code for list construction inside curly braces
    string buf_decls = "";
    string buf_code = "";

    // iterate backwards to build list
    string new_handle;
    string old_handle = "R_NilValue";
    for(i=len-1; i>=0; i--) {
      new_handle = temp_list_elements.new_label();
      string call;
      if (list_uses_tags) {
	call = emit_call3("tagged_cons", cars[i].get_handle(), tags[i].get_handle(), old_handle);
      } else {
	call = emit_call2((langs[i] ? "lcons" : "cons"), cars[i].get_handle(), old_handle);
      }
      string assign = emit_assign(new_handle, call);
      if (list_dependence == DEPENDENT) {
	decls    += cars[i].get_decls()    + tags[i].get_decls();
	code     += cars[i].get_code()     + tags[i].get_code();
	g_decls  += cars[i].get_g_decls()  + tags[i].get_g_decls();
	g_code   += cars[i].get_g_code()   + tags[i].get_g_code();
	del_text += cars[i].get_del_text() + tags[i].get_del_text();
      } else {
	// list is constant
	g_decls += cars[i].get_g_decls() + tags[i].get_g_decls();
	g_code  += cars[i].get_g_code()  + tags[i].get_g_code();
      }
      buf_decls += emit_decl(new_handle);
      buf_code += assign;
      old_handle = new_handle;
    }

    delete [] cars;
    delete [] tags;

    // connect list in buffer to other code
    if (list_dependence == DEPENDENT) {
      string outside_handle = scope.new_label();
      string out_assign = emit_prot_assign(outside_handle, new_handle);
      return Output(Decls(decls + emit_decl(outside_handle)),
		    Code(code
			 + emit_in_braces(buf_decls + buf_code + out_assign)),
		    GDecls(g_decls),
		    GCode(g_code),
		    Handle(outside_handle),
		    DelText(del_text + unp(outside_handle)),
		    DEPENDENT, VISIBLE);
    } else {                               // list is constant
      string outside_handle = scope.new_label();
      string out_assign = emit_prot_assign(outside_handle, new_handle);
      return Output::global(GDecls(g_decls + emit_decl(outside_handle)),
			    GCode(g_code
				  + emit_in_braces(buf_decls + buf_code + out_assign)),
			    Handle(outside_handle),
			    VISIBLE);
    }
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
  return Output::visible_const(Handle(out));
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


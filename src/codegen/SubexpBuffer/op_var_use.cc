#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_Defn.h>

#include <analysis/VarBinding.h>
#include <analysis/AnalysisResults.h>

#include <support/StringUtils.h>
#include <support/RccError.h>

#include <CodeGen.h>
#include <CodeGenUtils.h>
#include <GetName.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;
using namespace RAnnot;


// file-static forward declarations

typedef enum { PLAIN_VAR, FUNCTION_VAR } LookupType;

static Expression op_use(SubexpBuffer *sb, SEXP cell, string rho,
			 Protection resultProtection,
			 bool fullyEvaluatedResult, LookupType lookup_type);
static Expression op_lookup(SubexpBuffer * sb, string lookup_function,
			    string symbol, string rho,
			    Protection resultProtection,
			    bool fullyEvaluatedResult);
static Expression op_internal(SubexpBuffer * sb, SEXP e, SEXP env_val, string name,
			      string lookup_function, string rho);

// interface functions

Expression SubexpBuffer::op_var_use(SEXP cell, string rho,
				    Protection resultProtection,
				    bool fullyEvaluatedResult)
{
  return op_use(this, cell, rho, resultProtection, fullyEvaluatedResult, PLAIN_VAR);
}

Expression SubexpBuffer::op_fun_use(SEXP cell, string rho,
				    Protection resultProtection,
				    bool fullyEvaluatedResult)
{
  return op_use(this, cell, rho, resultProtection, fullyEvaluatedResult, FUNCTION_VAR);
}


// file-static function definitions

static Expression op_use(SubexpBuffer *sb, SEXP cell, string rho,
			 Protection resultProtection,
			 bool fullyEvaluatedResult, LookupType lookup_type)
{
  SEXP e = CAR(cell);
  string name = CHAR(PRINTNAME(e));
  string lookup_function = (lookup_type == FUNCTION_VAR ? "Rf_findFun" : "Rf_findVar");
  VarBinding * annot = getProperty(VarBinding, cell);

  if (annot->is_single()) {
    if (InternalLexicalScope * scope = dynamic_cast<InternalLexicalScope *>(*(annot->begin()))) {
      // check for global constants
      // binding_map is used for global variables with
      // constant values; it records the constant value of each name.
      map<string, string>::iterator value;
      value = ParseInfo::binding_map.find(name);
      if (value != ParseInfo::binding_map.end()) {       // in binding map
	return Expression(value->second, false, VISIBLE, "");
      } else {
	SEXP env_val;
	if (lookup_type == FUNCTION_VAR) {
	  env_val = Rf_findFunUnboundOK(e, R_GlobalEnv, TRUE);
	} else {
	  env_val = Rf_findVar(e, R_GlobalEnv);
	}
	assert(env_val != R_UnboundValue);
	return op_internal(sb, e, env_val, name, lookup_function, rho);
      }
    } else if (FundefLexicalScope * scope = dynamic_cast<FundefLexicalScope *>(*(annot->begin()))) {
      FuncInfo* fi = getProperty(FuncInfo, scope->get_fundef());
      if (fi->is_arg(e)) {
	// have to produce a lookup here; we don't know whether we need to evalaute a promise
	// TODO: handle this case
	return op_lookup(sb, lookup_function, make_symbol(e), rho,
			 resultProtection, fullyEvaluatedResult);
      } else {
	string location = annot->get_location(e, sb);
	string h = sb->appl1("R_GetVarLocValue", location, Unprotected);
	return Expression(h, true, VISIBLE, "");
      }
    } else {
      rcc_error("Unknown derived type of LexicalScope found");
    }
  } else {   // more than one possible scope
    return op_lookup(sb, lookup_function, make_symbol(e), rho,
		     resultProtection, fullyEvaluatedResult);
  }
}

/// output a lookup of a name in the given environment
static Expression op_lookup(SubexpBuffer * sb, string lookup_function,
			    string symbol, string rho,
			    Protection resultProtection, bool fullyEvaluatedResult)
{
  string value = sb->appl2(lookup_function, symbol, rho, Unprotected);
  string del_text;
  VisibilityType vis = VISIBLE;
  if (fullyEvaluatedResult) {
    value = sb->appl2("Rf_eval", value, rho, resultProtection);
    vis = CHECK_VISIBLE;
    if (resultProtection == Protected) del_text = unp(value);
  }
  return Expression(value, true, vis, del_text);
}


/// output the value of an internal R name
static Expression op_internal(SubexpBuffer * sb, SEXP e, SEXP env_val, string name,
			      string lookup_function, string rho)
{
  string h;
  SEXP sym_value = SYMVALUE(e);
  if (sym_value == R_UnboundValue) {
    // library function
    string call = emit_call2(lookup_function, make_symbol(e), "R_GlobalEnv");
    if (TYPEOF(env_val) == PROMSXP) {
      call = emit_call2("Rf_eval", call, "R_GlobalEnv");
    }
    h = ParseInfo::global_constants->new_sexp();
    string assign = emit_prot_assign(h, call);
    ParseInfo::global_constants->append_defs(assign);
  } else {
    // builtin/special function; symbol containts ptr to definition 
    if (TYPEOF(sym_value) == PROMSXP) {
      string sv = emit_call1("SYMVALUE", make_symbol(e));
      h = ParseInfo::global_constants->appl2("Rf_eval", sv, "R_GlobalEnv");
    } else {
      h = ParseInfo::global_constants->appl1("SYMVALUE", make_symbol(e), Unprotected);
    }
  }
  ParseInfo::binding_map.insert(pair<string,string>(name, h));
  return Expression(h, false, VISIBLE, "");
}


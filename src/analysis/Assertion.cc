#include <analysis/Utils.h>
#include <analysis/AnalysisResults.h>
#include <ParseInfo.h>
#include "Assertion.h"

namespace RAnnot {

void process_assert(SEXP assertion, FuncInfo* fi) {
  if (is_rcc_assert(assertion)) {
    // .rcc.assert
    SEXP list = CDR(assertion);
    for (; list != R_NilValue; list = CDR(list)) {
      SEXP e = CAR(list);
      if (is_var(e)) {
	// assertion of the form:
	// .rcc.assert(foo)
	if (e == Rf_install("no.oo")) {
	  ParseInfo::set_allow_oo(false);
	} else if (e == Rf_install("no.envir.manip")) {
	  ParseInfo::set_allow_envir_manip(false);
	} else if (e == Rf_install("no.special.redef")) {
	  ParseInfo::set_allow_special_redef(false);
	} else if (e == Rf_install("no.builtin.redef")) {
	  ParseInfo::set_allow_builtin_redef(false);
	} else if (e == Rf_install("no.library.redef")) {
	  ParseInfo::set_allow_library_redef(false);
	}
      } else if (is_value_assert(e)) {
	// assertion that a formal argument is call-by-value, of the form:
	// .rcc.assert(value(foo))
	SEXP v = CADR(e);
	char* vname = CHAR(PRINTNAME(v));
	int position = fi->findArgPosition(vname);
	SEXP arg = fi->getArg(position);
	FormalArgInfo* fargInfo = getProperty(FormalArgInfo, arg);
	fargInfo->setIsValue(true);
      }
    }
  } else if (is_rcc_assert_exp(assertion)) {
    // .rcc.assert.exp
    SEXP body_of_e = CDR(assertion);
    if (is_simple_assign(CDR(assertion))) //no attribute given in the assertion
      ;
    else {  //see if the attribute is "unique_definiton"
      SEXP body_of_e = CDR(assertion);
      SEXP header =CAR(body_of_e); //is_simple_assign(header) is true 
      SEXP attrib = CADR(body_of_e); 
      char *funcname;
      if (is_simple_assign(header)) { //get funcition name  
	funcname = CHAR(PRINTNAME(CADR(header)));
	if ( attrib == Rf_install("unique_definition"))  {
	  func_unique_defintion.insert(std::make_pair(funcname,fi)); 
	}
      }
    }
  } else if (is_rcc_assert_sym(assertion)) {
    // .rcc.assert.sym
  }
}
  
}

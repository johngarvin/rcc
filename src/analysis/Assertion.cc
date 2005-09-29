#include <analysis/Utils.h>
#include <analysis/AnalysisResults.h>
#include <analysis/AnalysisResults.h>
#include "Assertion.h"

namespace RAnnot {

void process_assert(SEXP assertion, FuncInfo* fi) {
  if (is_rcc_assert(assertion)) {
    // rcc_assert
    SEXP list = CDR(assertion);
    for (; list != R_NilValue; list = CDR(list)) {
      SEXP e = CAR(list);
      SEXP v = CADR(e);
      char* vname = CHAR(PRINTNAME(v));
      int position = fi->findArgPosition(vname);
      SEXP arg = fi->getArg(position);
      FormalArgInfo* fargInfo = getProperty(FormalArgInfo, arg);
      if (CAR(e) == Rf_install("value")) {
	fargInfo->setIsValue(true);
      } else if (CAR(e) == Rf_install("prom")) {
	fargInfo->setIsValue(false);
      }
    }
  } else if (is_rcc_assert_exp(assertion)) {
    // rcc_assert_expression
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
  }
}

}

#include <IOStuff.h>
#include "rcc_lib.h"

SEXP tagged_cons(SEXP car, SEXP tag, SEXP cdr) {
  SEXP ls = cons(car, cdr);
  SET_TAG(ls, tag);
  return ls;
}

Rcomplex mk_complex(double r, double i) {
  Rcomplex out;
  out.r = r;
  out.i = i;
  return out;
}

SEXP rcc_cons(SEXP car, SEXP cdr, int unp_car, int unp_cdr) {
  SEXP out;
  PROTECT(out = cons(car, cdr));
  if (unp_car) UNPROTECT_PTR(car);
  if (unp_cdr) UNPROTECT_PTR(cdr);
  return out;
}

Rboolean my_asLogicalNoNA(SEXP s)
{
    Rboolean cond = asLogical(s);
    if (cond == NA_LOGICAL) {
	char *msg = isLogical(s) ?
	    "missing value where logical needed" :
	    "argument is not interpretable as logical";
	errorcall(R_NilValue, msg);
    }
    return cond;
}

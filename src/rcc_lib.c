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

#include <IOStuff.h>
#include "rcc_lib.h"

SEXP tagged_cons(SEXP car, SEXP tag, SEXP cdr) {
  SEXP ls = cons(car, cdr);
  SET_TAG(ls, tag);
  return ls;
}

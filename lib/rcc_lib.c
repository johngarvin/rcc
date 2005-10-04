
#include <IOStuff.h>
#include "rcc_prot.h"
#include "rcc_lib.h"
#include "replacements.h"

void my_init_memory(SEXP mem, int n) {
  int i;
  for (i = 0; i < n; i++) {
    (mem+1)->sxpinfo.gp = 0;
    CDR(mem+i) = (i == 0 ? R_NilValue : &mem[i-1]);
    TYPEOF(mem+i) = LISTSXP;
    ATTRIB(mem+i) = R_NilValue;
    TAG(mem+i) = R_NilValue;
    MARK(mem+i) = 0;
    //    fool the GC into not taking
    //    self-allocated memory when its children are R-allocated in
    //    old_to_new situations
    mem[i].gengc_next_node = NULL;
    mem[i].gengc_prev_node = NULL;
    mem[i].sxpinfo.gcgen = -1000;
  }
}

SEXP tagged_cons(SEXP car, SEXP tag, SEXP cdr) {
  SEXP ls = cons(car, cdr);
  TAG(ls) = tag;
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

Rboolean my_asLogicalNoNA(SEXP s) {
    Rboolean cond = asLogical(s);
    if (cond == NA_LOGICAL) {
	char *msg = isLogical(s) ?
	    "missing value where logical needed" :
	    "argument is not interpretable as logical";
	errorcall(R_NilValue, msg);
    }
    return cond;
}

/* Modified from do_subassign_dflt in subassign.c */
SEXP rcc_subassign(SEXP x, SEXP sub, SEXP y) {
  int oldtype = 0;

  if (NAMED(x) == 2) {
    x = duplicate(x);
  }

  if (TYPEOF(x) == LISTSXP || TYPEOF(x) == LANGSXP) {
    PROTECT(x = PairToVectorList(x));
  }
  else if (length(x) == 0) {
    if (length(y) == 0) {
      return(x);
    }
    else {
      /* bug PR#2590 coerce only if null */
      if(isNull(x)) PROTECT(x = coerceVector(x, TYPEOF(y)));
      else PROTECT(x);
    }
  }
  else {
    PROTECT(x);
  }
  
  switch (TYPEOF(x)) {
  case LGLSXP:
  case INTSXP:
  case REALSXP:
  case CPLXSXP:
  case STRSXP:
  case EXPRSXP:
  case VECSXP:
    x = VectorAssign(R_NilValue, x, sub, y);
    break;
  default:
    error("object is not subsettable");
    break;
  }
  
  if (oldtype == LANGSXP) {
    x = VectorToPairList(x);
    SET_TYPEOF(x, LANGSXP);
  }
  
  UNPROTECT(1);
  SET_NAMED(x,0);
  return x;
}

/* -*- Mode: C -*-
 *
 * Copyright (c) 2007 Rice University
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 
 * File: rcc_lib.c
 *
 * Library of functions for RCC-generated code to call at runtime.
 *
 * Author: John Garvin (garvin@cs.rice.edu)
 */

#include <stdarg.h>

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
    /* fool the GC into not taking self-allocated memory when its
     * children are R-allocated in old_to_new situations
     */
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

/* rcc_subassign functions                            */
/* All modified from do_subassign_dflt in subassign.c */

SEXP rcc_subassign_0(SEXP x, SEXP y) {
  return rcc_subassign_varargs(x, y, 0);
}

SEXP rcc_subassign_1(SEXP x, SEXP sub, SEXP y) {
  return rcc_subassign_varargs(x, y, 1, sub);
}


SEXP rcc_subassign_cons(SEXP x, SEXP subs, SEXP y) {
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

  /* MatrixAssign, etc. mutate the input subs list; prevent them from
     mutating the original list in the constant pool */
  subs = duplicate(subs);
  
  switch (TYPEOF(x)) {
  case LGLSXP:
  case INTSXP:
  case REALSXP:
  case CPLXSXP:
  case STRSXP:
  case EXPRSXP:
  case VECSXP:
  case RAWSXP:
    switch (length(subs)) {
    case 0:
      x = VectorAssign(R_NilValue, x, R_MissingArg, y);
      break;
    case 1:
      x = VectorAssign(R_NilValue, x, CAR(subs), y);
      break;
    case 2:
      x = MatrixAssign(R_NilValue, x, subs, y);
      break;
    default:
      x = ArrayAssign(R_NilValue, x, subs, y);
      break;
    }
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

SEXP rcc_subassign_varargs(SEXP x, SEXP y, int nsubs, ...) {
  va_list ap;
  SEXP arg;

  va_start(ap, nsubs);

  int oldtype = 0;

  if (NAMED(x) == 2) {
    x = duplicate(x);
  }

  if (TYPEOF(x) == LISTSXP || TYPEOF(x) == LANGSXP) {
    PROTECT(x = PairToVectorList(x));
  }
  else if (length(x) == 0) {
    if (length(y) == 0) {
      va_end(ap);
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
  case RAWSXP:
    switch (nsubs) {
    case 0:
      x = rcc_VectorAssign(R_NilValue, x, R_MissingArg, y);
      break;
    case 1:
      x = rcc_VectorAssign(R_NilValue, x, va_arg(ap, SEXP), y);
      break;
      /*     case 2:  */
      /* x = rcc_MatrixAssign(R_NilValue, x, subs, y); */
      /* TODO: write version of MatrixAssign with deconsed "subs" argument */
    default:
      error("RCC error: rcc_subassign_varargs called with more than one subscript");
      /* x = ArrayAssign(R_NilValue, x, subs, y); */
      break;
    }
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
  va_end(ap);
  return x;
}

/* constructs evaluated promises out of an argument list to be passed
 * to applyClosure based on promiseArgs in eval.c; altered to deal
 * specially with arguments that are determined to be call-by-value by
 * assertion or analysis
 */
SEXP rcc_promise_args(SEXP el, SEXP rho) {
    SEXP ans, h, tail;

    PROTECT(ans = tail = CONS(R_NilValue, R_NilValue));

    while(el != R_NilValue) {

	/* If we have a ... symbol, we look to see what it is bound to.
	 * If its binding is Null (i.e. zero length)
	 * we just ignore it and return the cdr with all its
	 * expressions promised; if it is bound to a ... list
	 * of promises, we repromise all the promises and then splice
	 * the list of resulting values into the return value.
	 * Anything else bound to a ... symbol is an error
	 */

	/* Is this double promise mechanism really needed? */

	if (CAR(el) == R_DotsSymbol) {
	    h = findVar(CAR(el), rho);
	    if (TYPEOF(h) == DOTSXP || h == R_NilValue) {
		while (h != R_NilValue) {
		    SETCDR(tail, CONS(mkPROMISE(CAR(h), rho), R_NilValue));
		    SET_TAG(CDR(tail), CreateTag(TAG(h)));
		    tail = CDR(tail);
		    h = CDR(h);
		}
	    }
	    else if (h != R_MissingArg)
		error(_("... used in an incorrect context"));
	}
	else if (CAR(el) == R_MissingArg) {
	    SETCDR(tail, CONS(R_MissingArg, R_NilValue));
	    tail = CDR(tail);
	    SET_TAG(tail, CreateTag(TAG(el)));
	}
	else {
	  /* create already-evaluated promise */
	    SEXP promise = mkPROMISE(CAR(el), rho);
	    SET_PRVALUE(promise, CAR(el));
	    SET_PRSEEN(promise, TRUE);
	    SET_PRCODE(promise, R_NilValue);
	    SETCDR(tail, CONS(promise, R_NilValue));
	    tail = CDR(tail);
	    SET_TAG(tail, CreateTag(TAG(el)));
	}
	el = CDR(el);
    }
    UNPROTECT(1);
    return CDR(ans);  
}

SEXP make_thunked_promise(SEXP value) {
  SEXP promise = mkPROMISE(R_NilValue, R_NilValue);
  SET_PRVALUE(promise, value);
  return promise;
}

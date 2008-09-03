
#include <stdarg.h>
#include <stdlib.h>
#include <sys/file.h>
#include <IOStuff.h>
#include <Parse.h>
#include "arithmetic.h"
#include "replacements.h"

static SEXP real_binary(ARITHOP_TYPE, SEXP, SEXP);
static SEXP integer_binary(ARITHOP_TYPE, SEXP, SEXP, SEXP);

/* Copies of R internal functions without PROTECT_WITH_INDEX */

/* i1 = i % n1; i2 = i % n2;
 * this macro is quite a bit faster than having real modulo calls
 * in the loop (tested on Intel and Sparc)
 */
#define mod_iterate(n1,n2,i1,i2) for (i=i1=i2=0; i<n; \
	i1 = (++i1 == n1) ? 0 : i1,\
	i2 = (++i2 == n2) ? 0 : i2,\
	++i)
# define OPPOSITE_SIGNS(x, y) ((x < 0) ^ (y < 0))
# define GOODISUM(x, y, z) (((x) > 0) ? ((y) < (z)) : ! ((y) < (z)))
# define GOODIDIFF(x, y, z) (!(OPPOSITE_SIGNS(x, y) && OPPOSITE_SIGNS(x, z)))
#define GOODIPROD(x, y, z) ((double) (x) * (double) (y) == (z))
#define INTEGER_OVERFLOW_WARNING "NAs produced by integer overflow"
# define R_pow	pow

double R_tmp;

#if 0

static double myfmod(double x1, double x2)
{
    double q = x1 / x2;
    return x1 - floor(q) * x2;
}

/* Copy of R_binary that doesn't use 'class' as a variable name. Was
   preventing C++ compilation. */
SEXP rcc_R_binary(SEXP call, SEXP op, SEXP x, SEXP y)
{
    SEXP clazz, dims, tsp, xnames, ynames;
    int mismatch, nx, ny, xarray, yarray, xts, yts;
    SEXP lcall = call;
    SEXP copy;

    /* fix up NULL */
    if (isNull(x)) {
      copy = x;
      PROTECT(x = allocVector(REALSXP,0));
    }
    if (isNull(y)) {
      copy = y;
      PROTECT(y = allocVector(REALSXP,0));
    }

    if (!(isNumeric(x) || isComplex(x)) || !(isNumeric(y) || isComplex(y))) {
	errorcall(lcall, "non-numeric argument to binary operator");
	return R_NilValue;	/* -Wall */
    }

    mismatch = 0;
    xarray = isArray(x);
    yarray = isArray(y);
    xts = isTs(x);
    yts = isTs(y);

    /* If either x or y is a matrix with length 1 and the other is a
       vector, we want to coerce the matrix to be a vector. */

    /* FIXME: Danger Will Robinson.
     * -----  We might be trashing arguments here.
     * If we have NAMED(x) or NAMED(y) we should duplicate!
     */
    if (xarray != yarray) {
	if (xarray && length(x)==1 && length(y)!=1) {
	  copy = x;
	  PROTECT(x = duplicate(x));
	  setAttrib(x, R_DimSymbol, R_NilValue);
	}
	if (yarray && length(y)==1 && length(x)!=1) {
	  copy = y;
	  PROTECT(y = duplicate(y));
	  setAttrib(y, R_DimSymbol, R_NilValue);
	}
    }

    if (xarray || yarray) {
	nx = length(x);
	ny = length(y);
	if (xarray && yarray) {
	    if (!conformable(x, y))
		errorcall(lcall, "non-conformable arrays");
	    PROTECT(dims = getAttrib(x, R_DimSymbol));
	}
	else if (xarray) {
	    PROTECT(dims = getAttrib(x, R_DimSymbol));
	}
	else {			/* (yarray) */
	    PROTECT(dims = getAttrib(y, R_DimSymbol));
	}
	PROTECT(xnames = getAttrib(x, R_DimNamesSymbol));
	PROTECT(ynames = getAttrib(y, R_DimNamesSymbol));
    }
    else {
	nx = length(x);
	ny = length(y);
	if (nx > 0 && ny > 0) {
	    if (nx > ny) mismatch = nx % ny;
	    else mismatch = ny % nx;
	}
	PROTECT(dims = R_NilValue);
	PROTECT(xnames = getAttrib(x, R_NamesSymbol));
	PROTECT(ynames = getAttrib(y, R_NamesSymbol));
    }

    if (xts || yts) {
	if (xts && yts) {
	    if (!tsConform(x, y))
		errorcall(lcall, "Non-conformable time-series");
	    PROTECT(tsp = getAttrib(x, R_TspSymbol));
	    PROTECT(clazz = getAttrib(x, R_ClassSymbol));
	}
	else if (xts) {
	    if (length(x) < length(y))
		ErrorMessage(lcall, ERROR_TSVEC_MISMATCH);
	    PROTECT(tsp = getAttrib(x, R_TspSymbol));
	    PROTECT(clazz = getAttrib(x, R_ClassSymbol));
	}
	else {			/* (yts) */
	    if (length(y) < length(x))
		ErrorMessage(lcall, ERROR_TSVEC_MISMATCH);
	    PROTECT(tsp = getAttrib(y, R_TspSymbol));
	    PROTECT(clazz = getAttrib(y, R_ClassSymbol));
	}
    } else {
	clazz = tsp = R_NilValue; /* -Wall */
    }

    if (mismatch)
	warningcall(lcall, "longer object length\n\tis not a multiple of shorter object length");

/* need to preserve object here, as *_binary copies class attributes */
    if (TYPEOF(x) == CPLXSXP || TYPEOF(y) == CPLXSXP) {
        int xo = OBJECT(x), yo = OBJECT(y);
	copy = x;
	PROTECT(x = coerceVector(x, CPLXSXP));
	copy = y;
	PROTECT(y = coerceVector(y, CPLXSXP));
        SET_OBJECT(x, xo);
	SET_OBJECT(y, yo);
	x = complex_binary(PRIMVAL(op), x, y);
    }
    else
	if (TYPEOF(x) == REALSXP || TYPEOF(y) == REALSXP) {
            int xo = OBJECT(x), yo = OBJECT(y);
	    copy = x;
	    PROTECT(x = coerceVector(x, REALSXP));
	    copy = y;
	    PROTECT(y = coerceVector(y, REALSXP));
	    SET_OBJECT(x, xo);
	    SET_OBJECT(y, yo);
	    x = real_binary(PRIMVAL(op), x, y);
	}
	else {
	    x = integer_binary(PRIMVAL(op), x, y, lcall);
	}

    PROTECT(x);
    /* Don't set the dims if one argument is an array of size 0 and the
       other isn't of size zero, cos they're wrong */
    /* Not if the other argument is a scalar (PR#1979) */
    if (dims != R_NilValue) {
	if (!((xarray && (nx == 0) && (ny > 1)) ||
	      (yarray && (ny == 0) && (nx > 1)))){
	    setAttrib(x, R_DimSymbol, dims);
	    if (xnames != R_NilValue)
		setAttrib(x, R_DimNamesSymbol, xnames);
	    else if (ynames != R_NilValue)
		setAttrib(x, R_DimNamesSymbol, ynames);
	}
    }
    else {
	if (length(x) == length(xnames))
	    setAttrib(x, R_NamesSymbol, xnames);
	else if (length(x) == length(ynames))
	    setAttrib(x, R_NamesSymbol, ynames);
    }

    if (xts || yts) {		/* must set *after* dims! */
	setAttrib(x, R_TspSymbol, tsp);
	setAttrib(x, R_ClassSymbol, clazz);
	UNPROTECT(2);
    }

    UNPROTECT(6);
    return x;
}

SEXP rcc_do_arith(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP ans;

    if (DispatchGroup("Ops", call, op, args, env, &ans))
	return ans;

    switch (length(args)) {
    case 1:
	return R_unary(call, op, CAR(args));
    case 2:
	return rcc_R_binary(call, op, CAR(args), CADR(args));
    default:
	error("operator needs one or two arguments");
    }
    return ans;			/* never used; to keep -Wall happy */
}

static SEXP integer_binary(ARITHOP_TYPE code, SEXP s1, SEXP s2, SEXP lcall)
{
    int i, i1, i2, n, n1, n2;
    int x1, x2;
    SEXP ans;
    Rboolean naflag = FALSE;

    n1 = LENGTH(s1);
    n2 = LENGTH(s2);
    /* S4-compatibility change: if n1 or n2 is 0, result is of length 0 */
    if (n1 == 0 || n2 == 0) n = 0; else n = (n1 > n2) ? n1 : n2;

    if (code == DIVOP || code == POWOP)
	ans = allocVector(REALSXP, n);
    else
	ans = allocVector(INTSXP, n);
    if (n1 == 0 || n2 == 0) return(ans);
    /* note: code below was surely wrong in DIVOP and POWOP cases,
       since ans was a REALSXP.
     */

/*    if (n1 < 1 || n2 < 1) {
	for (i = 0; i < n; i++)
	    INTEGER(ans)[i] = NA_INTEGER;
	return ans;
	} */

    switch (code) {
    case PLUSOP:
	mod_iterate(n1, n2, i1, i2) {
	    x1 = INTEGER(s1)[i1];
	    x2 = INTEGER(s2)[i2];
	    if (x1 == NA_INTEGER || x2 == NA_INTEGER)
		INTEGER(ans)[i] = NA_INTEGER;
	    else {
		int val = x1 + x2;
		if (val != NA_INTEGER && GOODISUM(x1, x2, val))
		    INTEGER(ans)[i] = val;
		else {
		    INTEGER(ans)[i] = NA_INTEGER;
		    naflag = TRUE;
		}
	    }
	}
	if (naflag)
	    warningcall(lcall, INTEGER_OVERFLOW_WARNING);
	break;
    case MINUSOP:
	mod_iterate(n1, n2, i1, i2) {
	    x1 = INTEGER(s1)[i1];
	    x2 = INTEGER(s2)[i2];
	    if (x1 == NA_INTEGER || x2 == NA_INTEGER)
		INTEGER(ans)[i] = NA_INTEGER;
	    else {
		int val = x1 - x2;
		if (val != NA_INTEGER && GOODIDIFF(x1, x2, val))
		    INTEGER(ans)[i] = val;
		else {
		    naflag = TRUE;
		    INTEGER(ans)[i] = NA_INTEGER;
		}
	    }
	}
	if (naflag)
	    warningcall(lcall, INTEGER_OVERFLOW_WARNING);
	break;
    case TIMESOP:
	mod_iterate(n1, n2, i1, i2) {
	    x1 = INTEGER(s1)[i1];
	    x2 = INTEGER(s2)[i2];
	    if (x1 == NA_INTEGER || x2 == NA_INTEGER)
		INTEGER(ans)[i] = NA_INTEGER;
	    else {
		int val = x1 * x2;
		if (val != NA_INTEGER && GOODIPROD(x1, x2, val))
		    INTEGER(ans)[i] = val;
		else {
		    naflag = TRUE;
		    INTEGER(ans)[i] = NA_INTEGER;
		}
	    }
	}
	if (naflag)
	    warningcall(lcall, INTEGER_OVERFLOW_WARNING);
	break;
    case DIVOP:
	mod_iterate(n1, n2, i1, i2) {
	    x1 = INTEGER(s1)[i1];
	    x2 = INTEGER(s2)[i2];
#ifdef IEEE_754
	    if (x1 == NA_INTEGER || x2 == NA_INTEGER)
#else
		if (x1 == NA_INTEGER || x2 == NA_INTEGER || x2 == 0)
#endif
		    REAL(ans)[i] = NA_REAL;
		else
		    REAL(ans)[i] = (double) x1 / (double) x2;
	}
	break;
    case POWOP:
	mod_iterate(n1, n2, i1, i2) {
	    x1 = INTEGER(s1)[i1];
	    x2 = INTEGER(s2)[i2];
	    if (x1 == NA_INTEGER || x2 == NA_INTEGER)
		REAL(ans)[i] = NA_REAL;
	    else {
		REAL(ans)[i] = MATH_CHECK(R_pow((double) x1, (double) x2));
	    }
	}
	break;
    case MODOP:
	mod_iterate(n1, n2, i1, i2) {
	    x1 = INTEGER(s1)[i1];
	    x2 = INTEGER(s2)[i2];
	    if (x1 == NA_INTEGER || x2 == NA_INTEGER || x2 == 0)
		INTEGER(ans)[i] = NA_INTEGER;
	    else {
		INTEGER(ans)[i] = /* till 0.63.2:	x1 % x2 */
		    (x1 >= 0 && x2 > 0) ? x1 % x2 :
		    (int)myfmod((double)x1,(double)x2);
	    }
	}
	break;
    case IDIVOP:
	mod_iterate(n1, n2, i1, i2) {
	    x1 = INTEGER(s1)[i1];
	    x2 = INTEGER(s2)[i2];
	    if (x1 == NA_INTEGER || x2 == NA_INTEGER)
		INTEGER(ans)[i] = NA_INTEGER;
	    else if (x2 == 0)
		INTEGER(ans)[i] = 0;
	    else
		INTEGER(ans)[i] = floor((double)x1 / (double)x2);
	}
	break;
    }


    /* Copy attributes from longer argument. */

    if (n1 > n2)
	copyMostAttrib(s1, ans);
    else if (n1 == n2) {
	copyMostAttrib(s2, ans);
	copyMostAttrib(s1, ans);
    }
    else
	copyMostAttrib(s2, ans);

    return ans;
}

static SEXP real_binary(ARITHOP_TYPE code, SEXP s1, SEXP s2)
{
    int i, i1, i2, n, n1, n2;
    SEXP ans;
#ifndef IEEE_754
    double x1, x2;
#endif

    /* Note: "s1" and "s2" are protected above. */
    n1 = LENGTH(s1);
    n2 = LENGTH(s2);

    /* S4-compatibility change: if n1 or n2 is 0, result is of length 0 */
    if (n1 == 0 || n2 == 0) return(allocVector(REALSXP, 0));

    n = (n1 > n2) ? n1 : n2;
    ans = allocVector(REALSXP, n);

/*    if (n1 < 1 || n2 < 1) {
      for (i = 0; i < n; i++)
      REAL(ans)[i] = NA_REAL;
      return ans;
      } */

    switch (code) {
    case PLUSOP:
	mod_iterate(n1, n2, i1, i2) {
#ifdef IEEE_754
	    REAL(ans)[i] = REAL(s1)[i1] + REAL(s2)[i2];
#else
	    x1 = REAL(s1)[i1];
	    x2 = REAL(s2)[i2];
	    if (ISNA(x1) || ISNA(x2))
		REAL(ans)[i] = NA_REAL;
	    else
		REAL(ans)[i] = MATH_CHECK(x1 + x2);
#endif
	}
	break;
    case MINUSOP:
	mod_iterate(n1, n2, i1, i2) {
#ifdef IEEE_754
	    REAL(ans)[i] = REAL(s1)[i1] - REAL(s2)[i2];
#else
	    x1 = REAL(s1)[i1];
	    x2 = REAL(s2)[i2];
	    if (ISNA(x1) || ISNA(x2))
		REAL(ans)[i] = NA_REAL;
	    else
		REAL(ans)[i] = MATH_CHECK(x1 - x2);
#endif
	}
	break;
    case TIMESOP:
	mod_iterate(n1, n2, i1, i2) {
#ifdef IEEE_754
	    REAL(ans)[i] = REAL(s1)[i1] * REAL(s2)[i2];
#else
	    x1 = REAL(s1)[i1];
	    x2 = REAL(s2)[i2];
	    if (ISNA(x1) && ISNA(x2))
		REAL(ans)[i] = NA_REAL;
	    else
		REAL(ans)[i] = MATH_CHECK(x1 * x2);
#endif
	}
	break;
    case DIVOP:
	mod_iterate(n1, n2, i1, i2) {
#ifdef IEEE_754
	    REAL(ans)[i] = REAL(s1)[i1] / REAL(s2)[i2];
#else
	    x1 = REAL(s1)[i1];
	    x2 = REAL(s2)[i2];
	    if (ISNA(x1) || ISNA(x2) || x2 == 0)
		REAL(ans)[i] = NA_REAL;
	    else
		REAL(ans)[i] = MATH_CHECK(x1 / x2);
#endif
	}
	break;
    case POWOP:
	mod_iterate(n1, n2, i1, i2) {
#ifdef IEEE_754
	    REAL(ans)[i] = R_pow(REAL(s1)[i1], REAL(s2)[i2]);
#else
	    x1 = REAL(s1)[i1];
	    x2 = REAL(s2)[i2];
	    if (ISNA(x1) || ISNA(x2))
		REAL(ans)[i] = NA_REAL;
	    else
		REAL(ans)[i] = MATH_CHECK(R_pow(x1, x2));
#endif
	}
	break;
    case MODOP:
	mod_iterate(n1, n2, i1, i2) {
#ifdef IEEE_754
	    REAL(ans)[i] = myfmod(REAL(s1)[i1], REAL(s2)[i2]);
#else
	    x1 = REAL(s1)[i1];
	    x2 = REAL(s2)[i2];
	    if (ISNA(x1) || ISNA(x2) || x2 == 0)
		REAL(ans)[i] = NA_REAL;
	    else
		REAL(ans)[i] = MATH_CHECK(myfmod(x1, x2));
#endif
	}
	break;
    case IDIVOP:
	mod_iterate(n1, n2, i1, i2) {
#ifdef IEEE_754
	    REAL(ans)[i] = floor(REAL(s1)[i1] / REAL(s2)[i2]);
#else
	    x1 = REAL(s1)[i1];
	    x2 = REAL(s2)[i2];
	    if (ISNA(x1) || ISNA(x2))
		REAL(ans)[i] = NA_REAL;
	    else {
		if (x2 == 0)
		    REAL(ans)[i] = 0;
		else
		    REAL(ans)[i] = MATH_CHECK(floor(x1 / x2));
	    }
#endif
	}
	break;
    }


    /* Copy attributes from longer argument. */

    if (n1 > n2)
	copyMostAttrib(s1, ans);
    else if (n1 == n2) {
	copyMostAttrib(s2, ans);
	copyMostAttrib(s1, ans);
    }
    else
	copyMostAttrib(s2, ans);

    return ans;
}

#endif

/* Replacement for static VectorAssign in subassign.c */
SEXP rcc_VectorAssign(SEXP call, SEXP x, SEXP s, SEXP y)
{
    SEXP dim, indx;
    int i, ii, iy, n, nx, ny, stretch, which;
    double ry;

    if (isNull(x) && isNull(y)) {
	return R_NilValue;
    }

    /* Check to see if we have special matrix subscripting. */
    /* If so, we manufacture a real subscript vector. */

    dim = getAttrib(x, R_DimSymbol);
    if (isMatrix(s) && isArray(x) &&
	    (isInteger(s) || isReal(s)) &&
	    ncols(s) == length(dim)) {
	s = mat2indsub(dim, s);
    }
    PROTECT(s);

    stretch = 1;
    PROTECT(indx = makeSubscript(x, s, &stretch));
    n = length(indx);

    /* Here we make sure that the LHS has */
    /* been coerced into a form which can */
    /* accept elements from the RHS. */
    which = SubassignTypeFix(&x, &y, stretch, 1, call);
    /* = 100 * TYPEOF(x) + TYPEOF(y);*/
    ny = length(y);
    nx = length(x);

    if ((TYPEOF(x) != VECSXP && TYPEOF(x) != EXPRSXP) || y != R_NilValue) {
	if (n > 0 && ny == 0)
	    errorcall(call, "nothing to replace with");
	if (n > 0 && n % ny)
	    warning("number of items to replace is not a multiple of replacement length");
    }


    PROTECT(x);

    /* When array elements are being permuted the RHS */
    /* must be duplicated or the elements get trashed. */
    /* FIXME : this should be a shallow copy for list */
    /* objects.  A full duplication is wasteful. */

    if (x == y)
	PROTECT(y = duplicate(y));
    else
	PROTECT(y);

    /* Note that we are now committed. */
    /* Since we are mutating existing objects, */
    /* any changes we make now are (likely to be) permanent.  Beware! */

    switch(which) {

    case 1010:	/* logical   <- logical	  */
    case 1310:	/* integer   <- logical	  */
    case 1013:	/* logical   <- integer	  */
    case 1313:	/* integer   <- integer	  */

	for (i = 0; i < n; i++) {
	    ii = INTEGER(indx)[i];
	    if (ii == NA_INTEGER) continue;
	    ii = ii - 1;
	    INTEGER(x)[ii] = INTEGER(y)[i % ny];
	}
	break;

    case 1410:	/* real	     <- logical	  */
    case 1413:	/* real	     <- integer	  */

	for (i = 0; i < n; i++) {
	    ii = INTEGER(indx)[i];
	    if (ii == NA_INTEGER) continue;
	    ii = ii - 1;
	    iy = INTEGER(y)[i % ny];
	    if (iy == NA_INTEGER)
		REAL(x)[ii] = NA_REAL;
	    else
		REAL(x)[ii] = iy;
	}
	break;

    case 1014:	/* logical   <- real	  */
    case 1314:	/* integer   <- real	  */
    case 1414:	/* real	     <- real	  */

	for (i = 0; i < n; i++) {
	    ii = INTEGER(indx)[i];
	    if (ii == NA_INTEGER) continue;
	    ii = ii - 1;
	    REAL(x)[ii] = REAL(y)[i % ny];
	}
	break;

    case 1510:	/* complex   <- logical	  */
    case 1513:	/* complex   <- integer	  */

	for (i = 0; i < n; i++) {
	    ii = INTEGER(indx)[i];
	    if (ii == NA_INTEGER) continue;
	    ii = ii - 1;
	    iy = INTEGER(y)[i % ny];
	    if (iy == NA_INTEGER) {
		COMPLEX(x)[ii].r = NA_REAL;
		COMPLEX(x)[ii].i = NA_REAL;
	    }
	    else {
		COMPLEX(x)[ii].r = iy;
		COMPLEX(x)[ii].i = 0.0;
	    }
	}
	break;

    case 1514:	/* complex   <- real	  */

	for (i = 0; i < n; i++) {
	    ii = INTEGER(indx)[i];
	    if (ii == NA_INTEGER) continue;
	    ii = ii - 1;
	    ry = REAL(y)[i % ny];
	    if (ISNA(ry)) {
		COMPLEX(x)[ii].r = NA_REAL;
		COMPLEX(x)[ii].i = NA_REAL;
	    }
	    else {
		COMPLEX(x)[ii].r = ry;
		COMPLEX(x)[ii].i = 0.0;
	    }
	}
	break;

    case 1015:	/* logical   <- complex	  */
    case 1315:	/* integer   <- complex	  */
    case 1415:	/* real	     <- complex	  */
    case 1515:	/* complex   <- complex	  */

	for (i = 0; i < n; i++) {
	    ii = INTEGER(indx)[i];
	    if (ii == NA_INTEGER) continue;
	    ii = ii - 1;
	    COMPLEX(x)[ii] = COMPLEX(y)[i % ny];
	}
	break;

    case 1610:	/* character <- logical	  */
    case 1613:	/* character <- integer	  */
    case 1614:	/* character <- real	  */
    case 1615:	/* character <- complex	  */
    case 1616:	/* character <- character */
    case 1016:	/* logical   <- character */
    case 1316:	/* integer   <- character */
    case 1416:	/* real	     <- character */
    case 1516:	/* complex   <- character */

	for (i = 0; i < n; i++) {
	    ii = INTEGER(indx)[i];
	    if (ii == NA_INTEGER) continue;
	    ii = ii - 1;
	    SET_STRING_ELT(x, ii, STRING_ELT(y, i % ny));
	}
	break;

    case 1019:  /* vector     <- logical   */
    case 1319:  /* vector     <- integer   */
    case 1419:  /* vector     <- real      */
    case 1519:  /* vector     <- complex   */
    case 1619:  /* vector     <- character */

    case 1910:  /* vector     <- logical    */
    case 1913:  /* vector     <- integer    */
    case 1914:  /* vector     <- real       */
    case 1915:  /* vector     <- complex    */
    case 1916:  /* vector     <- character  */

    case 1919:  /* vector     <- vector     */

	for (i = 0; i < n; i++) {
	    ii = INTEGER(indx)[i];
	    if (ii == NA_INTEGER) continue;
	    ii = ii - 1;
	    SET_VECTOR_ELT(x, ii, VECTOR_ELT(y, i % ny));
	}
	break;

    case 2001:
    case 2006:	/* expression <- language   */
    case 2010:	/* expression <- logical    */
    case 2013:	/* expression <- integer    */
    case 2014:	/* expression <- real	    */
    case 2015:	/* expression <- complex    */
    case 2016:	/* expression <- character  */
    case 2020:	/* expression <- expression */

	for (i = 0; i < n; i++) {
	    ii = INTEGER(indx)[i];
	    if (ii == NA_INTEGER) continue;
	    ii = ii - 1;
	    SET_VECTOR_ELT(x, ii, VECTOR_ELT(y, i % ny));
	}
	break;

    case 1900:  /* vector     <- null       */
    case 2000:  /* expression <- null       */

	x = DeleteListElements(x, indx);
	UNPROTECT(4);
	return x;

    default:
	warningcall(call, "sub assignment (*[*] <- *) not done; __bug?__");
    }
    /* Check for additional named elements. */
    /* Note we are using a horrible hack in makeSubscript */
    /* Which passes the additional names back in the attribute */
    /* slot of the generated subscript vector.  (Shudder!) */
    if (ATTRIB(indx) != R_NilValue) {
	SEXP newnames = ATTRIB(indx);
	SEXP oldnames = getAttrib(x, R_NamesSymbol);
	if (oldnames != R_NilValue) {
	    for (i = 0; i < n; i++) {
		if (STRING_ELT(newnames, i) != R_NilValue) {
		    ii = INTEGER(indx)[i];
		    if (ii == NA_INTEGER) continue;
		    ii = ii - 1;
		    SET_STRING_ELT(oldnames, ii, STRING_ELT(newnames, i));
		}
	    }
	}
	else {
	    PROTECT(oldnames = allocVector(STRSXP, nx));
	    for (i = 0; i < nx; i++)
		SET_STRING_ELT(oldnames, i, R_BlankString);
	    for (i = 0; i < n; i++) {
		if (STRING_ELT(newnames, i) != R_NilValue) {
		    ii = INTEGER(indx)[i];
		    if (ii == NA_INTEGER) continue;
		    ii = ii - 1;
		    SET_STRING_ELT(oldnames, ii, STRING_ELT(newnames, i));
		}
	    }
	    setAttrib(x, R_NamesSymbol, oldnames);
	    UNPROTECT(1);
	}
    }
    UNPROTECT(4);
    return x;
}

/* replacement of static MatrixAssign in subassign.c */
SEXP rcc_MatrixAssign(SEXP call, SEXP x, SEXP s, SEXP y) {
    int i, j, ii, jj, ij, iy, k, n, which;
    double ry;
    int nr, ny;
    int nrs, ncs;
    SEXP sr, sc, dim;

    if (!isMatrix(x))
	error(_("incorrect number of subscripts on matrix"));

    nr = nrows(x);
    ny = LENGTH(y);

    /* Note that "s" has been protected. */
    /* No GC problems here. */

    dim = getAttrib(x, R_DimSymbol);
    sr = SETCAR(s, arraySubscript(0, CAR(s), dim, getAttrib,
				  (STRING_ELT), x));
    sc = SETCADR(s, arraySubscript(1, CADR(s), dim, getAttrib,
				   (STRING_ELT), x));
    nrs = LENGTH(sr);
    ncs = LENGTH(sc);
    if(ny > 1) {
	for(i = 0; i < nrs; i++)
	    if(INTEGER(sr)[i] == NA_INTEGER)
		error(_("NAs are not allowed in subscripted assignments"));
	for(i = 0; i < ncs; i++)
	    if(INTEGER(sc)[i] == NA_INTEGER)
		error(_("NAs are not allowed in subscripted assignments"));
    }

    n = nrs * ncs;

    /* <TSL> 21Oct97
       if (length(y) == 0)
       error("Replacement length is zero");
       </TSL>  */

    if (n > 0 && ny == 0)
	errorcall(call, _("nothing to replace with"));
    if (n > 0 && n % ny)
	errorcall(call, _("number of items to replace is not a multiple of replacement length"));

    which = SubassignTypeFix(&x, &y, 0, 1, call);

    PROTECT(x);

    /* When array elements are being permuted the RHS */
    /* must be duplicated or the elements get trashed. */
    /* FIXME : this should be a shallow copy for list */
    /* objects.  A full duplication is wasteful. */

    if (x == y)
	PROTECT(y = duplicate(y));
    else
	PROTECT(y);

    /* Note that we are now committed.  Since we are mutating */
    /* existing objects any changes we make now are permanent. */
    /* Beware! */

    k = 0;
    switch (which) {
	/* because we have called SubassignTypeFix the commented
	   values cannot occur (and would be unsafe) */

    case 1010:	/* logical   <- logical	  */
    case 1310:	/* integer   <- logical	  */
    /* case 1013: logical   <- integer	  */
    case 1313:	/* integer   <- integer	  */

	for (j = 0; j < ncs; j++) {
	    jj = INTEGER(sc)[j];
	    if (jj == NA_INTEGER) continue;
	    jj = jj - 1;
	    for (i = 0; i < nrs; i++) {
		ii = INTEGER(sr)[i];
		if (ii == NA_INTEGER) continue;
		ii = ii - 1;
		ij = ii + jj * nr;
		INTEGER(x)[ij] = INTEGER(y)[k];
		k = (k + 1) % ny;
	    }
	}
	break;

    case 1410:	/* real	     <- logical	  */
    case 1413:	/* real	     <- integer	  */

	for (j = 0; j < ncs; j++) {
	    jj = INTEGER(sc)[j];
	    if (jj == NA_INTEGER) continue;
	    jj = jj - 1;
	    for (i = 0; i < nrs; i++) {
		ii = INTEGER(sr)[i];
		if (ii == NA_INTEGER) continue;
		ii = ii - 1;
		ij = ii + jj * nr;
		iy = INTEGER(y)[k];
		if (iy == NA_INTEGER)
		    REAL(x)[ij] = NA_REAL;
		else
		    REAL(x)[ij] = iy;
		k = (k + 1) % ny;
	    }
	}
	break;

    /* case 1014:  logical   <- real	  */ 
    /* case 1314:  integer   <- real	  */
    case 1414:	/* real	     <- real	  */

	for (j = 0; j < ncs; j++) {
	    jj = INTEGER(sc)[j];
	    if (jj == NA_INTEGER) continue;
	    jj = jj - 1;
	    for (i = 0; i < nrs; i++) {
		ii = INTEGER(sr)[i];
		if (ii == NA_INTEGER) continue;
		ii = ii - 1;
		ij = ii + jj * nr;
		REAL(x)[ij] = REAL(y)[k];
		k = (k + 1) % ny;
	    }
	}
	break;

    case 1510:	/* complex   <- logical	  */
    case 1513:	/* complex   <- integer	  */

	for (j = 0; j < ncs; j++) {
	    jj = INTEGER(sc)[j];
	    if (jj == NA_INTEGER) continue;
	    jj = jj - 1;
	    for (i = 0; i < nrs; i++) {
		ii = INTEGER(sr)[i];
		if (ii == NA_INTEGER) continue;
		ii = ii - 1;
		ij = ii + jj * nr;
		iy = INTEGER(y)[k];
		if (iy == NA_INTEGER) {
		    COMPLEX(x)[ij].r = NA_REAL;
		    COMPLEX(x)[ij].i = NA_REAL;
		}
		else {
		    COMPLEX(x)[ij].r = iy;
		    COMPLEX(x)[ij].i = 0.0;
		}
		k = (k + 1) % ny;
	    }
	}
	break;

    case 1514:	/* complex   <- real	  */

	for (j = 0; j < ncs; j++) {
	    jj = INTEGER(sc)[j];
	    if (jj == NA_INTEGER) continue;
	    jj = jj - 1;
	    for (i = 0; i < nrs; i++) {
		ii = INTEGER(sr)[i];
		if (ii == NA_INTEGER) continue;
		ii = ii - 1;
		ij = ii + jj * nr;
		ry = REAL(y)[k];
		if (ISNA(ry)) {
		    COMPLEX(x)[ij].r = NA_REAL;
		    COMPLEX(x)[ij].i = NA_REAL;
		}
		else {
		    COMPLEX(x)[ij].r = ry;
		    COMPLEX(x)[ij].i = 0.0;
		}
		k = (k + 1) % ny;
	    }
	}
	break;

    /* case 1015:  logical   <- complex	  */
    /* case 1315:  integer   <- complex	  */
    /* case 1415:  real	     <- complex	  */
    case 1515:	/* complex   <- complex	  */

	for (j = 0; j < ncs; j++) {
	    jj = INTEGER(sc)[j];
	    if (jj == NA_INTEGER) continue;
	    jj = jj - 1;
	    for (i = 0; i < nrs; i++) {
		ii = INTEGER(sr)[i];
		if (ii == NA_INTEGER) continue;
		ii = ii - 1;
		ij = ii + jj * nr;
		COMPLEX(x)[ij] = COMPLEX(y)[k];
		k = (k + 1) % ny;
	    }
	}
	break;

    case 1610:	/* character <- logical	  */
    case 1613:	/* character <- integer	  */
    case 1614:	/* character <- real	  */
    case 1615:	/* character <- complex	  */
    case 1616:	/* character <- character */
    /* case 1016:  logical   <- character */
    /* case 1316:  integer   <- character */
    /* case 1416:  real	     <- character */
    /* case 1516:  complex   <- character */

	for (j = 0; j < ncs; j++) {
	    jj = INTEGER(sc)[j];
	    if (jj == NA_INTEGER) continue;
	    jj = jj - 1;
	    for (i = 0; i < nrs; i++) {
		ii = INTEGER(sr)[i];
		if (ii == NA_INTEGER) continue;
		ii = ii - 1;
		ij = ii + jj * nr;
		SET_STRING_ELT(x, ij, STRING_ELT(y, k));
		k = (k + 1) % ny;
	    }
	}
	break;
    case 1919: /* vector <- vector */

	for (j = 0; j < ncs; j++) {
	    jj = INTEGER(sc)[j];
	    if (jj == NA_INTEGER) continue;
	    jj = jj - 1;
	    for (i = 0; i < nrs; i++) {
		ii = INTEGER(sr)[i];
		if (ii == NA_INTEGER) continue;
		ii = ii - 1;
		ij = ii + jj * nr;
		SET_VECTOR_ELT(x, ij, VECTOR_ELT(y, k));
		k = (k + 1) % ny;
	    }
	}
	break;
    default:
	error(_("incompatible types (case %d) in matrix subset assignment"), 
	      which);
    }
    UNPROTECT(2);
    return x;
}

/* copied from subassign.c */
/* EnlargeVector() takes a vector "x" and changes its length to "newlen".
   This allows to assign values "past the end" of the vector or list.
   Note that, unlike S, we only extend as much as is necessary.
*/
SEXP EnlargeVector(SEXP x, R_len_t newlen)
{
    R_len_t i, len;
    SEXP newx, names, newnames;

    /* Sanity Checks */
    if (!isVector(x))
	error("attempt to enlarge non-vector");

    /* Enlarge the vector itself. */
    len = length(x);
    if (LOGICAL(GetOption(install("check.bounds"), R_NilValue))[0])
	warning("assignment outside vector/list limits (extending from %d to %d)",
		len, newlen);
    PROTECT(x);
    PROTECT(newx = allocVector(TYPEOF(x), newlen));

    /* Copy the elements into place. */
    switch(TYPEOF(x)) {
    case LGLSXP:
    case INTSXP:
	for (i = 0; i < len; i++)
	    INTEGER(newx)[i] = INTEGER(x)[i];
	for (i = len; i < newlen; i++)
	    INTEGER(newx)[i] = NA_INTEGER;
	break;
    case REALSXP:
	for (i = 0; i < len; i++)
	    REAL(newx)[i] = REAL(x)[i];
	for (i = len; i < newlen; i++)
	    REAL(newx)[i] = NA_REAL;
	break;
    case CPLXSXP:
	for (i = 0; i < len; i++)
	    COMPLEX(newx)[i] = COMPLEX(x)[i];
	for (i = len; i < newlen; i++) {
	    COMPLEX(newx)[i].r = NA_REAL;
	    COMPLEX(newx)[i].i = NA_REAL;
	}
	break;
    case STRSXP:
	for (i = 0; i < len; i++)
	    SET_STRING_ELT(newx, i, STRING_ELT(x, i));
	for (i = len; i < newlen; i++)
	    SET_STRING_ELT(newx, i, NA_STRING); /* was R_BlankString  < 1.6.0 */
	break;
    case EXPRSXP:
    case VECSXP:
	for (i = 0; i < len; i++)
	    SET_VECTOR_ELT(newx, i, VECTOR_ELT(x, i));
	for (i = len; i < newlen; i++)
	    SET_VECTOR_ELT(newx, i, R_NilValue);
	break;
    }

    /* Adjust the attribute list. */
    names = getAttrib(x, R_NamesSymbol);
    if (!isNull(names)) {
	PROTECT(newnames = allocVector(STRSXP, newlen));
	for (i = 0; i < len; i++)
	    SET_STRING_ELT(newnames, i, STRING_ELT(names, i));
	for (i = len; i < newlen; i++)
	    SET_STRING_ELT(newnames, i, R_BlankString);
	setAttrib(newx, R_NamesSymbol, newnames);
	UNPROTECT(1);
    }
    copyMostAttrib(x, newx);
    UNPROTECT(2);
    return newx;
}

/* copied from subassign.c */
int SubassignTypeFix(SEXP *x, SEXP *y, int stretch, int level, SEXP call)
{
    Rboolean redo_which =  (level == 1);
    int which = 100 * TYPEOF(*x) + TYPEOF(*y);
    /* coercion can lose the object bit */
    Rboolean x_is_object = OBJECT(*x);

    switch (which) {

    case 1900:  /* vector     <- null       */
    case 2000:  /* expression <- null       */

    case 1010:	/* logical    <- logical    */
    case 1310:	/* integer    <- logical    */
    case 1410:	/* real	      <- logical    */
    case 1510:	/* complex    <- logical    */
    case 1313:	/* integer    <- integer    */
    case 1413:	/* real	      <- integer    */
    case 1513:	/* complex    <- integer    */
    case 1414:	/* real	      <- real	    */
    case 1514:	/* complex    <- real	    */
    case 1515:	/* complex    <- complex    */
    case 1616:	/* character  <- character  */
    case 1919:  /* vector     <- vector     */
    case 2020:	/* expression <- expression */

	redo_which = FALSE;
	break;

    case 1013:	/* logical    <- integer    */

	*x = coerceVector(*x, INTSXP);
	break;

    case 1014:	/* logical    <- real	    */
    case 1314:	/* integer    <- real	    */

	*x = coerceVector(*x, REALSXP);
	break;

    case 1015:	/* logical    <- complex    */
    case 1315:	/* integer    <- complex    */
    case 1415:	/* real	      <- complex    */

	*x = coerceVector(*x, CPLXSXP);
	break;

    case 1610:	/* character  <- logical    */
    case 1613:	/* character  <- integer    */
    case 1614:	/* character  <- real	    */
    case 1615:	/* character  <- complex    */

	*y = coerceVector(*y, STRSXP);
	break;

    case 1016:	/* logical    <- character  */
    case 1316:	/* integer    <- character  */
    case 1416:	/* real	      <- character  */
    case 1516:	/* complex    <- character  */

	*x = coerceVector(*x, STRSXP);
	break;

    case 1901:  /* vector     <- symbol   */
    case 1904:  /* vector     <- environment   */
    case 1905:  /* vector     <- promise   */
    case 1906:  /* vector     <- language   */
    case 1910:  /* vector     <- logical    */
    case 1913:  /* vector     <- integer    */
    case 1914:  /* vector     <- real       */
    case 1915:  /* vector     <- complex    */
    case 1916:  /* vector     <- character  */
    case 1920:  /* vector     <- expression  */
#ifdef BYTECODE
    case 1921:  /* vector     <- bytecode   */
#endif
    case 1922:  /* vector     <- eternal pointer */
    case 1923:  /* vector     <- weak reference */
    case 1903: case 1907: case 1908: case 1999: /* functions */

	if (level == 1) {
	    /* Coerce the RHS into a list */
	    *y = coerceVector(*y, VECSXP);
	}
	else {
	    /* Wrap the RHS in a list */
	    SEXP tmp;
	    PROTECT(tmp = allocVector(VECSXP, 1));
	    SET_VECTOR_ELT(tmp, 0, NAMED(*y) ? duplicate(*y) : *y);
	    *y = tmp;
	    UNPROTECT(1);
	}
	break;

    case 1019:  /* logical    <- vector     */
    case 1319:  /* integer    <- vector     */
    case 1419:  /* real       <- vector     */
    case 1519:  /* complex    <- vector     */
    case 1619:  /* character  <- vector     */
	*x = coerceVector(*x, VECSXP);
	break;

    case 2001:	/* expression <- symbol	    */
    case 2006:	/* expression <- language   */
    case 2010:	/* expression <- logical    */
    case 2013:	/* expression <- integer    */
    case 2014:	/* expression <- real	    */
    case 2015:	/* expression <- complex    */
    case 2016:	/* expression <- character  */
    case 2019:  /* expression <- vector     */

	/* Note : No coercion is needed here. */
	/* We just insert the RHS into the LHS. */
	/* FIXME: is this true or should it be just like the "vector" case? */
	redo_which = FALSE;
	break;

    default:
	errorcall(call, "incompatible types");

    }

    if (stretch) {
	PROTECT(*y);
	*x = EnlargeVector(*x, stretch);
	UNPROTECT(1);
    }
    SET_OBJECT(*x, x_is_object);

    if(redo_which)
	return(100 * TYPEOF(*x) + TYPEOF(*y));
    else
	return(which);
}

SEXP DeleteListElements(SEXP x, SEXP which)
{
    SEXP include, xnew, xnames, xnewnames;
    R_len_t i, ii, len, lenw;
    len = length(x);
    lenw = length(which);
    /* calculate the length of the result */
    PROTECT(include = allocVector(INTSXP, len));
    for (i = 0; i < len; i++)
	INTEGER(include)[i] = 1;
    for (i = 0; i < lenw; i++) {
	ii = INTEGER(which)[i];
	if (0 < ii  && ii <= len)
	    INTEGER(include)[ii - 1] = 0;
    }
    ii = 0;
    for (i = 0; i < len; i++)
	ii += INTEGER(include)[i];
    if (ii == len) {
	UNPROTECT(1);
	return x;
    }
    PROTECT(xnew = allocVector(VECSXP, ii));
    ii = 0;
    for (i = 0; i < len; i++) {
	if (INTEGER(include)[i] == 1) {
	    SET_VECTOR_ELT(xnew, ii, VECTOR_ELT(x, i));
	    ii++;
	}
    }
    xnames = getAttrib(x, R_NamesSymbol);
    if (xnames != R_NilValue) {
	PROTECT(xnewnames = allocVector(STRSXP, ii));
	ii = 0;
	for (i = 0; i < len; i++) {
	    if (INTEGER(include)[i] == 1) {
		SET_STRING_ELT(xnewnames, ii, STRING_ELT(xnames, i));
		ii++;
	    }
	}
	setAttrib(xnew, R_NamesSymbol, xnewnames);
	UNPROTECT(1);
    }
    copyMostAttrib(x, xnew);
    UNPROTECT(2);
    return xnew;
}


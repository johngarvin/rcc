#include <stdarg.h>
#include <stdlib.h>
#include <sys/file.h>
#include <IOStuff.h>
#include <Parse.h>
#include "arithmetic.h"
/* #include "get_name.h" */
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

static double myfmod(double x1, double x2)
{
    double q = x1 / x2;
    return x1 - floor(q) * x2;
}

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

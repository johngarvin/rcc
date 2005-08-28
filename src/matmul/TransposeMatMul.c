/* Matrix multiplication with transposed arguments */
/* altered version of array.c from the R sources */

#include <include/R/R_RInternals.h>

SEXP do_matprod_t(SEXP x, SEXP y, SEXP trans_info);
static void matprod_t(double *x, int nrx, int ncx,
	              double *y, int nry, int ncy, double *z,
	              char *transa, char *transb);

/* Note: assumes non-complex args          */
/* manual specialization: PRIMVAL(op) == 0 */
/* because we are %*%, not crossprod       */
SEXP do_matprod_t(SEXP x, SEXP y, SEXP trans_info) {
  int ldx, ldy, nrx, ncx, nry, ncy, mode;
  SEXP xdims, ydims, ans;
  Rboolean sym;
  
  sym = Rf_isNull(y);
  if ( !(Rf_isNumeric(x) || Rf_isComplex(x))) 
    Rf_errorcall(x,"Matrix transpose requires numeric matrix/vector arguments");

  if(!(Rf_isNumeric(y) || Rf_isComplex(y)) )
    Rf_errorcall(y,"Matrix transpose requires numeric matrix/vector arguments");
  
  xdims = Rf_getAttrib(x, R_DimSymbol);
  ydims = Rf_getAttrib(y, R_DimSymbol);
  ldx = Rf_length(xdims);
  ldy = Rf_length(ydims);
  
  if (ldx != 2 && ldy != 2) {		/* x and y non-matrices */
    nrx = 1;
    ncx = LENGTH(x);
    nry = LENGTH(y);
    ncy = 1;
  }
  else if (ldx != 2) {		/* x not a matrix */
    nry = INTEGER(ydims)[0];
    ncy = INTEGER(ydims)[1];
    nrx = 0;
    ncx = 0;
    if (LENGTH(x) == nry) {	/* x as row vector */
      nrx = 1;
      ncx = LENGTH(x);
    }
    else if (nry == 1) {	/* x as col vector */
      nrx = LENGTH(x);
      ncx = 1;
    }
  }
  else if (ldy != 2) {		/* y not a matrix */
    nrx = INTEGER(xdims)[0];
    ncx = INTEGER(xdims)[1];
    nry = 0;
    ncy = 0;
    if (LENGTH(y) == ncx) {	/* y as col vector */
      nry = LENGTH(y);
      ncy = 1;
    }
    else if (ncx == 1) {	/* y as row vector */
      nry = 1;
      ncy = LENGTH(y);
    }
  }
  else {				/* x and y matrices */
    nrx = INTEGER(xdims)[0];
    ncx = INTEGER(xdims)[1];
    nry = INTEGER(ydims)[0];
    ncy = INTEGER(ydims)[1];
  }
  
  if (Rf_isComplex(x))
    Rf_errorcall(x,"Matrix transpose optimization not yet implemented for complex matrices");
  else if (Rf_isComplex(y))
    Rf_errorcall(y,"Matrix transpose optimization not yet implemented for complex matrices");
  else
    mode = REALSXP;

  x = Rf_coerceVector(x, mode);
  y = Rf_coerceVector(y, mode);
  
  PROTECT(ans = Rf_allocMatrix(mode, nrx, ncy));
  matprod_t(REAL(x), nrx, ncx,
	    REAL(y), nry, ncy, REAL(ans),
	    "t","t");
  
  PROTECT(xdims = Rf_getAttrib(x, R_DimNamesSymbol));
  PROTECT(ydims = Rf_getAttrib(y, R_DimNamesSymbol));
  
  if (xdims != R_NilValue || ydims != R_NilValue) {
    SEXP dimnames, dimnamesnames, dn;
    PROTECT(dimnames = Rf_allocVector(VECSXP, 2));
    PROTECT(dimnamesnames = Rf_allocVector(STRSXP, 2));
    if (xdims != R_NilValue) {
      if (ldx == 2 || ncx ==1) {
	dn = Rf_getAttrib(xdims, R_NamesSymbol);
	SET_VECTOR_ELT(dimnames, 0, VECTOR_ELT(xdims, 0));
	if(!Rf_isNull(dn))
	  SET_STRING_ELT(dimnamesnames, 0, STRING_ELT(dn, 0));
      }
    }
    if (ydims != R_NilValue) {
      if (ldy == 2 ){
	dn = Rf_getAttrib(ydims, R_NamesSymbol);
	SET_VECTOR_ELT(dimnames, 1, VECTOR_ELT(ydims, 1));
	if(!Rf_isNull(dn))
	  SET_STRING_ELT(dimnamesnames, 1, STRING_ELT(dn, 1));
      } else if (nry == 1) {
	dn = Rf_getAttrib(ydims, R_NamesSymbol);
	SET_VECTOR_ELT(dimnames, 1, VECTOR_ELT(ydims, 0));
	if(!Rf_isNull(dn))
	  SET_STRING_ELT(dimnamesnames, 1, STRING_ELT(dn, 0));
      }
    }
    Rf_setAttrib(dimnames, R_NamesSymbol, dimnamesnames);
    Rf_setAttrib(ans, R_DimNamesSymbol, dimnames);
    UNPROTECT(2);
  }

  UNPROTECT(3);
  return ans;
}


static void matprod_t(double *x, int nrx, int ncx,
	              double *y, int nry, int ncy, double *z,
	              char *transa, char *transb)
#ifdef IEEE_754
{
    int i,  j, k;
    double one = 1.0, zero = 0.0, sum;
    Rboolean have_na = FALSE;

    if (nrx > 0 && ncx > 0 && nry > 0 && ncy > 0) {
	/* Don't trust the BLAS to handle NA/NaNs correctly: PR#4582
	 * The test is only O(n) here
	 */
	for (i = 0; i < nrx*ncx; i++)
	    if (ISNAN(x[i])) {have_na = TRUE; break;}
	if (!have_na) 
	    for (i = 0; i < nry*ncy; i++)
		if (ISNAN(y[i])) {have_na = TRUE; break;}
	if (have_na) {
	    for (i = 0; i < nrx; i++)
		for (k = 0; k < ncy; k++) {
		    sum = 0.0;
		    for (j = 0; j < ncx; j++)
			sum += x[i + j * nrx] * y[j + k * nry];
		    z[i + k * nrx] = sum;
		}
	} else
	    F77_CALL(dgemm)(transa, transb, &nrx, &ncy, &ncx, &one,
			    x, &nrx, y, &nry, &zero, z, &nrx);
    } else /* zero-extent operations should return zeroes */
	for(i = 0; i < nrx*ncy; i++) z[i] = 0;
}
#else
{
  Rf_errorcall(R_NilValue, "Matrix multiplication with transposition without BLAS is not yet implemented");
}
#endif

#include "R_opt.h"

using namespace OA;

void opt_matmul(OA_ptr<CFG::CFGIRInterface> rir_ptr, SEXP exp)
{
  // For matrix multiplication where one or more arguments is
  // transposed (e.g. "t(x) %*% y"), replace with a form that does the
  // transposition in the computation. This avoids the need to create
  // a whole new transposed matrix.

  const SEXP sym_t_matmul_t = install("t#%*%#t");
  const SEXP sym_t_matmul = install("t#%*%");
  const SEXP sym_matmul_t = install("%*%#t");
  const SEXP sym_matmul = install("%*%");
  const SEXP sym_t = install("t");
  
  R_PreorderIterator pre_iter(exp);
  for( ; pre_iter.isValid(); ++pre_iter)
  {
    SEXP e = pre_iter.current();
    if (CAR(e) == sym_matmul)
    {
      //      cout << "Matrix multiply:\n";
      //      PrintExpression(e);
      if (CAR(CADR(e)) == sym_t       // both args transposed
	  && CAR(CADDR(e)) == sym_t)
      {
	SETCAR(e, sym_t_matmul_t);
	SETCAR(CDR(e), CADR(CADR(e)));  // remove left "t"
	SETCAR(CDDR(e), CADR(CADDR(e))); // remove right "t"
      }
      else if (CAR(CADR(e)) == sym_t) // left arg is transposed
      {
	SETCAR(e, sym_t_matmul);
	SETCAR(CDR(e), CADR(CADR(e)));  // remove left "t"
      }
      else if (CAR(CADDR(e)) == sym_t) // right arg is transposed
      {
	SETCAR(e, sym_matmul_t);
	SETCAR(CDDR(e), CADR(CADDR(e))); // remove right "t"	
      }
      //      cout << "New matrix multiply:\n";
      //      PrintExpression(e);
    }
  }
}

// callable from C
SEXP opt_matmul_plain(SEXP e)
{
  R_IRInterface *rir = new R_IRInterface;
  OA_ptr<CFG::CFGIRInterface> rir_ptr;
  rir_ptr = rir;

  opt_matmul(rir_ptr, e);
  return e;
}

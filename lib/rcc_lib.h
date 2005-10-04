void my_init_memory(SEXP mem, int n);
SEXP tagged_cons(SEXP car, SEXP tag, SEXP cdr);
Rcomplex mk_complex(double r, double i);
SEXP rcc_cons(SEXP car, SEXP cdr, int unp_car, int unp_cdr);
Rboolean my_asLogicalNoNA(SEXP s);
SEXP R_binary(SEXP call, SEXP op, SEXP x, SEXP y);
SEXP R_unary(SEXP call, SEXP op, SEXP x);
SEXP rcc_do_arith(SEXP call, SEXP op, SEXP args, SEXP env);
SEXP rcc_subassign(SEXP x, SEXP sub, SEXP y);

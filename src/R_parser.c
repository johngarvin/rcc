/* Copyright (c) 2003-2004 John Garvin 
 *
 * July 11, 2003 
 *
 * Parses R code, turns into C code that uses internal R functions.
 * Attempts to output some code in regular C rather than using R
 * functions.
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
 */

#include "R_parser.h"

void init_R() {
  char *myargs[5];
  myargs[0] = "";
  myargs[1] = "--gui=none";
  myargs[2] = "--slave";
  myargs[3] = "--vanilla";
  myargs[4] = NULL;
  Rf_initialize_R(4,myargs);
  setup_Rmainloop();
}

/* Reads 'in_file' and parses it as R code. Sets 'exps' as a
 * NULL-terminated array of SEXPs representing the list of
 * expressions.
 */
void parse_R(FILE *in_file, SEXP *p_exps[]) {
  int n = 0;
  SEXP e;
  ParseStatus status;
  SEXP *exps;

  exps = (SEXP *)malloc(sizeof(SEXP));

  do {
    /* parse each expression */
    PROTECT(e = R_Parse1File(in_file, 1, &status));
    switch(status) {
    case PARSE_NULL:
      break;
    case PARSE_OK:
      n++;
      exps = (SEXP *)realloc((void *)exps, (n+1)*sizeof(SEXP));
      exps[n-1] = e;
      break;
    case PARSE_INCOMPLETE:
      break;
    case PARSE_ERROR:
      fprintf(stderr, "Error: parsing returned PARSE_ERROR.\n");
      exit(1);
      break;
    case PARSE_EOF:
      break;
    }
  } while (status != PARSE_EOF && status != PARSE_INCOMPLETE);
  exps[n] = NULL;

  *p_exps = exps;
}

/* Parse R code into a sequence of R AST expressions, then makes and
 * returns the sequence as a big function with no arguments. If the
 * input file containts expressions e1,e2,...en, then the output is
 * the SEXP representation of something like this:
 *
 * function()
 * {
 *   e1
 *   e2
 *   ...
 *   en
 * }
 *
 * FIXME: This is done because OpenAnalysis assumes that, as in C or
 * Fortran, all code is within some function--an assumption that's
 * invalid for languages like R. This hack alters the semantics:
 * definitions that were global are now local.
 */
SEXP parse_R_as_function(FILE *in_file) {
  SEXP *exps, *e;
  SEXP stmts = R_NilValue;
  parse_R(in_file, &exps);
  if (*exps != NULL) {
    e = exps;
    while(*e != NULL) {
      e++;
    }
    do {
      e--;
      stmts = CONS(*e, stmts);
    } while (e != exps);
  }
  PROTECT(stmts);
  SEXP lbrace = Rf_install("{");
  SEXP body = PROTECT(LCONS(lbrace, stmts));
  UNPROTECT_PTR(stmts);
  SEXP args = R_NilValue; // empty argument list
  SEXP func_sym = Rf_install("function");
  SEXP func_exp = PROTECT(LCONS(func_sym, CONS(args, CONS(body, R_NilValue))));
  UNPROTECT_PTR(body);
  SEXP fname = Rf_install("<toplevel>");
  SEXP arrow = Rf_install("<-");
  SEXP big_exp = PROTECT(LCONS(arrow, CONS(fname, CONS(func_exp, R_NilValue))));
  UNPROTECT_PTR(func_exp);
  return big_exp;
}

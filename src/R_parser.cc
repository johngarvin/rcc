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

// Parses R code. Reads 'filename' if it exists or STDIN if filename
// is NULL. Sets 'e' to be a list of the resulting R
// expressions. Returns the number of expressions.

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

/*
  if (!filename) {
    inFile = stdin;
  } else {
    inFile = fopen(filename, "r");
  }
  if (!inFile) {
    cerr << "Error: input file \"" << filename << "\" not found\n";
    exit(1);
  }
*/


// int parse_R(list<SEXP> & e, char *filename) {
list<SEXP> *parse_R(FILE *in_file) {
  list<SEXP> *exps;
  SEXP e;
  ParseStatus status;

  exps = new list<SEXP>;

  do {
    /* parse each expression */
    PROTECT(e = R_Parse1File(in_file, 1, &status));
    switch(status) {
    case PARSE_NULL:
      break;
    case PARSE_OK:
      exps->push_back(e);
      break;
    case PARSE_INCOMPLETE:
      break;
    case PARSE_ERROR:
      err("parsing returned PARSE_ERROR.\n");
      break;
    case PARSE_EOF:
      break;
    }
  } while (status != PARSE_EOF && status != PARSE_INCOMPLETE);
  
  return exps;
}

SEXP parse_R_as_function(FILE *in_file) {
  list <SEXP> *exps;
  exps = parse_R(in_file);
  SEXP stmts = R_NilValue;
  while(!exps->empty()) {
    SEXP e = exps->back();
    stmts = CONS(e, stmts);
    exps->pop_back();
  }
  PROTECT(stmts);
  delete exps;
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

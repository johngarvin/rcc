// -*- Mode: C++ -*-
//
// Copyright (c) 2003-2006 Rice University
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
//
// File: Parser.cc
//
// Interface to the R parser. Parses R code, returns S-expressions.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <include/R/R_Parse.h>
#include <support/RccError.h>

#include <support/Parser.h>

// forward declaration of internal function

static bool is_simple_source_call(SEXP e);

void init_R() {
  char *myargs[5];
  myargs[0] = "";
  myargs[1] = "--gui=none";
  myargs[2] = "--slave";
  myargs[3] = "--vanilla";
  myargs[4] = NULL;
  Rf_initEmbeddedR(4, myargs);
}

//! Reads 'in_file' and parses it as R code. Sets 'exps' as a
//! NULL-terminated array of SEXPs representing the list of
//! expressions. The caller must free p_exps.
//!
//! If the parser encounters a call to 'source' at top level with one
//! non-named argument, it parses the named file at compile time.
void parse_R(FILE *in_file, SEXP *p_exps[]) {
  int n = 0;
  SEXP e;
  ParseStatus status;
  SEXP *exps;

  exps = (SEXP *)malloc(sizeof(SEXP));

  do {
    // parse each expression
    Rf_protect(e = R_Parse1File(in_file, 1, &status));
    switch(status) {
    case PARSE_NULL:
      break;
    case PARSE_OK:

      // special handling for source()
      if (is_simple_source_call(e)) {
	SEXP interp_arg = Rf_eval(CADR(e), R_GlobalEnv);
	if (TYPEOF(interp_arg) != STRSXP) {
	  rcc_error("Problem interpreting argument to 'source'");
	}
	char * name_c = CHAR(STRING_ELT(interp_arg, 0));
	FILE * new_file = fopen(name_c, "r");
	if (new_file == NULL) {
	  rcc_error("Couldn't open sourced file " + std::string(name_c));
	}
	SEXP * new_exps;
	parse_R(new_file, &new_exps);
	// append new exps to end
	for( ; *new_exps != NULL; new_exps++) {
	  n++;
	  exps = (SEXP *)realloc((void *)exps, (n+1)*sizeof(SEXP));
	  exps[n-1] = *new_exps;
	}
      } else {
	n++;
	exps = (SEXP *)realloc((void *)exps, (n+1)*sizeof(SEXP));
	exps[n-1] = e;
      }
      break;
    case PARSE_INCOMPLETE:
      break;
    case PARSE_ERROR:
      rcc_error("parsing returned PARSE_ERROR");
      break;
    case PARSE_EOF:
      break;
    }
    Rf_unprotect(1);
  } while (status != PARSE_EOF && status != PARSE_INCOMPLETE);
  exps[n] = NULL;

  *p_exps = exps;
}

//!  Parse R code into a sequence of R AST expressions, then makes and
//!  returns the sequence as a big function with no arguments. If the
//!  input file containts expressions e1,e2,...en, then the output is
//!  the SEXP representation of something like this:
//!  
//!  `<toplevel>` <- function()
//!  {
//!    e1
//!    e2
//!    ...
//!    en
//!  }
//!  
//!  FIXME: This is done because OpenAnalysis assumes that, as in C or
//!  Fortran, all code is within some function--an assumption that's
//!  invalid for languages like R. Beware of altering the semantics:
//!  naively using the result will cause global variables to be treated
//!  as local to the newly created function.

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
      stmts = Rf_cons(*e, stmts);
    } while (e != exps);
  }
  Rf_protect(stmts);
  SEXP lbrace = Rf_install("{");
  SEXP body = Rf_protect(Rf_lcons(lbrace, stmts));
  Rf_unprotect_ptr(stmts);
  SEXP args = R_NilValue; // empty argument list
  SEXP func_sym = Rf_install("function");
  SEXP func_exp = Rf_protect(Rf_lcons(func_sym, Rf_cons(args, Rf_cons(body, R_NilValue))));
  Rf_unprotect_ptr(body);
  SEXP fname = Rf_install("<toplevel>");
  SEXP arrow = Rf_install("<-");
  SEXP big_exp = Rf_protect(Rf_lcons(arrow, Rf_cons(fname, Rf_cons(func_exp, R_NilValue))));
  Rf_unprotect_ptr(func_exp);
  return big_exp;
}

/// Returns true if the given SEXP is a call to the 'source' function
/// with exactly one argument. The single argument must not be a named
/// argument.
static bool is_simple_source_call(SEXP e) {
  return (TYPEOF(e) == LANGSXP &&
	  CAR(e) == Rf_install("source") &&
	  CDR(e) != R_NilValue &&
	  TAG(CDR(e)) == R_NilValue &&
	  CDDR(e) == R_NilValue);
}

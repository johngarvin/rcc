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

int parse_R(list<SEXP> & e, char *filename) {
  SEXP exp;
  ParseStatus status;
  int num_exps = 0;
  FILE *inFile;
  char *myargs[5];
  myargs[0] = "";
  myargs[1] = "--gui=none";
  myargs[2] = "--slave";
  myargs[3] = "--vanilla";
  myargs[4] = NULL;
  Rf_initialize_R(4,myargs);
  setup_Rmainloop();

  if (!filename) {
    inFile = stdin;
  } else {
    inFile = fopen(filename, "r");
  }
  if (!inFile) {
    cerr << "Error: input file \"" << filename << "\" not found\n";
    exit(1);
  }

  do {
    /* parse each expression */
    PROTECT(exp = R_Parse1File(inFile, 1, &status));
    switch(status) {
    case PARSE_NULL:
      break;
    case PARSE_OK:
      num_exps++;
      e.push_back(exp);
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
  
  return num_exps;
}

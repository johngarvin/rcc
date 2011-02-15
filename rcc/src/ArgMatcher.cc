// -*- Mode: C++ -*-
//
// Copyright (c) 2003-2009 Rice University
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 

// File: ArgMatcher.cc
//
// Matches actual arguments with formal arguments, resolving named
// arguments and default arguments.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "ArgMatcher.h"

ArgMatcher::ArgMatcher(SEXP formals, SEXP supplied) : m_formals(formals), m_supplied(supplied)
{
}

SEXP ArgMatcher::match() {
  SEXP actuals;

  actuals = Rf_matchArgs(m_formals, m_supplied);
  resolve_defaults(m_formals, actuals);
  return actuals;
}

void ArgMatcher::resolve_defaults(SEXP formals, SEXP actuals) {
  SEXP f, a;
  /*  Use the default code for unbound formals. */
  
  /* This piece of code is destructively modifying the actuals list. */
  
  f = formals;
  a = actuals;
  while (f != R_NilValue) {
    if (CAR(a) == R_MissingArg && CAR(f) != R_MissingArg) {
      //      SETCAR(a, mkPROMISE(CAR(f), newrho));    // interpreter version
      SETCAR(a, CAR(f));
      SET_MISSING(a, 2);
    }
    f = CDR(f);
    a = CDR(a);
  }
}

// -*- Mode: C++ -*-
//
// Copyright (c) 2009 Rice University
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

// File: ResolvedArgAnnotationMap.cc
//
// Maps each call site to a description of its arguments with named
// arguments and default arguments resolved.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "ResolvedArgAnnotationMap.h"

namespace RAnnot {

ResolvedArgAnnotationMap::ResolvedArgAnnotationMap() {
}

ResolvedArgAnnotationMap::get_instance() {
  if (s_instance == 0) {
    s_instance = new ResolvedArgAnnotationMap();
    analysisResults.add(s_handle, s_instance);
  }
  return s_instance;
}

ResolvedArgAnnotationMap::handle() {
  if (s_instance == 0) {
    s_instance = new ResolvedArgAnnotationMap();
    analysisResults.add(s_handle, s_instance);
  }
  return s_handle;
}

ResolvedArgAnnotationMap::compute() {
  FuncInfo * fi;
  ExpressionInfo * cs;
  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_CALL_SITE(fi, cs) {
      // get unique callee if it exists and get its formal arguments
      SEXP formals = ...;
      SEXP actuals = call_args(CAR(cs->get_cell()));
      actuals = Rf_matchArgs(formals, actuals);
      actuals = resolve_defaults(formals, actuals);
      m_map[cs] = new ResolvedArgs(actuals);
    }
  }
}

ResolvedArgAnnotationMap::resolve_defaults(SEXP formals, SEXP actuals) {
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

} // namespace RAnnot

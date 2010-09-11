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

// File: CallType.cc
//
// For a procedure call, gives information: whether it's a call to a user
// procedure, library procedure, etc.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <analysis/AnalysisResults.h>
#include <analysis/OACallGraphAnnotation.h>
#include <analysis/Settings.h>
#include <analysis/Utils.h>

#include "CallType.h"

using namespace RAnnot;

static CallType non_user_call_type(SEXP lhs);

CallType get_call_type(SEXP call) {
  SEXP lhs = call_lhs(call);
  if (TYPEOF(lhs) != SYMSXP) {
    return CALL_NON_SYMBOL;
  }
  OACallGraphAnnotation * cga = 0;
  if (Settings::instance()->get_call_graph()) {
    cga = getProperty(OACallGraphAnnotation, call);
  }
  if (cga == 0) {
    // not in call graph, or call graph not available
    return non_user_call_type(lhs);
  } else {
    OA::ProcHandle proc = cga->get_singleton_if_exists();
    if (proc == OA::ProcHandle(0)) {
      return CALL_USER_MULTIPLE;
    } else {
      return CALL_USER_SINGLETON;
    }
  }
}

CallType non_user_call_type(SEXP lhs) {
  if (is_library(lhs)) {
    switch (TYPEOF(library_value(lhs))) {
    case CLOSXP:
      return CALL_LIBRARY_CLOSURE;
      break;
    case BUILTINSXP:
      return CALL_LIBRARY_BUILTIN;
      break;
    case SPECIALSXP:
      return CALL_LIBRARY_SPECIAL;
      break;
    default:
      assert(0);
      break;
    }
  } else {
    return CALL_UNKNOWN;
  }
}

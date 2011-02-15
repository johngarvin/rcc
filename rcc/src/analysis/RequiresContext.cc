// -*- Mode: C++ -*-
//
// Copyright (c) 2006 Rice University
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

// File: RequiresContext.cc
//
// Determines whether a function needs to include calls to
// beginContext and endContext when translated into C.
//
// Author: John Mellor-Crummey (johnmc@cs.rice.edu)

#include <assert.h>
#include <set>

#include <include/R/R_Internal.h>
#include <analysis/Utils.h>

#if 0
using namespace _GLIBCXX_STD;
#else
using namespace std;
#endif

#define PRIM(x) (CCODE) x
static CCODE unsafePrimitives[] = {

  // "as" functions,
  PRIM(do_ascharacter),
  PRIM(do_asvector),

  // "dim" functions),
  PRIM(do_dim),
  PRIM(do_dimgets),
  PRIM(do_dimnames),
  PRIM(do_dimnamesgets),

  // "is" functions),
  PRIM(do_is),
  PRIM(do_isna),
  PRIM(do_isnan),

  // "subassign" functions),
  PRIM(do_subassign),
  PRIM(do_subassign2),
  PRIM(do_subassign3),

  // "subset" functions),
  PRIM(do_subset),
  PRIM(do_subset2),
  PRIM(do_subset3)
};


class PrimitiveRequiresContext {
private:
  set<CCODE> theSet;
public:
  explicit PrimitiveRequiresContext(); 
  bool getPrimitiveRequiresContext(CCODE prim);
};

PrimitiveRequiresContext primRequiresContext;

//------------------------------------------------------------------------------
// forward declarations
//------------------------------------------------------------------------------

static bool listRequiresContext(SEXP e);
static bool expressionRequiresContext(SEXP e);



//------------------------------------------------------------------------------
// class methods
//------------------------------------------------------------------------------

PrimitiveRequiresContext::PrimitiveRequiresContext()
{
  int count = sizeof(unsafePrimitives)/sizeof(CCODE);
  for (int i=0;i < count; i++) {
    theSet.insert(unsafePrimitives[i]);
  }
}

bool PrimitiveRequiresContext::getPrimitiveRequiresContext(CCODE prim)
{
  return theSet.find(prim) != theSet.end();
}



//------------------------------------------------------------------------------
// interface functions
//------------------------------------------------------------------------------

bool functionRequiresContext(SEXP fundef)
{
  SEXP code = CAR(procedure_body_c(fundef));
  bool result = expressionRequiresContext(code);
  return result;
}

static bool listRequiresContext(SEXP e)
{
  if (e == R_NilValue) return false;

  if (expressionRequiresContext(CAR(e))) return true;

  return expressionRequiresContext(CDR(e));
}

static bool expressionRequiresContext(SEXP e)
{
  switch(TYPEOF(e)) {
  case NILSXP:
  case STRSXP:
  case LGLSXP:
  case INTSXP:
  case REALSXP:
  case CPLXSXP:
  case VECSXP:
  case SYMSXP:
  case CHARSXP:
  case CLOSXP:
  case ENVSXP:
  case EXPRSXP:
  case EXTPTRSXP:
  case WEAKREFSXP:
    return false;
  case LISTSXP:
    return listRequiresContext(e);
  case PROMSXP:
    return expressionRequiresContext(PREXPR(e));
  case LANGSXP:
    if (TYPEOF(call_lhs(e)) == SYMSXP && is_library(call_lhs(e))) {
      return expressionRequiresContext(library_value(call_lhs(e)));
    } else {
      return expressionRequiresContext(call_lhs(e));
    }
  case SPECIALSXP:
  case BUILTINSXP:
    return primRequiresContext.getPrimitiveRequiresContext(PRIMFUN(e)); 
  default:
    assert(0 && "Internal error: requiresContext encountered bad type\n");
    return false;
  }
}

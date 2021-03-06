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

#include "ResolvedArgsAnnotationMap.h"

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/ExpressionInfoAnnotationMap.h>
#include <analysis/FuncInfo.h>
#include <analysis/FuncInfoAnnotationMap.h>
#include <analysis/HandleInterface.h>
#include <analysis/LibraryFuncInfo.h>
#include <analysis/LibraryFuncInfoAnnotationMap.h>
#include <analysis/OACallGraphAnnotation.h>
#include <analysis/OEscapeInfoAnnotationMap.h>
#include <analysis/SexpTraversal.h>
#include <analysis/ExpressionSideEffectAnnotationMap.h>
#include <analysis/VarAnnotationMap.h>

#include <analysis/ResolvedArgs.h>

namespace RAnnot {

ResolvedArgsAnnotationMap::ResolvedArgsAnnotationMap() {
}

ResolvedArgsAnnotationMap * ResolvedArgsAnnotationMap::instance() {
  if (s_instance == 0) {
    s_instance = new ResolvedArgsAnnotationMap();
    analysisResults.add(s_handle, s_instance);
  }
  return s_instance;
}

PropertyHndlT ResolvedArgsAnnotationMap::handle() {
  if (s_instance == 0) {
    s_instance = new ResolvedArgsAnnotationMap();
    analysisResults.add(s_handle, s_instance);
  }
  return s_handle;
}

void ResolvedArgsAnnotationMap::compute() {
  SEXP formals, supplied;
  FuncInfo * fi;
  FuncInfo::const_call_site_iterator csi;
  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_CALL_SITE(fi, csi) {
      SEXP cell = *csi;
      OACallGraphAnnotation * cga = getProperty(OACallGraphAnnotation, CAR(cell));
      if (cga) {
	// call to user procedure
	OA::ProcHandle ph = cga->get_singleton_if_exists();
	if (ph == OA::ProcHandle(0)) continue;
	BasicFuncInfo * callee = getProperty(BasicFuncInfo, HandleInterface::make_sexp(ph));
	formals = fundef_args(callee->get_sexp());
      } else if (is_var(call_lhs(CAR(cell))) && is_library(call_lhs(CAR(cell))) && is_closure(library_value(call_lhs(CAR(cell))))) {
	// call to library closure
	LibraryFuncInfo * callee = getProperty(LibraryFuncInfo, library_value(call_lhs(CAR(cell))));
	formals = closure_args(callee->get_sexp());
      } else {
	continue;
      }
      ResolvedArgs * value = new ResolvedArgs(call_args(CAR(cell)), formals);
      get_map()[cell] = value;
    }
  }
}

ResolvedArgsAnnotationMap * ResolvedArgsAnnotationMap::s_instance = 0;
PropertyHndlT ResolvedArgsAnnotationMap::s_handle = "ResolvedArgs";

#if 0

  //  version from interpreter (match.c)
  //    hack: MULTIPLE_MATCHES un-#defined
  //    hack: no dots "..."

SEXP matchArgs(SEXP formals, SEXP supplied)
{
    int i;
    SEXP f, a, b, actuals;

    actuals = R_NilValue;
    for (f = formals ; f != R_NilValue ; f = CDR(f)) {
	actuals = CONS(R_MissingArg, actuals);
	SET_MISSING(actuals, 1);
	SET_ARGUSED(f, 0);
    }

    for(b = supplied; b != R_NilValue; b=CDR(b))
	SET_ARGUSED(b, 0);

    PROTECT(actuals);

    /* First pass: exact matches by tag */
    /* Grab matched arguments and check */
    /* for multiple exact matches. */

    f = formals;
    a = actuals;
    while (f != R_NilValue) {
      i = 1;
      for (b = supplied; b != R_NilValue; b = CDR(b)) {
	if (TAG(b) != R_NilValue && pmatch(TAG(f), TAG(b), 1)) {
	  if (ARGUSED(f) == 2)
	    error(_("formal argument \"%s\" matched by multiple actual arguments"),
		  var_name(TAG(f))));
	  if (ARGUSED(b) == 2)
	    error(_("argument %d matches multiple formal arguments"), i);
	  SETCAR(a, CAR(b));
	  if(CAR(b) != R_MissingArg)
	    SET_MISSING(a, 0);	/* not missing this arg */
	  SET_ARGUSED(b, 2);
	  SET_ARGUSED(f, 2);
	}
	i++;
      }
      f = CDR(f);
      a = CDR(a);
    }
    
    /* Second pass: partial matches based on tags */

    f = formals;
    a = actuals;
    while (f != R_NilValue) {
	if (ARGUSED(f) == 0) {
	  i = 1;
	  for (b = supplied; b != R_NilValue; b = CDR(b)) {
	    if (ARGUSED(b) != 2 && TAG(b) != R_NilValue &&
		pmatch(TAG(f), TAG(b), 0)) {
	      if (ARGUSED(b))
		error(_("argument %d matches multiple formal arguments"), i);
	      if (ARGUSED(f) == 1)
		error(_("formal argument \"%s\" matched by multiple actual arguments"),
		      var_name(TAG(f)).c_str());
	      SETCAR(a, CAR(b));
	      if (CAR(b) != R_MissingArg)
		SET_MISSING(a, 0);       /* not missing this arg */
	      SET_ARGUSED(b, 1);
	      SET_ARGUSED(f, 1);
	    }
	    i++;
	  }
	}
	f = CDR(f);
	a = CDR(a);
    }

    /* Third pass: matches based on order */
    /* All args specified in tag=value form */
    /* have now been matched.  If we find ... */
    /* we gobble up all the remaining args. */
    /* Otherwise we bind untagged values in */
    /* order to any unmatched formals. */

    f = formals;
    a = actuals;
    b = supplied;

    while (f != R_NilValue && b != R_NilValue) {
      if (CAR(a) != R_MissingArg) {
	    /* Already matched by tag */
	    /* skip to next formal */
	    f = CDR(f);
	    a = CDR(a);
	}
	else if (ARGUSED(b) || TAG(b) != R_NilValue) {
	    /* This value used or tagged , skip to next value */
	    /* The second test above is needed because we */
	    /* shouldn't consider tagged values for positional */
	    /* matches. */
	    /* The formal being considered remains the same */
	    b = CDR(b);
	}
	else {
	    /* We have a positional match */
	    SETCAR(a, CAR(b));
	    if(CAR(b) != R_MissingArg)
		SET_MISSING(a, 0);
	    SET_ARGUSED(b, 1);
	    b = CDR(b);
	    f = CDR(f);
	    a = CDR(a);
	}
    }


    /* Check that all arguments are used */
    for (b = supplied; b != R_NilValue; b = CDR(b))
      if (!ARGUSED(b) && CAR(b) != R_MissingArg)
	errorcall(R_GlobalContext->call,
		  _("unused argument(s) (%s ...)"),
		  /* anything better when b is "untagged" ? : */
		  TAG(b) != R_NilValue ? var_name(TAG(b)).c_str() : "");
    UNPROTECT(1);
    return(actuals);
}

#endif

} // namespace RAnnot

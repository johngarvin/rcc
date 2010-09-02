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
#include <analysis/OACallGraphAnnotation.h>
#include <analysis/OEscapeInfoAnnotationMap.h>
#include <analysis/SideEffectAnnotationMap.h>
#include <analysis/VarAnnotationMap.h>

#include <analysis/ResolvedArgs.h>

namespace RAnnot {

ResolvedArgsAnnotationMap::ResolvedArgsAnnotationMap() {
}

ResolvedArgsAnnotationMap * ResolvedArgsAnnotationMap::get_instance() {
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
  SEXP formals, actuals;
  FuncInfo * fi;
  FuncInfo::const_call_site_iterator csi;
  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_CALL_SITE(fi, csi) {
      SEXP cell = *csi;
      OACallGraphAnnotation * cga = getProperty(OACallGraphAnnotation, CAR(cell));
      if (cga == 0) continue;
      OA::ProcHandle ph = cga->get_singleton_if_exists();
      if (ph == OA::ProcHandle(0)) continue;
      FuncInfo * callee = getProperty(FuncInfo, HandleInterface::make_sexp(ph));
      if (callee == 0) continue;
      if (callee->get_has_var_args()) continue;
      formals = fundef_args(callee->get_sexp());
      actuals = call_args(CAR(cell));
      ResolvedArgs * value = new ResolvedArgs(actuals, formals);
      get_map()[cell] = value;
      SETCDR(CAR(cell), value->get_resolved());
    }
  }

  // sneak each resolved actual into ExpressionInfo map. This is
  // so that we can do CBV analysis, which wants SideEffect info,
  // which wants ExpressionInfo.
  for (const_iterator it = begin(); it != end(); it++) {
    ResolvedArgs * value = dynamic_cast<ResolvedArgs *>(it->second);
    for (SEXP x = value->get_resolved(); x != R_NilValue; x = CDR(x)) {
      ExpressionInfoAnnotationMap::get_instance()->make_annot(x);
      // give an answer for call sites that are already resolved.
      if (is_call(CAR(x))) {
	std::pair<ResolvedArgs::ResolvedSource, SEXP> pair = value->source_from_resolved(x);
	if (pair.first == ResolvedArgs::RESOLVED_ACTUAL) {
	  assert(is_call(CAR(pair.second)));
	  const_iterator res = get_map().find(pair.second);
	  if (res == get_map().end()) continue;
	  get_map()[x] = dynamic_cast<ResolvedArgs *>(res->second);
	}
      }
    }
  }
  FuncInfoAnnotationMap::get_instance()->reset();
  VarAnnotationMap::get_instance()->reset();
  SideEffectAnnotationMap::get_instance()->reset();
  OEscapeInfoAnnotationMap::get_instance()->reset();
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
		  CHAR(PRINTNAME(TAG(f))));
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
		      CHAR(PRINTNAME(TAG(f))));
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
		  TAG(b) != R_NilValue ? CHAR(PRINTNAME(TAG(b))) : "");
    UNPROTECT(1);
    return(actuals);
}

#endif

} // namespace RAnnot

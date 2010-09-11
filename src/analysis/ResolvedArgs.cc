// -*- Mode: C++ -*-
//
// Copyright (c) 2010 Rice University
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

// File: ResolvedArgs.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "ResolvedArgs.h"

#include <support/RccError.h>

#include <analysis/ResolvedArgsAnnotationMap.h>
#include <analysis/Utils.h>

#define ARGUSED(x) LEVELS(x)
#define SET_ARGUSED(x,v) SETLEVELS(x,v)

namespace RAnnot {

ResolvedArgs::ResolvedArgs(SEXP actuals, SEXP formals)
  : m_actuals(actuals),
    m_formals(formals),
    m_lazy_info(Rf_length(formals)),
    m_actual_to_resolved(),
    m_resolved_to_source()
{
  resolve();
}

void ResolvedArgs::resolve() {
  // adapted from matchArgs

  int i;
  Rboolean seendots;
  SEXP f, a, b, dots, resolved;

  resolved = R_NilValue;
  for (f = m_formals ; f != R_NilValue ; f = CDR(f)) {
    resolved = Rf_cons(R_MissingArg, resolved);
    SET_MISSING(resolved, 1);
    SET_ARGUSED(f, 0);
  }

  for(b = m_actuals; b != R_NilValue; b=CDR(b))
    SET_ARGUSED(b, 0);

  Rf_protect(resolved);

  /* First pass: exact matches by tag */
  /* Grab matched arguments and check */
  /* for multiple exact matches. */

  f = m_formals;
  a = resolved;
  while (f != R_NilValue) {
    if (TAG(f) != R_DotsSymbol) {
      i = 1;
      for (b = m_actuals; b != R_NilValue; b = CDR(b)) {
	if (TAG(b) != R_NilValue && Rf_pmatch(TAG(f), TAG(b), TRUE)) {
	  if (ARGUSED(f) == 2)
	    Rf_error(_("formal argument \"%s\" matched by multiple actual arguments"),
		     var_name(TAG(f)).c_str());
	  if (ARGUSED(b) == 2)
	    Rf_error(_("argument %d matches multiple formal arguments"), i);
	  SETCAR(a, CAR(b));
	  if(CAR(b) != R_MissingArg)
	    SET_MISSING(a, 0);	/* not missing this arg */
	  SET_ARGUSED(b, 2);
	  SET_ARGUSED(f, 2);
	  m_actual_to_resolved[b] = a;
	  m_resolved_to_source[a] = std::make_pair(RESOLVED_ACTUAL, b);
	}
	i++;
      }
    }
    f = CDR(f);
    a = CDR(a);
  }

  /* Second pass: partial matches based on tags */
  /* An exact match is required after first ... */
  /* The location of the first ... is saved in "dots" */

  dots = R_NilValue;
  seendots = FALSE;
  f = m_formals;
  a = resolved;
  while (f != R_NilValue) {
    if (ARGUSED(f) == 0) {
      if (TAG(f) == R_DotsSymbol && !seendots) {
	/* Record where ... value goes */
	dots = a;
	seendots = TRUE;
      }
      else {
	i = 1;
	for (b = m_actuals; b != R_NilValue; b = CDR(b)) {
	  if (ARGUSED(b) != 2 && TAG(b) != R_NilValue &&
	      Rf_pmatch(TAG(f), TAG(b), seendots)) {
	    if (ARGUSED(b))
	      Rf_error(_("argument %d matches multiple formal arguments"), i);
	    if (ARGUSED(f) == 1)
	      Rf_error(_("formal argument \"%s\" matched by multiple actual arguments"),
		       var_name(TAG(f)).c_str());
	    SETCAR(a, CAR(b));
	    if (CAR(b) != R_MissingArg)
	      SET_MISSING(a, 0);       /* not missing this arg */
	    SET_ARGUSED(b, 1);
	    SET_ARGUSED(f, 1);
	    m_actual_to_resolved[b] = a;
	    m_resolved_to_source[a] = std::make_pair(RESOLVED_ACTUAL, b);
	  }
	  i++;
	}
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

  f = m_formals;
  a = resolved;
  b = m_actuals;
  seendots = FALSE;

  while (f != R_NilValue && b != R_NilValue && !seendots) {
    if (TAG(f) == R_DotsSymbol) {
      /* Skip ... matching until all tags done */
      seendots = TRUE;
      f = CDR(f);
      a = CDR(a);
    }
    else if (CAR(a) != R_MissingArg) {
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
      m_actual_to_resolved[b] = a;
      m_resolved_to_source[a] = std::make_pair(RESOLVED_ACTUAL, b);
      b = CDR(b);
      f = CDR(f);
      a = CDR(a);
    }
  }

  if (dots != R_NilValue) {
    /* Gobble up all unused actuals */
    SET_MISSING(dots, 0);
    i=0;
    for(a=m_actuals; a!=R_NilValue ; a=CDR(a) )
      if(!ARGUSED(a)) i++;

    if (i) {
      a = Rf_allocList(i);
      SET_TYPEOF(a, DOTSXP);
      f=a;
      for(b=m_actuals;b!=R_NilValue;b=CDR(b))
	if(!ARGUSED(b)) {
	  SETCAR(f, CAR(b));
	  SET_TAG(f, TAG(b));
	  m_resolved_to_source[f] = std::make_pair(RESOLVED_ACTUAL, b);
	  f=CDR(f);
	}
      SETCAR(dots, a);
    }
  }
  else {
    /* Check that all arguments are used */
    for (b = m_actuals; b != R_NilValue; b = CDR(b))
      if (!ARGUSED(b) && CAR(b) != R_MissingArg)
	Rf_errorcall(R_GlobalContext->call,
		     _("unused argument(s) (%s ...)"),
		     /* anything better when b is "untagged" ? : */
		     TAG(b) != R_NilValue ? var_name(TAG(b)).c_str() : "");
  }
  Rf_unprotect(1);
  m_resolved = resolved;
}

SEXP ResolvedArgs::get_resolved() const {
  return m_resolved;
}

std::pair<ResolvedArgs::ResolvedSource, SEXP> ResolvedArgs::source_from_resolved(SEXP cell) {
  ResolvedToSourceMap::const_iterator answer = m_resolved_to_source.find(cell);
  if (answer == m_resolved_to_source.end()) {
    rcc_error("Source of resolved arg not found");
  }
  return answer->second;
}

SEXP ResolvedArgs::resolved_from_actual(SEXP cell) {
  ActualToResolvedMap::const_iterator answer = m_actual_to_resolved.find(cell);
  if (answer == m_actual_to_resolved.end()) {
    rcc_error("Resolved actual arg not found");
  }
  return answer->second;
}

EagerLazyT ResolvedArgs::get_eager_lazy(int arg) const {
  return m_lazy_info.at(arg);
}
  
void ResolvedArgs::set_eager_lazy(int arg, EagerLazyT x) {
  m_lazy_info.at(arg) = x;
}

std::vector<EagerLazyT> ResolvedArgs::get_lazy_info() const {
  return m_lazy_info;
}

AnnotationBase * ResolvedArgs::clone() {
  return 0;
}

std::ostream & ResolvedArgs::dump(std::ostream & stream) const {
  // TODO
}

PropertyHndlT ResolvedArgs::handle() {
  return ResolvedArgsAnnotationMap::handle();
}

}

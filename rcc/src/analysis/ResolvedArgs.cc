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

ResolvedArgs::ResolvedArgs(SEXP supplied, SEXP formals)
  : m_supplied(supplied),
    m_formals(formals),
    m_lazy_info(Rf_length(formals)),
    m_resolved_args(Rf_length(formals))
{
  resolve();
}

ResolvedArgs::~ResolvedArgs() {
}

void ResolvedArgs::resolve() {
  // adapted from matchArgs

  int i, j, dots;
  Rboolean seendots;
  SEXP f, a, b;
  MyArgSetT::iterator it;

  i = 0;
  for (f = m_formals ; f != R_NilValue ; f = CDR(f)) {
    SET_ARGUSED(f, 0);
    m_resolved_args.at(i).cell = Rf_cons(R_MissingArg, R_NilValue);
    m_resolved_args.at(i).formal = f;
    m_resolved_args.at(i).source = RESOLVED_DEFAULT;
    m_resolved_args.at(i).is_missing = true;
    i++;
  }

  for(b = m_supplied; b != R_NilValue; b=CDR(b))
    SET_ARGUSED(b, 0);

  /* First pass: exact matches by tag */
  /* Grab matched arguments and check */
  /* for multiple exact matches. */

  i = 0;
  for (f = m_formals; f != R_NilValue; f = CDR(f)) {
    if (TAG(f) != R_DotsSymbol) {
      j = 1;
      for (b = m_supplied; b != R_NilValue; b = CDR(b)) {
	if (TAG(b) != R_NilValue && Rf_pmatch(TAG(f), TAG(b), TRUE)) {
	  if (ARGUSED(f) == 2)
	    Rf_error(_("formal argument \"%s\" matched by multiple actual arguments"),
		     var_name(TAG(f)).c_str());
	  if (ARGUSED(b) == 2)
	    Rf_error(_("argument %d matches multiple formal arguments"), j);
	  m_resolved_args.at(i).cell = b;
	  m_resolved_args.at(i).source = RESOLVED_TAG_EXACT;
	  if(CAR(b) != R_MissingArg)
	    m_resolved_args.at(i).is_missing = false;
	  m_supplied_to_formal_map[b] = f;
	  SET_ARGUSED(b, 2);
	  SET_ARGUSED(f, 2);
	}
	j++;
      }
    }
    i++;
  }

  /* Second pass: partial matches based on tags */
  /* An exact match is required after first ... */
  /* The location of the first ... is saved in "dots" */

  dots = -1;
  seendots = FALSE;
  i = 0;
  it = m_resolved_args.begin();
  for (f = m_formals; f != R_NilValue; f = CDR(f)) {
    if (ARGUSED(f) == 0) {
      if (TAG(f) == R_DotsSymbol && !seendots) {
	/* Record where ... value goes */
	it->cell = R_NilValue;
	it->source = RESOLVED_DOT;
	it->is_missing = false;
	dots = i;
	seendots = TRUE;
      } else {
	j = 1;
	for (b = m_supplied; b != R_NilValue; b = CDR(b)) {
	  if (ARGUSED(b) != 2 && TAG(b) != R_NilValue &&
	      Rf_pmatch(TAG(f), TAG(b), seendots)) {
	    if (ARGUSED(b))
	      Rf_error(_("argument %d matches multiple formal arguments"), j);
	    if (ARGUSED(f) == 1)
	      Rf_error(_("formal argument \"%s\" matched by multiple actual arguments"),
		       var_name(TAG(f)).c_str());
	    it->cell = b;
	    it->source = RESOLVED_TAG_PARTIAL;
	    if (CAR(b) != R_MissingArg)
	      it->is_missing = false;
	    m_supplied_to_formal_map[b] = f;
	    SET_ARGUSED(b, 1);
	    SET_ARGUSED(f, 1);
	  }
	  j++;
	}
      }
    }
    it++;
    i++;
  }

  /* Third pass: matches based on order */
  /* All args specified in tag=value form */
  /* have now been matched.  If we find ... */
  /* we gobble up all the remaining args. */
  /* Otherwise we bind untagged values in */
  /* order to any unmatched formals. */

  f = m_formals;
  b = m_supplied;
  seendots = FALSE;
  i = 0;
  while (f != R_NilValue && b != R_NilValue && !seendots) {
    if (TAG(f) == R_DotsSymbol) {
      /* Skip ... matching until all tags done */
      seendots = TRUE;
      f = CDR(f);
      i++;
    }
    else if (m_resolved_args.at(i).source != RESOLVED_DEFAULT) {
      /* Already matched by tag */
      /* skip to next formal */
      f = CDR(f);
      i++;
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
      m_resolved_args.at(i).cell = b;
      m_resolved_args.at(i).source = RESOLVED_POSITION;
      if(CAR(b) != R_MissingArg)
	m_resolved_args.at(i).is_missing = false;
      m_supplied_to_formal_map[b] = f;
      SET_ARGUSED(b, 1);
      b = CDR(b);
      f = CDR(f);
      i++;
    }
  }

  if (dots != -1) {
    /* Gobble up all unused actuals */
    j = 0;
    for (b = m_supplied; b != R_NilValue; b = CDR(b))
      if (!ARGUSED(b)) j++;

    if (j > 0) {
      MyArgT dummy_arg;
      m_resolved_args.insert(m_resolved_args.begin() + dots + 1, j, dummy_arg); // insert j empty args after dots
      m_lazy_info.insert(m_lazy_info.begin() + dots + 1, j, LAZY);
      i = dots + 1;
      for (b = m_supplied; b != R_NilValue; b = CDR(b)) {
	if (!ARGUSED(b)) {
	  m_resolved_args.at(i).cell = b;
	  m_resolved_args.at(i).formal = m_resolved_args.at(dots).formal;
	  m_resolved_args.at(i).source = RESOLVED_DOT;
	  m_resolved_args.at(i).is_missing = false;
	  m_supplied_to_formal_map[b] = m_resolved_args.at(dots).formal;
	  f = CDR(f);
	  i++;
	}
      }
    }
    m_resolved_args.erase(m_resolved_args.begin() + dots);  // remove dots argument
  }
  else {
    /* Check that all arguments are used */
    for (b = m_supplied; b != R_NilValue; b = CDR(b))
      if (!ARGUSED(b) && CAR(b) != R_MissingArg)
	Rf_errorcall(R_GlobalContext->call,
		     _("unused argument(s) (%s ...)"),
		     /* anything better when b is "untagged" ? : */
		     TAG(b) != R_NilValue ? var_name(TAG(b)).c_str() : "");
  }
}

int ResolvedArgs::size() {
  return m_resolved_args.size();
}

// ----- iterators -----

ResolvedArgs::const_iterator ResolvedArgs::begin() const {
  return m_resolved_args.begin();
}

ResolvedArgs::const_iterator ResolvedArgs::end() const {
  return m_resolved_args.end();
}

ResolvedArgs::const_reverse_iterator ResolvedArgs::rbegin() const {
  return m_resolved_args.rbegin();
}

ResolvedArgs::const_reverse_iterator ResolvedArgs::rend() const {
  return m_resolved_args.rend();
}

// ----- get_formal_for_actual -----

SEXP ResolvedArgs::get_formal_for_actual(const SEXP actual) const {
  std::map<SEXP, SEXP>::const_iterator it = m_supplied_to_formal_map.find(actual);
  if (it == m_supplied_to_formal_map.end()) {
    return 0;
  } else {
    return it->second;
  }
}

// ----- AnnotationMap methods -----

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

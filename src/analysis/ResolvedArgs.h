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

// File: ResolvedArgs.h
//
// Resolved actual arguments to a function. Generally speaking, does
// what matchArgs() does in R, only at compile time.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef RESOLVED_ARGS_H
#define RESOLVED_ARGS_H

#include <map>
#include <vector>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationBase.h>
#include <analysis/EagerLazy.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class ResolvedArgs : public AnnotationBase {
public:
  explicit ResolvedArgs(SEXP supplied, SEXP formals);
  virtual ~ResolvedArgs();

  /// source of resolved argument
  /// Example: f <- function(a, b, c=10, foo=13)
  /// f(11, a=12, fo=14)
  /// a is RESOLVED_TAG_EXACT, b is RESOLVED_POSITION, c is RESOLVED_DEFAULT, foo is RESOLVED_TAG_PARTIAL.
  enum ResolvedSource {
    RESOLVED_TAG_EXACT,   // matched by tag, e.g. f(a=12)
    RESOLVED_TAG_PARTIAL, // matched by tag, but not exactly, e.g. f(fo=13)
    RESOLVED_POSITION,    // matched by position, e.g. f(10)
    RESOLVED_DEFAULT,     // not supplied ("missing"), uses default argument
    RESOLVED_DOT          // this is "...". If an argument in
			  // m_resolved_args is this, then m_dot_args is
			  // the list of args that match the "...".
  };
  typedef struct {
    SEXP cell;
    ResolvedSource source;
    bool is_missing;
  } MyArgT;
  typedef std::vector<EagerLazyT> MyLazyInfoSetT;
  typedef std::vector<MyArgT> MyArgSetT;
  typedef MyArgSetT::const_iterator const_iterator;
  typedef MyArgSetT::const_reverse_iterator const_reverse_iterator;

  // indexed from 0
  EagerLazyT get_eager_lazy(int arg) const;
  void set_eager_lazy(int arg, EagerLazyT x);
  MyLazyInfoSetT get_lazy_info() const;

  const_iterator begin() const;
  const_iterator end() const;
  const_reverse_iterator rbegin() const;
  const_reverse_iterator rend() const;
  const_iterator begin_dot_args() const;
  const_iterator end_dot_args() const;
  const_reverse_iterator rbegin_dot_args() const;
  const_reverse_iterator rend_dot_args() const;

  AnnotationBase * clone();
  std::ostream & dump(std::ostream & stream) const;
  static PropertyHndlT handle();

private:
  void resolve();

  SEXP m_supplied;
  SEXP m_formals;
  MyLazyInfoSetT m_lazy_info;
  MyArgSetT m_resolved_args;
  MyArgSetT m_dot_args;      // arguments that match "..." in the function def
};

} // namespace RAnnot

#endif

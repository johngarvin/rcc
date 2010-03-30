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

// File: SimpleIterators.h
//
// Iterators for various SEXPs.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef SIMPLE_ITERATORS_H
#define SIMPLE_ITERATORS_H

#include <list>
#include <assert.h>

#include <include/R/R_RInternals.h>

/// Abstract class to enumerate a set of R expressions.
template<class T> class R_Iterator {
public:
  virtual ~R_Iterator() { }
  
  virtual T current() const = 0;
  virtual bool isValid() const = 0;
  virtual void operator++() = 0;
  void operator++(int) { ++*this; }
  virtual void reset() = 0;
};

typedef R_Iterator<SEXP> R_SEXPIterator;

/// Singleton iterator for a single statement.
template<class T> class R_SingletonIterator : public R_Iterator<T> {
public:
  explicit R_SingletonIterator(T t);

  virtual T current() const;
  virtual bool isValid() const;
  virtual void operator++();
  virtual void reset();
private:
  const T m_t;
  bool m_valid;
};

typedef R_SingletonIterator<SEXP> R_SingletonSEXPIterator;

/// Enumerate the elements of a list (in R, a chain of CONS cells).
/// To make sure all locations are unique, the iterator gives you the
/// cons cell; take the CAR to get the data you want.
class R_ListIterator : public R_SEXPIterator {
public:
  explicit R_ListIterator(SEXP _exp);
  virtual ~R_ListIterator();
  
  SEXP current() const;
  bool isValid() const;
  void operator++();       // prefix
  void operator++(int);    // postfix
  void reset();
private:
  const SEXP m_exp;
  SEXP m_curr;
};

class R_CallArgsIterator : public R_SEXPIterator {
public:
  explicit R_CallArgsIterator(SEXP _call);
  virtual ~R_CallArgsIterator();

  SEXP current() const;
  bool isValid() const;
  void operator++();       // prefix
  void operator++(int);    // postfix
  void reset();
private:
  const SEXP m_call;
  SEXP m_curr;
};

/// preorder traversal of an R object through CARs and CDRs
class R_PreorderIterator {
public:
  explicit R_PreorderIterator(SEXP exp);
  virtual ~R_PreorderIterator();
  
  SEXP current() const;
  bool isValid() const;
  void operator++();
  void reset();
private:
  void build_pre(SEXP e);

  std::list<SEXP> m_preorder;
  std::list<SEXP>::const_iterator m_iter;
  const SEXP m_exp;
  SEXP m_curr;
};

#endif

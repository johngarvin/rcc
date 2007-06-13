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

#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>

#include "SimpleIterators.h"

//--------------------------------------------------------------------
// R_SingletonIterator methods
//--------------------------------------------------------------------

template<class T> R_SingletonIterator<T>::R_SingletonIterator(T t) : m_t(t), m_valid(true)
{
}

template<class T> T R_SingletonIterator<T>::current() const {
  return m_t;
}

template<class T> bool R_SingletonIterator<T>::isValid() const {
  return m_valid;
}

template<class T> void R_SingletonIterator<T>::operator++() { 
  m_valid = false;
}

template<class T> void R_SingletonIterator<T>::reset() { 
  m_valid = true;
}

//--------------------------------------------------------------------
// R_ListIterator methods
//--------------------------------------------------------------------

SEXP R_ListIterator::current() const {
  return curr;
}

bool R_ListIterator::isValid() const {
  return (curr != R_NilValue);
}

void R_ListIterator::operator++() {
  // must be a data or language cons cell to be able to take the CDR
  assert(TYPEOF(curr) == LISTSXP || TYPEOF(curr) == LANGSXP);
  curr = CDR(curr);
}

void R_ListIterator::reset() {
  curr = exp;
}

//--------------------------------------------------------------------
// R_PreorderIterator methods
//--------------------------------------------------------------------

SEXP R_PreorderIterator::current() const {
  return *iter;
}
  
bool R_PreorderIterator::isValid() const {
  return (iter != preorder.end());
}

void R_PreorderIterator::operator++() {
  ++iter; 
}

void R_PreorderIterator::reset() {
  iter = preorder.begin();
}

/// Preorder iterator: building the preorder list of nodes
void R_PreorderIterator::build_pre(SEXP e) {
  assert(e != NULL);
  switch(TYPEOF(e)) {
  case NILSXP:
    return;
    break;
  case LISTSXP:
  case LANGSXP:
    preorder.push_back(e);
    build_pre(CAR(e));
    build_pre(CDR(e));
    break;
  default:
    preorder.push_back(e);
    break;
  }
}

//------------------------------------------------------------
// template instantiation
//------------------------------------------------------------

template class R_SingletonIterator<SEXP>;
template class R_SingletonIterator<OA::OA_ptr<OA::MemRefExpr> >;

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

// File: VarRefSet.cc
//
// Set of VarRefs and iterator for VarRefs.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/Utils/OA_ptr.hpp>

#include <analysis/SimpleIterators.h>
#include <analysis/VarRefFactory.h>

#include "VarRefSet.h"

using namespace OA;

//--------------------------------------------------------------------
// R_VarRefSetIterator methods
//--------------------------------------------------------------------

R_VarRefSetIterator::R_VarRefSetIterator(OA_ptr<MySetT> vars)
  : m_vars(vars)
{
  assert(!m_vars.ptrEqual(0));
  m_iter = m_vars->begin();
}

OA_ptr<R_VarRef> R_VarRefSetIterator::current() const {
  return *m_iter;
}

bool R_VarRefSetIterator::isValid() const {
  return (m_iter != m_vars->end());
}

void R_VarRefSetIterator::operator++() {
  ++m_iter;
}

void R_VarRefSetIterator::reset() {
  m_iter = m_vars->begin();
}

//--------------------------------------------------------------------
// R_VarRefSet methods
//--------------------------------------------------------------------

R_VarRefSet::R_VarRefSet() {
  m_vars = new MySetT;
}

void R_VarRefSet::insert_ref(OA_ptr<R_BodyVarRef> var) {
  OA_ptr<R_VarRef> upcast_var;
  upcast_var = var.convert<R_VarRef>();
  m_vars->insert(upcast_var);
}

void R_VarRefSet::insert_arg(OA_ptr<R_ArgVarRef> var) {
  OA_ptr<R_VarRef> upcast_var;
  upcast_var = var.convert<R_VarRef>();
  m_vars->insert(upcast_var);
}

void R_VarRefSet::set_union(const R_VarRefSet & set2) {
  OA_ptr<R_VarRefSetIterator> it; it = set2.get_iterator();
  for ( ; it->isValid(); ++*it) {
    m_vars->insert(it->current());
  }
}

void R_VarRefSet::set_union(OA_ptr<R_VarRefSet> set2) {
  OA_ptr<R_VarRefSetIterator> it; it = set2->get_iterator();
  for ( ; it->isValid(); ++*it) {
    m_vars->insert(it->current());
  }
}

OA_ptr<R_VarRefSetIterator> R_VarRefSet::get_iterator() const {
  OA_ptr<R_VarRefSetIterator> it;
  it = new R_VarRefSetIterator(m_vars);
  return it;
}

/// Given a list of formal arguments, construct and return a VarRefSet
/// of corresponding ArgRefs
OA_ptr<R_VarRefSet> R_VarRefSet::refs_from_arglist(SEXP arglist) {
  VarRefFactory * fact = VarRefFactory::get_instance();
  OA_ptr<R_VarRefSet> refs; refs = new R_VarRefSet;
  R_ListIterator iter(arglist);
  for( ; iter.isValid(); ++iter) {
    OA_ptr<R_ArgVarRef> arg;
    arg = fact->make_arg_var_ref(iter.current());
    refs->insert_arg(arg);
  }
  return refs;
}

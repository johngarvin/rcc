// -*- Mode: C++ -*-
//
// Copyright (c) 2007 Rice University
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

// File: VarRefFactory.cc
//
// Factory for VarRefs; allows VarRefs to be either created or reused
// if they have already created.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "VarRefFactory.h"

using namespace OA;

OA_ptr<R_BodyVarRef> VarRefFactory::make_body_var_ref(SEXP e) {
  OA_ptr<R_BodyVarRef> ref;
  std::map<SEXP, OA_ptr<R_BodyVarRef> >::const_iterator iter = m_body_refs.find(e);
  if (iter == m_body_refs.end()) {
    ref = new R_BodyVarRef(e);
    m_body_refs[e] = ref;
  } else {
    ref = iter->second;
  }
  return ref;
}

OA_ptr<R_ArgVarRef> VarRefFactory::make_arg_var_ref(SEXP e) {
  OA_ptr<R_ArgVarRef> ref;
  std::map<SEXP, OA_ptr<R_ArgVarRef> >::const_iterator iter = m_arg_refs.find(e);
  if (iter == m_arg_refs.end()) {
    ref = new R_ArgVarRef(e);
    m_arg_refs[e] = ref;
  } else {
    ref = iter->second;
  }
  return ref;
}

VarRefFactory * VarRefFactory::instance() {
  if (s_instance == 0) {
    s_instance = new VarRefFactory();
  }
  return s_instance;
}

VarRefFactory::VarRefFactory() {
}

VarRefFactory * VarRefFactory::s_instance = 0;

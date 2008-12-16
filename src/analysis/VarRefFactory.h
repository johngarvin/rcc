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

// File: VarRefFactory.h
//
// Factory for VarRefs; allows VarRefs to be either created or reused
// if they have already created (Flyweight pattern).
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef VAR_REF_FACTORY_H
#define VAR_REF_FACTORY_H

#include <map>

#include <OpenAnalysis/Utils/OA_ptr.hpp>

#include <include/R/R_RInternals.h>

#include <analysis/VarRef.h>

class VarRefFactory {
public:
  OA::OA_ptr<R_BodyVarRef> make_body_var_ref(SEXP e);
  OA::OA_ptr<R_ArgVarRef> make_arg_var_ref(SEXP e);

  static VarRefFactory * get_instance();

private:
  // singleton pattern
  explicit VarRefFactory();
  static VarRefFactory * s_instance;

  std::map<SEXP, OA::OA_ptr<R_BodyVarRef> > m_body_refs;
  std::map<SEXP, OA::OA_ptr<R_ArgVarRef> > m_arg_refs;
};

#endif

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

// File: MemRefExprInterface.h
//
// Interface for creating OA MemRefExprs out of SEXPs.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef MEM_REF_EXPR_INTERFACE_H
#define MEM_REF_EXPR_INTERFACE_H

#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <include/R/R_RInternals.h>

class MemRefExprInterface {
  
public:
  static OA::OA_ptr<OA::MemRefExpr> convert_sexp_c(SEXP cell);
  static OA::OA_ptr<OA::MemRefExpr> convert_mem_ref_handle(OA::MemRefHandle mrh);
  static OA::OA_ptr<OA::MemRefExpr> convert_sym_handle(OA::SymHandle sh);
};

#endif

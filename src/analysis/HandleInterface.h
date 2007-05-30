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

// File: HandleInterface.h
//
// Conversions back and forth between OA handles and SEXPs. Intended
// to put all the reinterpret_casts in one place.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef HANDLE_INTERFACE_H
#define HANDLE_INTERFACE_H

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

/// Provide an interface between IRHandles and SEXPs. This will
/// improve typing ability and provide a single point of control for
/// the casts necessary.
namespace HandleInterface {

OA::ProcHandle make_proc_h(SEXP e);
OA::MemRefHandle make_mem_ref_h(SEXP e);
OA::StmtHandle make_stmt_h(SEXP e);
OA::LeafHandle make_leaf_h(SEXP e);
OA::SymHandle make_sym_h(SEXP e);
OA::CallHandle make_call_h(SEXP e);
 
SEXP make_sexp(OA::IRHandle h);
SEXP make_sexp(OA::ProcHandle h);
SEXP make_sexp(OA::MemRefHandle h);
SEXP make_sexp(OA::StmtHandle h);
SEXP make_sexp(OA::LeafHandle h);
SEXP make_sexp(OA::SymHandle h);

}

#endif

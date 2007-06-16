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

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <analysis/Utils.h>

#include "HandleInterface.h"

namespace HandleInterface {

// ----- forward declarations -----

static OA::irhandle_t as_handle(const SEXP e);
static const SEXP as_sexp(const OA::irhandle_t h);

// interface functions

OA::ProcHandle make_proc_h(const SEXP e) {
  assert(is_fundef(e));
  return OA::ProcHandle(as_handle(e));
}

OA::SymHandle make_sym_h(const SEXP e) {
  assert(is_var(e));
  return OA::SymHandle(as_handle(e));
}

OA::ExprHandle make_expr_h(const SEXP e) {
  return OA::ExprHandle(as_handle(e));
}

OA::MemRefHandle make_mem_ref_h(const SEXP e) {
  return OA::MemRefHandle(as_handle(e));
}

OA::CallHandle make_call_h(const SEXP e) {
  assert(is_call(e));
  return OA::CallHandle(as_handle(e));
}

OA::OpHandle make_op_h(const SEXP e) {
  return OA::OpHandle(as_handle(e));
}
  
OA::ConstSymHandle make_const_sym_h(const SEXP e) {
  assert(is_var(e));
  return OA::ConstSymHandle(as_handle(e));
}

OA::ConstValHandle make_const_val_h(const SEXP e) {
  assert(is_const(e));
  return OA::ConstValHandle(as_handle(e));
}

OA::StmtHandle make_stmt_h(const SEXP e) {
  return OA::StmtHandle(as_handle(e));
}

OA::LeafHandle make_leaf_h(const SEXP e) {
  return OA::LeafHandle(as_handle(e));
}

const SEXP make_sexp(OA::IRHandle h)       { return as_sexp(h.hval()); }
const SEXP make_sexp(OA::ProcHandle h)     { return as_sexp(h.hval()); }
const SEXP make_sexp(OA::SymHandle h)      { return as_sexp(h.hval()); }
const SEXP make_sexp(OA::ExprHandle h)     { return as_sexp(h.hval()); }
const SEXP make_sexp(OA::MemRefHandle h)   { return as_sexp(h.hval()); }
const SEXP make_sexp(OA::CallHandle h)     { return as_sexp(h.hval()); }
const SEXP make_sexp(OA::OpHandle h)       { return as_sexp(h.hval()); }
const SEXP make_sexp(OA::ConstSymHandle h) { return as_sexp(h.hval()); }
const SEXP make_sexp(OA::ConstValHandle h) { return as_sexp(h.hval()); }
const SEXP make_sexp(OA::StmtHandle h)     { return as_sexp(h.hval()); }
const SEXP make_sexp(OA::LeafHandle h)     { return as_sexp(h.hval()); }

// ----- local conversion functions -----

// reinterpret SEXP as an irhandle_t.
static OA::irhandle_t as_handle(const SEXP e) {
  return reinterpret_cast<OA::irhandle_t>(e);
}

// reinterpret irhandle_t as SEXP
static const SEXP as_sexp(const OA::irhandle_t h) {
  return reinterpret_cast<SEXP>(h);
}

}

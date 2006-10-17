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

// ----- forward declarations -----

static OA::irhandle_t as_handle(SEXP e);
static SEXP as_sexp(OA::irhandle_t h);

// interface functions

OA::ProcHandle HandleInterface::make_proc_h(SEXP e) {
  assert(is_fundef(e));
  return OA::ProcHandle(as_handle(e));
}

OA::MemRefHandle HandleInterface::make_mem_ref_h(SEXP e) {
  return OA::MemRefHandle(as_handle(e));
}

OA::StmtHandle HandleInterface::make_stmt_h(SEXP e) {
  return OA::StmtHandle(as_handle(e));
}

OA::LeafHandle HandleInterface::make_leaf_h(SEXP e) {
  return OA::LeafHandle(as_handle(e));
}

OA::SymHandle HandleInterface::make_sym_h(SEXP e) {
  return OA::SymHandle(as_handle(e));
}

// file static:
// reinterpret SEXP as an irhandle_t.
static OA::irhandle_t as_handle(SEXP e) {
  return reinterpret_cast<OA::irhandle_t>(e);
}

SEXP HandleInterface::make_sexp(OA::IRHandle h)     { return as_sexp(h.hval()); }
SEXP HandleInterface::make_sexp(OA::ProcHandle h)   { return as_sexp(h.hval()); }
SEXP HandleInterface::make_sexp(OA::MemRefHandle h) { return as_sexp(h.hval()); }
SEXP HandleInterface::make_sexp(OA::StmtHandle h)   { return as_sexp(h.hval()); }
SEXP HandleInterface::make_sexp(OA::LeafHandle h)   { return as_sexp(h.hval()); }
SEXP HandleInterface::make_sexp(OA::SymHandle h)    { return as_sexp(h.hval()); }

// file static:
// reinterpret irhandle_t as SEXP
static SEXP as_sexp(OA::irhandle_t h) {
  return reinterpret_cast<SEXP>(h);
}

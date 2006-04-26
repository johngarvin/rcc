#include "HandleInterface.h"

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <analysis/Utils.h>

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

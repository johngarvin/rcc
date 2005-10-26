#include "HandleInterface.h"

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <analysis/Utils.h>

static OA::irhandle_t as_handle(SEXP e);

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

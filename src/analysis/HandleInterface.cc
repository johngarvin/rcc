#include "HandleInterface.h"

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <analysis/Utils.h>

OA::ProcHandle HandleInterface::make_proc_h(SEXP e) {
  assert(is_fundef(e));
  OA::irhandle_t handle = reinterpret_cast<OA::irhandle_t>(e);
  return OA::ProcHandle(handle);
}

OA::MemRefHandle HandleInterface::make_mem_ref_h(SEXP e) {
  OA::irhandle_t handle = reinterpret_cast<OA::irhandle_t>(e);
  return OA::MemRefHandle(handle);
}


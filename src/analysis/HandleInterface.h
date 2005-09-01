// -*- Mode: C++ -*-

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

//! Provide an interface between IRHandles and SEXPs. This will
//! improve typing ability and provide a single point of control for
//! the casts necessary.
class HandleInterface {
public:
  static OA::ProcHandle make_proc_h(SEXP e);
  static OA::MemRefHandle make_mem_ref_h(SEXP e);
};

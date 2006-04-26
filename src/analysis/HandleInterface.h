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
  static OA::StmtHandle make_stmt_h(SEXP e);
  static OA::LeafHandle make_leaf_h(SEXP e);
  static OA::SymHandle make_sym_h(SEXP e);
 
  static SEXP make_sexp(OA::IRHandle h);
  static SEXP make_sexp(OA::ProcHandle h);
  static SEXP make_sexp(OA::MemRefHandle h);
  static SEXP make_sexp(OA::StmtHandle h);
  static SEXP make_sexp(OA::LeafHandle h);
  static SEXP make_sexp(OA::SymHandle h);
};

// -*-Mode: C++;-*-
#ifndef ANALYST_H
#define ANALYST_H

// includes

#include <set>
#include <map>
#include <ostream>

#include <include/R/R_RInternals.h>

#include <analysis/IRInterface.h>
#include <analysis/AnnotationSet.h>
#include <analysis/ScopeTree.h>
#include <analysis/VarRefSet.h>     // FIXME: delete when ExpUDInfo disappears

// forward declarations

// local
class R_ExpUDLocInfo;

// from OA
class OA::ProcHandle;

// from Annotation.h
class RAnnot::FuncInfo;
class RAnnot::Var;

// from VarRef.h
//class R_VarRef;
//class R_VarRefSet;
//class R_VarRefSetIterator;

//! R_Analyst
//! 
//! Manager for annotations. Given requests for information, perform
//! the analysis or return data already stored.  Uses Singleton
//! pattern: we want only one analyst for our program.

class R_Analyst {
public:
  //! construct an R_Analyst by providing an SEXP representing the whole program
  R_Analyst(SEXP _program);

  //! methods to get information

  // scope tree
  RAnnot::FuncInfo *get_scope_tree_root();

  const R_ExpUDLocInfo & get_local_info(SEXP statement) const;

  //! methods to spit out debugging information
  void dump_scope_tree(std::ostream &);
  void dump_cfg(std::ostream &, SEXP proc);
  void dump_all_cfgs(std::ostream &);

private:
  SEXP m_program;
  OA::OA_ptr<R_IRInterface> m_interface;

  RAnnot::FuncInfo *m_scope_tree_root;

  // Annotation sets
  //  RAnnot::AnnotationSet m_func_info; // maps each scope (ProcHandle) to a (ptr to a) FuncInfo annotation
  //  RAnnot::AnnotationSet m_var_info;  // maps each VarRef (MemRefHandle) to Var annotation
  
  void build_scope_tree(SEXP e, RAnnot::FuncInfo *parent);

  void build_cfgs();
};

//! Contains basic use and def information about the given
//! SEXP. Traverses the expression in the constructor to build the
//! sets. What we have here is only syntactic information: whether defs
//! are local or free, and whether uses are applications or not.
//! Stores locations instead of just names.
//! Uses and defs must be mutually exclusive for different
//! statements--that is, in a compound statement only the variables
//! directly used or modified count, not those found in the loop body.
class R_ExpUDLocInfo {
 public:
  R_ExpUDLocInfo(SEXP e) : exp(e) {
    local_defs = new R_VarRefSet();
    free_defs = new R_VarRefSet();
    app_uses = new R_VarRefSet();
    non_app_uses = new R_VarRefSet();
    build_ud_rhs(e);
  }
  OA::OA_ptr<R_VarRefSet> get_local_defs() { return local_defs; }
  OA::OA_ptr<R_VarRefSet> get_free_defs() { return free_defs; }
  OA::OA_ptr<R_VarRefSet> get_app_uses() { return app_uses; }
  OA::OA_ptr<R_VarRefSet> get_non_app_uses() { return non_app_uses; }
private:
  typedef enum {LOCAL, FREE} local_pred;
  SEXP exp;
  void build_ud_rhs(const SEXP e);
  void build_ud_lhs(const SEXP e, local_pred is_local);
  OA::OA_ptr<R_VarRefSet> local_defs;
  OA::OA_ptr<R_VarRefSet> free_defs;
  OA::OA_ptr<R_VarRefSet> app_uses;
  OA::OA_ptr<R_VarRefSet> non_app_uses;
};

#endif

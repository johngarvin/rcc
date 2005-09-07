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
#include <analysis/VarRefSet.h>     // FIXME: delete when ExpUDInfo disappears

// forward declarations

// local
class R_ExpUDLocInfo;

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
//! the analysis or return data already stored.

class R_Analyst {
public:
  //! construct an R_Analyst by providing an SEXP representing the whole program
  R_Analyst(SEXP _program);

  //! methods to get information
  OA::OA_ptr<R_IRInterface> get_interface() { return m_interface; }

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

  void build_cfgs();
  void build_use_def_info();
  void build_local_variable_info();
};

#endif

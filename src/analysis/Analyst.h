// -*-Mode: C++;-*-
#ifndef ANALYST_H
#define ANALYST_H

// includes

#include <ostream>

#include <include/R/R_RInternals.h>

#include <analysis/IRInterface.h>

// forward declarations

// from Annotation.h

namespace RAnnot {
class FuncInfo;
class Var;
}

//! R_Analyst
//! 
//! Singleton manager for annotations.

class R_Analyst {

  // implement Singleton pattern
public:
  static R_Analyst * get_instance(SEXP _program); // regular Singleton: construct or return
  static R_Analyst * get_instance();  // only get the existing instance; error if not instantiated

public:
  //! Perform analysis. Return true if analysis was successful, false otherwise.
  bool perform_analysis();

  //! methods to get information
  OA::OA_ptr<R_IRInterface> get_interface() { return m_interface; }

  // scope tree
  RAnnot::FuncInfo *get_scope_tree_root();

  //! methods to spit out debugging information
  void dump_scope_tree(std::ostream &);

  void dump_cfg(std::ostream &, SEXP proc);
  void dump_all_cfgs(std::ostream &);

protected:
  //! construct an R_Analyst by providing an SEXP representing the whole program
  R_Analyst(SEXP _program);

private:
  static R_Analyst * m_instance;

private:
  SEXP m_program;
  OA::OA_ptr<R_IRInterface> m_interface;

  RAnnot::FuncInfo *m_scope_tree_root;

  void build_cfgs();
  void build_locality_info();
  void build_local_variable_info();
  void build_local_function_info();
  void build_bindings();
  void build_call_graph();
};

#endif

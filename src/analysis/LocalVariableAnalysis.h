// -*-Mode: C++;-*-
#ifndef LOCAL_VARIABLE_ANALYSIS_H
#define LOCAL_VARIABLE_ANALYSIS_H

#include <vector>

#include <include/R/R_RInternals.h>

#include <analysis/Var.h>

//! LocalVariableAnalysis 
//! 
//! Given an SEXP, traverses the expression to discover basic use and
//! def information. What we have here is only syntactic information:
//! whether defs are local or free, and whether uses are applications
//! or not. Stores locations instead of just names. Since this is used
//! by later passes that traverse the CFG, the analysis must be
//! confined to the current CFG node. This means that in a loop
//! statement, the only variables that count are those directly used
//! or modified, not those found in the loop body.

class LocalVariableAnalysis {
public:
  typedef RAnnot::Var * MyVarT;
  typedef std::list<MyVarT>::const_iterator const_var_iterator;
  typedef SEXP MyCallSiteT;
  typedef std::list<MyCallSiteT>::const_iterator const_call_site_iterator;

  LocalVariableAnalysis(const SEXP _stmt);

  void perform_analysis();

  const_var_iterator begin_vars() const;
  const_var_iterator end_vars() const;
  const_call_site_iterator begin_call_sites() const;
  const_call_site_iterator end_call_sites() const;

private:
  enum LhsType {
    IN_LOCAL_ASSIGN,
    IN_FREE_ASSIGN
  };
  void build_ud_rhs(const SEXP cell);
  void build_ud_lhs(const SEXP cell, const SEXP rhs, RAnnot::Var::MayMustT may_must_type, LhsType lhs_type);
  const SEXP m_stmt;
  std::list<MyVarT> m_vars;
  std::list<MyCallSiteT> m_call_sites;
};

#endif

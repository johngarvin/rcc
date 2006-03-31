// -*-Mode: C++;-*-
#ifndef LOCAL_VARIABLE_ANALYSIS_H
#define LOCAL_VARIABLE_ANALYSIS_H

#include <vector>

#include <include/R/R_RInternals.h>

#include <analysis/Annotation.h>

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
  typedef RAnnot::Var * MyKeyT;
  typedef std::vector<MyKeyT>::const_iterator const_iterator;

  LocalVariableAnalysis(const SEXP _stmt);

  void perform_analysis();

  const_iterator begin() const;
  const_iterator end() const;

private:
  enum LhsType {
    IN_LOCAL_ASSIGN,
    IN_FREE_ASSIGN
  };
  void build_ud_rhs(const SEXP cell);
  void build_ud_lhs(const SEXP cell, const SEXP rhs, RAnnot::Var::MayMustT may_must_type, LhsType lhs_type);
  const SEXP m_stmt;
  std::vector<MyKeyT> m_vars;
};

#endif

// -*-Mode: C++;-*-
#ifndef LOCAL_VARIABLE_ANALYSIS_H
#define LOCAL_VARIABLE_ANALYSIS_H

#include <include/R/R_RInternals.h>

//! LocalVariableAnalysis 
//! 
//! Discovers basic use and def information about the given SEXP and
//! adds the corresponding annotations to analysisResults. Traverses
//! the expression in the constructor to build the sets. What we have
//! here is only syntactic information: whether defs are local or
//! free, and whether uses are applications or not.  Stores locations
//! instead of just names. Since this is used by later passes that
//! traverse the CFG, the analysis must be confined to the current CFG
//! node. This means that in a loop statement, the only variables that
//! count are those directly used or modified, not those found in the
//! loop body.
class LocalVariableAnalysis {
 public:
  LocalVariableAnalysis(const SEXP _stmt);
  ~LocalVariableAnalysis();

  void perform_analysis();
private:
  enum LhsType {
    IN_LOCAL_ASSIGN,
    IN_FREE_ASSIGN
  };
  void build_ud_rhs(const SEXP cell);
  void build_ud_lhs(const SEXP cell, RAnnot::Var::MayMustT may_must_type, LhsType lhs_type);
  const SEXP m_stmt;
  RAnnot::ExpressionInfo * m_stmt_annot;
};

#endif

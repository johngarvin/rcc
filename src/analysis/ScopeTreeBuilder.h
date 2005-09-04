// -*- mode: C++ -*-

#ifndef SCOPE_TREE_BUILDER_H
#define SCOPE_TREE_BUILDER_H

#include <include/R/R_RInternals.h>
#include <analysis/Annotation.h>

class ScopeTreeBuilder {
public:
  static RAnnot::FuncInfo * build_scope_tree_with_given_root(SEXP r_root);
private:
  static void build_scope_tree(SEXP e, RAnnot::FuncInfo *parent);
};

#endif

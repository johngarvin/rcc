#ifndef SCOPE_TREE_H
#define SCOPE_TREE_H

class ScopeTreeBuilder {
private:
  tree<SEXP> data;
public:
  ScopeTree();
  tree<SEXP> build(SEXP e);
};

#endif

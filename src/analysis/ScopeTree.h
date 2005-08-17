class ScopeTreeBuilder {
private:
  tree<SEXP> data;
public:
  ScopeTree();
  tree<SEXP> build(SEXP e);
};

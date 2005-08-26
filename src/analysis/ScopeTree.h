// -*- mode: C++ -*-
#ifndef SCOPE_TREE_H
#define SCOPE_TREE_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <PeetersTree/tree.hh>

class RFunctionScopeInfo {
public:
  RFunctionScopeInfo(SEXP _name, SEXP _defn) : name(_name), defn(_defn) {};
  SEXP get_args() { return CAR(fundef_args_c(defn)); };
  SEXP get_defn() { return defn; };
private:
  SEXP name;
  SEXP defn;
};

typedef tree<OA::OA_ptr<RFunctionScopeInfo> > ScopeTreeImp;

typedef tree<RFunctionScopeInfo *> RScopeTree;
// TODO: change users of RScopeTree to use ScopeTree instead

class PreOrderScopeTreeIterator {
public:
  PreOrderScopeTreeIterator(OA::OA_ptr<ScopeTreeImp> _tree) : tree(_tree) {
    assert(!tree.ptrEqual(NULL)); it = tree->begin();
  }
  OA::OA_ptr<RFunctionScopeInfo> current() const { return *it; }
  bool isValid() const { return (it != tree->end()); }
  void operator++() { ++it; }
  void reset() { it = tree->begin(); }

private:
  OA::OA_ptr<ScopeTreeImp> tree;
  ScopeTreeImp::iterator it;
};

class PostOrderScopeTreeIterator {
};

//! Handle for a specific scope within a scope tree
class ScopeTreePtr {
private:
  ScopeTreeImp::iterator it;
};

//! The tree of enclosing scopes (function definitions)
class ScopeTree {
public:
  OA::OA_ptr<PreOrderScopeTreeIterator> get_pre_iterator();
  ScopeTreePtr get_root();
  void insert_scope(const ScopeTreePtr &);
  void get_parent(const ScopeTreePtr &);

private:
  ScopeTreeImp st;

  friend class PreOrderScopeTreeIterator;
};

#endif

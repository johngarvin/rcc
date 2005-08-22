// -*-Mode: C++;-*-
#ifndef ANALYST_H
#define ANALYST_H

#include <set>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <PeetersTree/tree.hh>

#include <MyRInternals.h>
#include "Utils.h"
#include "SimpleIterators.h"

class RFunctionScopeInfo {
public:
  RFunctionScopeInfo(SEXP _name, SEXP _defn) : name(_name), defn(_defn) {};
  SEXP get_args() { return CAR(fundef_args_c(defn)); };
  SEXP get_defn() { return defn; };
private:
  SEXP name;
  SEXP defn;
};

typedef tree<RFunctionScopeInfo *> RScopeTree;

class ScopeTree;

class ScopeTreeIterator {
private:
  OA::OA_ptr<ScopeTree> tree;
};

class ScopeTree {
public:
  OA::OA_ptr<ScopeTreeIterator> get_iterator();
  void insert_scope(const RFunctionScopeInfo &);
  void get_parent(const RFunctionScopeInfo &);
private:
  tree<RFunctionScopeInfo *> st;

  friend class ScopeTreeIterator;
};

//! R_Analyst
//! Contains an entire R program along with the results of analysis.
class R_Analyst {
private:
  SEXP exp;
  OA::OA_ptr<RScopeTree> scope_tree;
  void build_scope_tree_rec(SEXP e, OA::OA_ptr<RScopeTree> t, RScopeTree::iterator &curr);
  // map<SEXP, CFG> cfg_info;  // CFG of each procedure
  // map<CFG::Node, R_ExpIterator> // List of simple statements for each CFG node
  // map<SEXP, R_ExpUDInfo> stmt_info;                  // maps statements to variable information
  // map<RScopeTree::iterator, vector<SEXP> > scope_stmts; // maps scopes to list of statements

public:
  R_Analyst(SEXP e) : exp(e) {}
  OA::OA_ptr<RScopeTree> get_scope_tree();
  // TODO: memoize results from UDLocInfo
  //  const set<SEXP> & get_local_defs(const SEXP e);
  //  const set<SEXP> & get_free_defs(const SEXP e);
  //  const set<SEXP> & get_app_uses(const SEXP e);
  //  const set<SEXP> & get_non_app_uses(const SEXP e);
  //  set<SEXP>::iterator get_defs(SEXP e);
  //  set<SEXP>::iterator get_uses(SEXP e);
};

//! R_VarRef
//! An R_VarRef represents a mention (use or def) of a variable in R
//! code. Designed to be compared in different ways: two different
//! locations with the same name are "==" but not "equiv".  The name of
//! the mention is a SYMSXP in the R AST. Since all SYMSXPs with the
//! same name are the same pointer, we differentiate refs by
//! representing an R_VarRef as a CONS cell containing the symbol.
//! This is a base class: the way to get the name will be different
//! depending on what kind of reference it is.
class R_VarRef {
public:
  virtual ~R_VarRef() {};

  // Comparison operators for use in STL containers
  virtual bool operator<(R_VarRef & loc2) const = 0;
  virtual bool operator<(const R_VarRef & loc2) const = 0;
  virtual bool operator==(R_VarRef & loc2) const = 0;

  virtual bool equiv(R_VarRef & loc2) const = 0;

  virtual std::string toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir) const = 0;
  virtual std::string toString() const = 0;

  virtual SEXP get_sexp() const = 0;
  virtual SEXP get_name() const = 0;
};

//! R_BodyVarRef
//! A VarRef found in code. A CONS cell where the CAR is the symbol.
class R_BodyVarRef : public R_VarRef {
public:
  R_BodyVarRef(SEXP _loc) : m_loc(_loc) {}
  
  // Comparison operators for use in STL containers
  bool operator<(R_VarRef & loc2) const;
  bool operator<(const R_VarRef & loc2) const;
  bool operator==(R_VarRef & loc2) const;

  bool equiv(R_VarRef & loc2) const;

  std::string toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir) const;
  std::string toString() const;

  SEXP get_sexp() const;
  SEXP get_name() const;
  
private:
  SEXP m_loc;
};

//! R_ArgVarRef
//! A VarRef found as a formal argument. A CONS cell where the TAG is the symbol.
class R_ArgVarRef : public R_VarRef {
public:
  R_ArgVarRef(SEXP _loc) : m_loc(_loc) {}
  
  // Comparison operators for use in STL containers
  bool operator<(R_VarRef & loc2) const;
  bool operator<(const R_VarRef & loc2) const;
  bool operator==(R_VarRef & loc2) const;

  bool equiv(R_VarRef & loc2) const;

  std::string toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir) const;
  std::string toString() const;

  SEXP get_sexp() const;
  SEXP get_name() const;

private:
  SEXP m_loc;
};

class R_VarRefSetIterator {
public:
  R_VarRefSetIterator(OA::OA_ptr<std::set<OA::OA_ptr<R_VarRef> > > _vars) : vars(_vars) {
    assert(!vars.ptrEqual(NULL));
    it = vars->begin();
  }
  OA::OA_ptr<R_VarRef> current() const { return *it; }
  bool isValid() const { return (it != vars->end());}
  void operator++() { ++it; }
  void reset() { it = vars->begin(); }

private:
  OA::OA_ptr<std::set<OA::OA_ptr<R_VarRef> > > vars;
  std::set<OA::OA_ptr<R_VarRef> >::iterator it;
};


//! A set of R_VarRefs.
class R_VarRefSet {
 public:
  R_VarRefSet() { vars = new std::set<OA::OA_ptr<R_VarRef> >; }
  void insert_ref(const OA::OA_ptr<R_BodyVarRef> var);
  void insert_arg(const OA::OA_ptr<R_ArgVarRef> var);
  void set_union(const R_VarRefSet & set2);
  void set_union(OA::OA_ptr<R_VarRefSet> set2);
  //void remove(SEXP var);

  OA::OA_ptr<R_VarRefSetIterator> get_iterator() const {
    OA::OA_ptr<R_VarRefSetIterator> it;
    it = new R_VarRefSetIterator(vars);
    return it;
  }

  void dump();

protected:
  friend class VarRefSetIterator;

  OA::OA_ptr<std::set<OA::OA_ptr<R_VarRef> > > vars;
};

//! construct a VarRefSet from a list of formal arguments
OA::OA_ptr<R_VarRefSet> refs_from_arglist(SEXP arglist);

//! Iterator over a VarSet.
//class VarSetIterator : public R_ExpIterator {
class VarSetIterator {
public:
  VarSetIterator(OA::OA_ptr<std::set<SEXP> > _vars) : vars(_vars) {
    assert(!vars.ptrEqual(NULL));
    it = vars->begin();
  }
  SEXP current() const { return *it; }
  bool isValid() const { return (it != vars->end());}
  void operator++() { ++it; }
  void reset() { it = vars->begin(); }
private:
  OA::OA_ptr<std::set<SEXP> > vars;
  std::set<SEXP>::iterator it;
};

//! A set of variable names (SYMSXPs).
class VarSet {
public:
  VarSet() { vars = new std::set<SEXP>(); }
  void insert(const SEXP var);
  void set_union(const VarSet & set2);
  void set_union(OA::OA_ptr<VarSet> set2);
  void set_union(OA::OA_ptr<std::set<R_VarRef> > set2);
  //void remove(SEXP var);

  OA::OA_ptr<VarSetIterator> get_iterator() const {
    OA::OA_ptr<VarSetIterator> it;
    it = new VarSetIterator(vars);
    return it;
  }

  void dump();

protected:
  friend class VarSetIterator;

  OA::OA_ptr<std::set<SEXP> > vars;
};

//! Contains basic use and def information about the given
//! SEXP. Traverses the expression in the constructor to build the
//! sets. What we have here is only syntactic information: whether defs
//! are local or free, and whether uses are applications or not.
//! Uses and defs must be mutually exclusive for different
//! statements--that is, in a compound statement only the variables
//! directly used or modified count, not those found in the loop body.
class R_ExpUDInfo {
public:
  R_ExpUDInfo(SEXP e) : exp(e) {
    local_defs = new VarSet();
    free_defs = new VarSet();
    app_uses = new VarSet();
    non_app_uses = new VarSet();
    build_ud_rhs(e);
  }
  OA::OA_ptr<VarSet> get_local_defs() { return local_defs; }
  OA::OA_ptr<VarSet> get_free_defs() { return free_defs; }
  OA::OA_ptr<VarSet> get_app_uses() { return app_uses; }
  OA::OA_ptr<VarSet> get_non_app_uses() { return non_app_uses; }
private:
  typedef enum {LOCAL, FREE} local_pred;
  SEXP exp;
  void build_ud_rhs(const SEXP e);
  void build_ud_lhs(const SEXP e, local_pred is_local);
  OA::OA_ptr<VarSet> local_defs;
  OA::OA_ptr<VarSet> free_defs;
  OA::OA_ptr<VarSet> app_uses;
  OA::OA_ptr<VarSet> non_app_uses;
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

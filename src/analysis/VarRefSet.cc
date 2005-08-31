#include <OpenAnalysis/Utils/OA_ptr.hpp>

#include <analysis/SimpleIterators.h>

#include "VarRefSet.h"

//--------------------------------------------------------------------
// R_VarRefSetIterator methods
//--------------------------------------------------------------------

OA::OA_ptr<R_VarRef> R_VarRefSetIterator::current() const {
  return *it;
}

bool R_VarRefSetIterator::isValid() const {
  return (it != vars->end());
}

void R_VarRefSetIterator::operator++() {
  ++it;
}

void R_VarRefSetIterator::reset() {
  it = vars->begin();
}

//--------------------------------------------------------------------
// R_VarRefSet methods
//--------------------------------------------------------------------

void R_VarRefSet::insert_ref(const OA::OA_ptr<R_BodyVarRef> var) {
  vars->insert(var);
}

void R_VarRefSet::insert_arg(const OA::OA_ptr<R_ArgVarRef> var) {
  vars->insert(var);
}

void R_VarRefSet::set_union(const R_VarRefSet & set2) {
  OA::OA_ptr<R_VarRefSetIterator> it; it = set2.get_iterator();
  for ( ; it->isValid(); ++*it) {
    vars->insert(it->current());
  }
}

void R_VarRefSet::set_union(OA::OA_ptr<R_VarRefSet> set2) {
  OA::OA_ptr<R_VarRefSetIterator> it; it = set2->get_iterator();
  for ( ; it->isValid(); ++*it) {
    vars->insert(it->current());
  }
}

OA::OA_ptr<R_VarRefSetIterator> R_VarRefSet::get_iterator() const {
  OA::OA_ptr<R_VarRefSetIterator> it;
  it = new R_VarRefSetIterator(vars);
  return it;
}

//! Given a list of formal arguments, construct and return a VarRefSet
//! of corresponding ArgRefs
OA::OA_ptr<R_VarRefSet> R_VarRefSet::refs_from_arglist(SEXP arglist) {
  OA::OA_ptr<R_VarRefSet> refs; refs = new R_VarRefSet;
  R_ListIterator iter(arglist);
  for( ; iter.isValid(); ++iter) {
    OA::OA_ptr<R_ArgVarRef> arg;
    arg = new R_ArgVarRef(iter.current());
    refs->insert_arg(arg);
  }
  return refs;
}

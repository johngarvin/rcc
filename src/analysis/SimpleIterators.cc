#include "analysis/SimpleIterators.h"

//--------------------------------------------------------------------
// R_SingletonIterator methods
//--------------------------------------------------------------------

SEXP R_SingletonIterator::current() const {
  return exp;
}

bool R_SingletonIterator::isValid() const {
  return valid;
}

void R_SingletonIterator::operator++() { 
  valid = false;
}

void R_SingletonIterator::reset() { 
  valid = true;
}

//--------------------------------------------------------------------
// R_ListIterator methods
//--------------------------------------------------------------------

SEXP R_ListIterator::current() const {
  return curr;
}

bool R_ListIterator::isValid() const {
  return (curr != R_NilValue);
}

void R_ListIterator::operator++() {
  // must be a data or language cons cell to be able to take the CDR
  assert(TYPEOF(curr) == LISTSXP || TYPEOF(curr) == LANGSXP);
  curr = CDR(curr);
}

void R_ListIterator::reset() {
  curr = exp;
}

//--------------------------------------------------------------------
// R_PreorderIterator methods
//--------------------------------------------------------------------

SEXP R_PreorderIterator::current() const {
  return *iter;
}
  
bool R_PreorderIterator::isValid() const {
  return (iter != preorder.end());
}

void R_PreorderIterator::operator++() {
  ++iter; 
}

void R_PreorderIterator::reset() {
  iter = preorder.begin();
}

//! Preorder iterator: building the preorder list of nodes
void R_PreorderIterator::build_pre(SEXP e) {
  assert(e != NULL);
  switch(TYPEOF(e)) {
  case NILSXP:
    return;
    break;
  case LISTSXP:
  case LANGSXP:
    preorder.push_back(e);
    build_pre(CAR(e));
    build_pre(CDR(e));
    break;
  default:
    preorder.push_back(e);
    break;
  }
}

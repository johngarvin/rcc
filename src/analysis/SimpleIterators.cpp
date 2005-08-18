#include "SimpleIterators.hpp"



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

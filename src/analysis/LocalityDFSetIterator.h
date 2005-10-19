// -*- Mode: C++ -*-

#ifndef LOCALITY_DF_SET_ITERATOR_H
#define LOCALITY_DF_SET_ITERATOR_H

#include <set>

#include <OpenAnalysis/Utils/OA_ptr.hpp>

#include <analysis/LocalityDFSetElement.h>

namespace Locality {

//! Iterator over DFSetElement's in an DFSet
class DFSetIterator {
public:
  DFSetIterator (OA::OA_ptr<std::set<OA::OA_ptr<DFSetElement> > > _set);
  ~DFSetIterator () {}
  
  void operator++();
  bool isValid();
  OA::OA_ptr<DFSetElement> current();
  void reset();

private:
  OA::OA_ptr<std::set<OA::OA_ptr<DFSetElement> > > mSet;
  std::set<OA::OA_ptr<DFSetElement> >::iterator mIter;
};

}  // namespace Locality

#endif

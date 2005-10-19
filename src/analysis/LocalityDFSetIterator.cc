#include <analysis/LocalityDFSetElement.h>

#include <cassert>

#include "LocalityDFSetIterator.h"

using namespace OA;

namespace Locality {

DFSetIterator::DFSetIterator (OA::OA_ptr<std::set<OA::OA_ptr<DFSetElement> > > _set)
  : mSet(_set)
{
  assert(!mSet.ptrEqual(NULL));
  mIter = mSet->begin();
}

void DFSetIterator::operator++() {
  if (isValid()) mIter++;
}

//! is the iterator at the end
bool DFSetIterator::isValid() {
  return (mIter != mSet->end());
}

//! return copy of current node in iterator
OA_ptr<DFSetElement> DFSetIterator::current() {
  assert(isValid());
  return (*mIter);
}

//! reset iterator to beginning of set
void DFSetIterator::reset() {
  mIter = mSet->begin();
}

}  // namespace Locality

#include "LocSetIterator.hpp"

namespace OA {

LocSetIterator::LocSetIterator(OA_ptr<LocSet> pSet) : mSet(pSet) {
    mIter = mSet->begin();
}

LocSetIterator::~LocSetIterator() { }

OA_ptr<Location> LocSetIterator::current() {
    if (isValid()) {
        return *mIter; 
    } else {
        OA_ptr<Location> retval;
        return retval;
    }
}

bool LocSetIterator::isValid() {
    return (mIter != mSet->end());
}

void LocSetIterator::operator++() {
    if (mIter!=mSet->end()) ++mIter;
}

void LocSetIterator::reset() {
    mIter = mSet->begin();
}

} // end namespace

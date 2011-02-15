/* Interface for DFAGen Tool 
   DFAGenDFSet.hpp
   Each flow-value needs to implement this interface
   Shweta Behere, Andy Stone (aistone@gmail.com)
*/

#ifndef DFAGenDFSet_H
#define DFAGenDFSet_H

#include <iostream>
#include <set>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>

namespace OA {
  namespace DataFlow {

/**
 * Analyses constructed by the dfagen tool require that the union and
 * intersect operators be defined.  The DFAGenDFSet template satisfies
 * the DataFlowSet interface so it can be used by the dataflow on
 * control flow graph solver (CFGDFSolver).  It also derives from set so normal
 * STL set operations can be performed on it.
 */
template<typename T>
class DFAGenDFSet : public virtual DataFlowSet, public std::set<T> {
  public:
    // note, this class uses this implicit copy constructor derived
    // from set<T>.  If you want to create an explicit constructor make
    // sure to use the 'explicit' keyword
    DFAGenDFSet () {}

    DFAGenDFSet (std::set<T> copy) {
        typename std::set<T>::iterator iter;
        for(iter = copy.begin(); iter != copy.end(); iter++) {
            insert(*iter);
        }
    }

    virtual ~DFAGenDFSet() {}

    void unionEqu(const DFAGenDFSet<T> &rhs);
    void intersectEqu(const DFAGenDFSet<T> &rhs);
    void minusEqu(const DFAGenDFSet<T> &rhs);

    bool isEmpty() { return this->empty(); }
    bool isSubset(const DFAGenDFSet<T> &rhs);
    bool isProperSubset(const DFAGenDFSet<T> &rhs);
    bool isSuperset(const DFAGenDFSet<T> &rhs);
    bool isProperSuperset(const DFAGenDFSet<T> &rhs);

    bool operator==(DataFlowSet &other) const;
    bool operator!=(DataFlowSet &other) const;

    void dump(std::ostream &os) { assert(false); }

    void dump(std::ostream &os, OA_ptr<IRHandlesIRInterface>) {
        assert(false);
    }

    virtual OA_ptr<DataFlowSet> clone();
};

template<typename T>
void DFAGenDFSet<T>::unionEqu(const DFAGenDFSet<T> &rhs) {
    // copy elements of the right hand set into this set.
    copy(rhs.begin(), rhs.end(), inserter(*this, this->begin()));
}

template<typename T>
void DFAGenDFSet<T>::intersectEqu(const DFAGenDFSet<T> &rhs) {
    typename DFAGenDFSet<T>::iterator lhsIter, lhsEnd;
    typename DFAGenDFSet<T>::const_iterator rhsIter, rhsEnd;

    lhsIter = this->begin();
    rhsIter = rhs.begin();
    lhsEnd = this->end();
    rhsEnd = rhs.end();

    // Elements in both sets are gauranteed to be sorted.  We want to remove
    // all elements from the left-hand set that are not in the right hand
    // set.  To do this we carefully iterate through both sets.
    // Iterate through all elements in the left-hand set
    while(lhsIter != lhsEnd) {
        // if we've reached the end in the right-hand set, remove all
        // remaning elements in the left hand set and stop.
        if(rhsIter == rhsEnd) {
            this->erase(lhsIter, lhsEnd);
            return;
        }
        // iterate through elements in the left hand set until we get to
        // an element that's greater than or equal to the one in the
        // left hand set
        while(rhsIter != rhsEnd && *rhsIter < *lhsIter) { ++rhsIter; }

        // if the left and right hand elements are not equal remove the
        // element in the left hand set.
        if(*lhsIter != *rhsIter) {
            this->erase(lhsIter++);
            lhsEnd = this->end();
        } else {
            ++lhsIter;
        }
    }
}

template<typename T>
void DFAGenDFSet<T>::minusEqu(const DFAGenDFSet<T> &rhs) {
    typename DFAGenDFSet<T>::iterator lhsIter, lhsEnd;
    typename DFAGenDFSet<T>::const_iterator rhsIter, rhsEnd;

    lhsIter = this->begin();
    rhsIter = rhs.begin();
    lhsEnd = this->end();
    rhsEnd = rhs.end();

    // we want to remove every element from the left hand side that's
    // also in the right hand side.  To do this we iterate through each
    // element on the right hand side and remove them.
    while(rhsIter != rhsEnd) {
        if(lhsIter == lhsEnd) { return; }

        while(lhsIter != lhsEnd && *lhsIter < *rhsIter) { ++lhsIter; }

        // if the left and right hand elements are equal remove the
        // element from the left hand set.
        if(*lhsIter == *rhsIter) {
            this->erase(lhsIter++);
            lhsEnd = this->end();
        } else {
            ++rhsIter;
        }
    }
}


template<typename T>
bool DFAGenDFSet<T>::isSubset(const DFAGenDFSet<T> &rhs) {
    /* is this set a subset of the rhs */
    typename DFAGenDFSet<T>::iterator iter;

    for(iter = this->begin(); iter != this->end(); iter++) {
        if(rhs.find(*iter) == rhs.end()) {
            return false;
        }
    }

    return true;
}


template<typename T>
bool DFAGenDFSet<T>::isProperSubset(const DFAGenDFSet<T> &rhs) {
    return isSubset(rhs) && this->size() != rhs.size();
} 

template<typename T>
bool DFAGenDFSet<T>::isSuperset(const DFAGenDFSet<T> &rhs) {
    assert(false);
/*    typename DFAGenDFSet<T>::iterator iter;

    for(iter = rhs.begin(); iter != rhs.end(); iter++) {
        if(this->find(*iter) == this->end()) {
            return false;
        }
    }

    return true;*/
}


template<typename T>
bool DFAGenDFSet<T>::isProperSuperset(const DFAGenDFSet<T> &rhs) {
    return isSuperset(rhs) && this->size() != rhs.size();
}


template<typename T>
bool DFAGenDFSet<T>::operator==(DataFlowSet &other) const {
    DFAGenDFSet<T> *castOther;

    castOther = dynamic_cast<DFAGenDFSet<T> *>(&other);

    return std::set<T>(*this) == std::set<T>(*castOther);
}


template<typename T>
bool DFAGenDFSet<T>::operator!=(DataFlowSet &other) const {
    DFAGenDFSet<T> *castOther;

    castOther = dynamic_cast<DFAGenDFSet<T> *>(&other);

    return std::set<T>(*this) != std::set<T>(*castOther);
}


template<typename T>
OA_ptr<DataFlowSet> DFAGenDFSet<T>::clone() {
    OA_ptr<DataFlowSet> newSet;
    newSet = new DFAGenDFSet<T>(*this);
    return newSet;
}


  } // end of DataFlow namespace
} // end of OA namespace

#endif

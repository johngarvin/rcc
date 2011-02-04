/*! \file
  
  \brief Templated implementation of an OpenAnalysis iterator interface
         for a list of objects.

  \authors Andy Stone, Michelle Strout
*/

#ifndef ListIterator_H
#define ListIterator_H

#include <list>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
using namespace std;

namespace OA {

template <class T>
class ListIterator {
public:
    ListIterator (OA_ptr<list<T> > pList) : mList(pList), mIter(pList->begin()) 
    {}
    virtual ~ListIterator () {}

    void operator++() { if (mIter != mList->end()) { mIter++; } }
    void operator++(int) { ++*this; } 

    //! returns true if we are not past the end of the list
    bool isValid() { return (mIter != mList->end()); }

    //! returns the statement handle that the iterator is currently pointing 
    T current() { return *mIter; }

    void reset() { mIter = mList->begin(); }

  private:
    OA_ptr<list<T> > mList;
    typename list<T>::iterator mIter;
};




} // end of OA namespace

#endif

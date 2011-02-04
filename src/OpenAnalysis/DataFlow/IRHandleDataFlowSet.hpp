/*! \file
  
  \brief DataflowSet that consists of a set of IRHandles.

  \authors Michelle Strout (June 2004)
  \version $Id: IRHandleDataFlowSet.hpp,v 1.11 2005/06/10 02:32:04 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  Don't actually create an IRHandleDataFlowSet.  Instead inherit 
  privately from it to get implementation of DataFlowSet operations
  for a set of IRHandle objects.
*/

#ifndef IRHandleDataFlowSet_h
#define IRHandleDataFlowSet_h

#include <iostream>
#include <set>
#include <algorithm>
#include <iterator>
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace DataFlow {

template <class T> class IRHandleIterator;



template <class T> 
class IRHandleDataFlowSet : public virtual DataFlowSet {
  public:
    //************************************************************
    // Constructor / Destructor
    //************************************************************
    
    IRHandleDataFlowSet() : mUniversal(false) {}
    IRHandleDataFlowSet(const IRHandleDataFlowSet<T> &other) :
        mSet(other.mSet),
        mUniversal(false)
    {}
    ~IRHandleDataFlowSet() {}
    OA_ptr<DataFlowSet> clone() const;

    //************************************************************
    // Methods inherited from DataFlowSet
    //************************************************************
    virtual bool operator==(DataFlowSet &other) const;
    virtual bool operator!=(DataFlowSet &other) const;
    virtual void setUniversal();
    virtual void clear();
    virtual int  size() const;
    virtual bool isUniversalSet() const;
    virtual bool isEmpty() const;
    virtual void output(IRHandlesIRInterface& ir) const;
    virtual void dump(std::ostream &os, OA_ptr<IRHandlesIRInterface>);

    //************************************************************
    // Set operations
    //************************************************************

    //! Union the argument with this set, store results in this set
    IRHandleDataFlowSet<T>& unionEqu(DataFlowSet &other);
      
    //! Intersects the argument with this set, store results in this set
    IRHandleDataFlowSet<T>& intersectEqu(DataFlowSet &other);

    //! Subtract the argument from this set, store results in this set
    IRHandleDataFlowSet<T>& minusEqu(DataFlowSet &other);

    //! Insert element into set
    void insert(T h) { mSet.insert(h); }

    //! Remove elemtent from set
    void remove(T h) { mSet.erase(h); }

    //! Return true if this set contains item
    bool contains(const T &item) const;


  protected:
    std::set<T> mSet;
    bool mUniversal;

    friend class IRHandleIterator<T>;
};

//--------------------------------------------------------------------
/*! An iterator over IRHandles in the set.
*/
//--------------------------------------------------------------------
//template <class T> class std::set<T>::iterator;

template <class T>
class IRHandleIterator {
public:
    // have to get OA_ptr to a set because otherwise the set
    // might disappear before user of this iterator references it
    IRHandleIterator (OA_ptr<IRHandleDataFlowSet<T> > DFSet) : mDFSet(DFSet) 
    { hIter = mDFSet->mSet.begin(); }
    ~IRHandleIterator () {}

    void operator ++ () { if (hIter != mDFSet->mSet.end()) hIter++; }
    void operator++(int) { ++*this; }  // postfix
    
    //! is the iterator at the end
    bool isValid() const { return (hIter != mDFSet->mSet.end()); }
    
    //! return current node
    T current() const { return (*hIter); }

private:
    OA_ptr<IRHandleDataFlowSet<T> > mDFSet;
    typename std::set<T>::iterator hIter;
};





template<typename T> 
OA_ptr<DataFlowSet> IRHandleDataFlowSet<T>::clone() const {
    OA_ptr<IRHandleDataFlowSet<T> > retval;
    retval = new IRHandleDataFlowSet<T>(*this); return retval;
}

template<typename T>
bool IRHandleDataFlowSet<T>::operator==(DataFlowSet &other) const
{
    IRHandleDataFlowSet<T>& recastOther =
        dynamic_cast<IRHandleDataFlowSet<T>&>(other);
    return mSet == recastOther.mSet; 
}

template<typename T>
bool IRHandleDataFlowSet<T>::operator!=(DataFlowSet &other) const
{ 
    IRHandleDataFlowSet<T>& recastOther =
        dynamic_cast<IRHandleDataFlowSet<T>&>(other);
    return mSet != recastOther.mSet; 
}

template<typename T>
void IRHandleDataFlowSet<T>::setUniversal() {
    mUniversal = true;
}

template<typename T>
void IRHandleDataFlowSet<T>::clear() {
   mSet.clear();
   mUniversal = false;
}

template<typename T>
bool IRHandleDataFlowSet<T>::isUniversalSet() const {
   return mUniversal;
}

template<typename T>
int IRHandleDataFlowSet<T>::size() const {
    return mSet.size();
}

template<typename T>
bool IRHandleDataFlowSet<T>::isEmpty() const {
   return mSet.empty() && !mUniversal;
}

template<typename T>
void IRHandleDataFlowSet<T>::output(IRHandlesIRInterface& ir) const {
    typename std::set<T>::iterator iter;

    sOutBuild->objStart("DataFlowSet");
    sOutBuild->listStart();
    for(iter =  mSet.begin();
        iter != mSet.end(); iter++)
    {
        sOutBuild->listItemStart();
        sOutBuild->outputIRHandle(*iter, ir);
        sOutBuild->listItemEnd();
    }
    sOutBuild->listEnd();
    sOutBuild->objEnd("DataFlowSet"); 
}

template<typename T>
void IRHandleDataFlowSet<T>::dump(
    std::ostream &os, OA_ptr<IRHandlesIRInterface> ir)
{
    typename std::set<T>::iterator iter;
    os << "{";
    for(iter = mSet.begin(); iter != mSet.end(); iter++) {
        if(iter != mSet.begin()) {
            os << ", ";
        }
        os << ir->toString(*iter);
    }
    os << "}";
}


template<typename T>
IRHandleDataFlowSet<T>& IRHandleDataFlowSet<T>::unionEqu(DataFlowSet &other) {
    // first check if either side is a universal set. A universal set
    // unioned with any other set results in a universal set.
    if(isUniversalSet() || other.isUniversalSet()) {
        setUniversal();
    }

    // Insert all elements in the RHS set into this set
    IRHandleDataFlowSet<T>& recastOther =
        dynamic_cast<IRHandleDataFlowSet<T>&>(other);
    std::set<T> temp;
    std::set_union(mSet.begin(), mSet.end(), 
    recastOther.mSet.begin(), recastOther.mSet.end(),
    std::inserter(temp,temp.end()));
    mSet = temp;
    return *this;
}

template<typename T>
IRHandleDataFlowSet<T>& IRHandleDataFlowSet<T>::intersectEqu(
    DataFlowSet &other)
{
    // first check if either side is a universal set. A universal set
    // intersected with any other set results in the other set
    if(isUniversalSet()) {
        mUniversal = false;

        IRHandleDataFlowSet<T>& recastOther =
            dynamic_cast<IRHandleDataFlowSet<T>&>(other);

        mSet.clear();
        mSet.insert(recastOther.mSet.begin(), recastOther.mSet.end());

        if(other.isUniversalSet()) { mUniversal = true; }

        return *this;
    }
    else if(other.isUniversalSet()) {
        return *this;
    }

    // Perform set intersection
    IRHandleDataFlowSet<T>& recastOther =
        dynamic_cast<IRHandleDataFlowSet<T>&>(other);
    std::set<T> temp;
    std::set_intersection(mSet.begin(), mSet.end(),
    recastOther.mSet.begin(), recastOther.mSet.end(),
    std::inserter(temp,temp.end()));
    mSet = temp;
    return *this;

}

template<typename T>
IRHandleDataFlowSet<T>& IRHandleDataFlowSet<T>::minusEqu(
    DataFlowSet &other)
{
    // TODO: Determine how to handle universal sets in this case
    if(isUniversalSet()) {
        assert(false);
    }
 
    IRHandleDataFlowSet<T>& recastOther =
        dynamic_cast<IRHandleDataFlowSet<T>&>(other);
    std::set<T> temp;
    std::set_difference(mSet.begin(), mSet.end(),
    recastOther.mSet.begin(), recastOther.mSet.end(),
    std::inserter(temp,temp.end()));
    mSet = temp;
    return *this;
}

template<typename T>
bool IRHandleDataFlowSet<T>::contains(const T &item) const {
    return mSet.find(item) != mSet.end();
}


  } // end of DataFlow namespace
} // end of OA namespace

#endif

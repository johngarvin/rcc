/*! \file
  
  \brief Templated class used in dataflow analyses.

  \authors Michelle Strout

  Requires that the size of the universal set is finite and known at
  construction time, because underlying implementation is a
  bit vector.
  Copied initially from DataFlowSetImpl.hpp, but then significantly edited.

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef DFSetBitImpl_h
#define DFSetBitImpl_h

#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>
#include <set>
#include <boost/dynamic_bitset.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace DataFlow {

template <class T> class DFSetBitImplIterator;

/*! Templated set type based on a bit vector for propogating 
 *  information in a data-flow analysis */
template<typename T>
class DFSetBitImpl : public virtual DataFlowSet {
  public:

  // ===============================================
  // DataFlowSetImpl Constructor and Destructor
  // ===============================================
      DFSetBitImpl(int maxsize) : mSet(maxsize)
      {}

      DFSetBitImpl(const DFSetBitImpl<T>& other) : 
          mSet(other.mSet)
      {}

      DFSetBitImpl(int maxsize, std::set<T>& other) ;

      ~DFSetBitImpl() {}

  // ===============================================
  // Methods inherited from DataFlow interface
  // ===============================================
      virtual OA_ptr<DataFlowSet> clone() const;

      virtual bool operator ==(DataFlowSet &other) const;

      virtual bool operator !=(DataFlowSet &other) const;

      virtual void setUniversal() {
           mSet.set();
      }

      virtual void clear() {
           mSet.reset();
      }
     
      virtual int size() const {
           return mSet.count();
      }

      virtual bool isUniversalSet() const {
           return mSet.size() == mSet.count();
      }

      virtual bool isEmpty() const {
           return mSet.none();
      }
  
  // ================================================
  // DFSetBitImpl Set Operations
  // ================================================

      //! Union the argument with this set, store results in this set
      DFSetBitImpl<T>& unionEqu(DataFlowSet &other);
      
      //! Intersects the argument with this set, store results in this set
      DFSetBitImpl<T>& intersectEqu(DataFlowSet &other);

      //! Subtract the argument from this set, store results in this set
      DFSetBitImpl<T>& minusEqu(DataFlowSet &other);

      //! is this set a subset of the argument
      bool isSubset(DFSetBitImpl<T> &rhs);
      
      //! is this set a proper subset the argument
      bool isProperSubset(DFSetBitImpl<T> &rhs);

      //! is this set a superset of the argument
      bool isSuperset(DFSetBitImpl<T> &rhs);
       
      //! is this set a proper superset of argument
      bool isProperSuperset(DFSetBitImpl<T> &rhs);

  // ===============================================
  // Modifier Methods
  // ===============================================

      //! Insert an item into this set
      void insert(T h) {
          mSet.set(sValToInt[h]);
      }

      //! Find and remove item from this set
      void remove(T h) {
           mSet.reset(sValToInt[h]);
      }

  // ===============================================
  // Query Methods
  // ===============================================
    
      //! Return true if this set contains the passed item
      bool contains(const T &item) const {
           return mSet.test(sValToInt[item]);
      }
      
  //! ==============================================
  //! Output routines
  //! ==============================================

      virtual void output(IRHandlesIRInterface& ir) const;

    virtual std::string toString(IRHandlesIRInterface& ir);
    void dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> ir);


  protected:
      boost::dynamic_bitset<> mSet;
      static std::map<T,int> sValToInt;
      static std::map<int,T> sIntToVal;
      static int sMapCount;
      friend class DFSetBitImplIterator<T>;
};


/*! An iterator over objects in the set.  */
/* 
 * FIXME: Think I want to make this not be a friend and instead of pointing
 * to an existing set have it make a copy.
 */
template <class T>
class DFSetBitImplIterator {
  public:

    DFSetBitImplIterator () {
        // don't want to create an empty set and then iterate through
        // static map, because that would take a long time
        mEmpty = true;
    }

    // FIXME: very inefficient so should not be used often
    DFSetBitImplIterator (OA_ptr<DFSetBitImpl<T> > DFSet)
    {
      if (DFSet.ptrEqual(NULL)) {
        mEmpty = true;
      } else {
        // create a set with all of the objects of type T
        // that are in the bitset
        for (int i=0; i!=DFSet->mSet.size(); i++) {
            if (DFSet->mSet[i]) {
                mSet.insert(DFSetBitImpl<T>::sIntToVal[i]);
            }
        }
        mIter = mSet.begin();
        mEmpty = false;
      }
    }

    ~DFSetBitImplIterator () {}

    //! Move the iterator one position down
    void operator++() {
        if (mIter != mSet.end()) mIter++;
    }

    //! Move the iterator one position down
    void operator++(int) {
        ++*this;
    }
    
    //! Is the iterator at the end
    bool isValid() const { return (!mEmpty && mIter != mSet.end()); }
    
    //! Return current node
    T current() const { return (*mIter); }

    //! Reset iterator (move to start)
    void reset() { mIter = mSet.begin(); }

    //! Return true if iterator is still on first element
    bool isFirst() { return mIter == mSet.begin(); }

    //! Is the underlying set a universal set?
    bool isUniversalSetIter() { assert(0); }
        //return (mDFSet->isUniversalSet()); }

  private:
    bool mEmpty;
    typename std::set<T> mSet;
    typename std::set<T>::iterator mIter;
};

template<typename T>
DFSetBitImpl<T>::DFSetBitImpl(int maxsize, std::set<T>& other) :
    mSet(maxsize)
{
    // iterate through items in set
    typename std::set<T>::iterator iter;
    for (iter=other.begin(); iter!=other.end(); iter++) 
    {
        // make sure that object is in static map
        if (sValToInt.find(*iter)==sValToInt.end()) {
            sValToInt[*iter] = sMapCount;
            sIntToVal[sMapCount] = *iter;
            sMapCount++;
        }
        // set bit in set
        mSet.set(sValToInt[*iter]);
    }
}

template<typename T>
OA_ptr<DataFlowSet> DFSetBitImpl<T>::clone() const 
{ 
    OA_ptr<DFSetBitImpl<T> > retval;
    retval = new DFSetBitImpl<T>(*this); 
    return retval; 
}


template<typename T>
bool DFSetBitImpl<T>::operator==(DataFlowSet &other) const 
{ 
    DFSetBitImpl<T>& recastOther =
        dynamic_cast<DFSetBitImpl<T>&>(other);
    return (mSet == recastOther.mSet);
}


template<typename T>
bool DFSetBitImpl<T>::operator!=(DataFlowSet &other) const 
{ 
    DFSetBitImpl<T>& recastOther =
        dynamic_cast<DFSetBitImpl<T>&>(other);
    return (mSet != recastOther.mSet); 
}


template<typename T>
DFSetBitImpl<T>& DFSetBitImpl<T>::unionEqu(DataFlowSet &other) 
{ 
    // Insert all elements in the RHS set into this set
    DFSetBitImpl<T>& recastOther =
      dynamic_cast<DFSetBitImpl<T>&>(other);

    mSet |= recastOther.mSet;
    return *this;
}


template<typename T>
DFSetBitImpl<T>& DFSetBitImpl<T>::intersectEqu(DataFlowSet &other) 
{
    // Intersect this set with other and put result in this.
    DFSetBitImpl<T>& recastOther =
      dynamic_cast<DFSetBitImpl<T>&>(other);

    mSet &= recastOther.mSet;
    return *this;
}


template<typename T>
DFSetBitImpl<T>& DFSetBitImpl<T>::minusEqu(DataFlowSet &other) 
{
    DFSetBitImpl<T>& recastOther =
      dynamic_cast<DFSetBitImpl<T>&>(other);

    mSet -= recastOther.mSet;
    return *this;
}


template<typename T>
bool DFSetBitImpl<T>::isSubset(DFSetBitImpl<T> &rhs) 
{
    return mSet.is_subset_of(rhs.mSet);
}


template<typename T>
bool DFSetBitImpl<T>::isProperSubset(DFSetBitImpl<T> &rhs) 
{
    return mSet.is_proper_subset_of(rhs.mSet);
} 


template<typename T>
bool DFSetBitImpl<T>::isSuperset(DFSetBitImpl<T> &rhs) 
{
    return rhs.mSet.is_subset_of(mSet);
}

template<typename T>
bool DFSetBitImpl<T>::isProperSuperset(DFSetBitImpl<T> &rhs) 
{
    return rhs.mSet.is_proper_subset_of(mSet);
}


template<typename T>
void DFSetBitImpl<T>::output(IRHandlesIRInterface& ir) const 
{
    typename std::set<T>::iterator iter;

    sOutBuild->objStart("DataFlowSet");
    
    sOutBuild->listStart();
    for (int i=0; i!=this->size(); ++i) 
    {
        sOutBuild->listItemStart();
        sIntToVal[mSet[i]].output(ir);
        sOutBuild->listItemEnd();
    }
    sOutBuild->listEnd();

    sOutBuild->objEnd("DataFlowSet");
}


template<typename T>
std::string  DFSetBitImpl<T>::toString(IRHandlesIRInterface& ir)
{
    std::ostringstream os;

    typename std::set<T>::iterator iter;

    os << "{";
    os << mSet;
    os << "}";

    return os.str();
}

template<typename T>
void DFSetBitImpl<T>::dump(std::ostream &os,
    OA_ptr<IRHandlesIRInterface> ir)
{
    typename std::set<T>::iterator iter;

    os << "{";
    os << mSet;
    os << "}";
}


// define static class variables
template<typename T>
typename std::map<T,int> DFSetBitImpl<T>::sValToInt;
template<typename T>
typename std::map<int,T> DFSetBitImpl<T>::sIntToVal;
template<typename T>
int DFSetBitImpl<T>::sMapCount=0;

  } // end of DataFlow namespace
} // end of OA namespace

#endif

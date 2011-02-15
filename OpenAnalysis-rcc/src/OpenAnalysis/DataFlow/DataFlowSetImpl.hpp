/*! \file
  
  \brief Templated class used in dataflow analyses.

  \authors Andrew Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef DataFlowSetImpl_h
#define DataFlowSetImpl_h

#include <iostream>
#include <set>
#include <algorithm>
#include <iterator>
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace DataFlow {

template <class T> class DataFlowSetImplIterator;

/*! Templated set type for propogating information in a data-flow analysis */
template<typename T>
class DataFlowSetImpl : public virtual DataFlowSet {
  public:

  // ===============================================
  // DataFlowSetImpl Constructor and Destructor
  // ===============================================
      DataFlowSetImpl() :
           mUniversal(false)
      {}

      DataFlowSetImpl(OA_ptr<std::set<T> >other) :
           mSet(*(other)),
           mUniversal(false)
      {}

      DataFlowSetImpl(std::set<T> other) :
           mSet(other),
           mUniversal(false)
      {}

      ~DataFlowSetImpl() {}

  // ===============================================
  // Methods inherited from DataFlow interface
  // ===============================================
      virtual OA_ptr<DataFlowSet> clone() const;

      virtual bool operator ==(DataFlowSet &other) const;

      virtual bool operator !=(DataFlowSet &other) const;

      virtual void setUniversal() {
           mUniversal = true;
           mSet.clear();
      }

      virtual void clear() {
           mSet.clear();
           mUniversal = false;
      }
     
      virtual int size() const {
           return mSet.size();
      }

      virtual bool isUniversalSet() const {
           return mUniversal;
      }

      virtual bool isEmpty() const {
           return mSet.empty() && !mUniversal;
      }
  
  // ================================================
  // DataFlowSetImpl Set Operations
  // ================================================

      //! Union the argument with this set, store results in this set
      DataFlowSetImpl<T>& unionEqu(DataFlowSet &other);
      
      //! Intersects the argument with this set, store results in this set
      DataFlowSetImpl<T>& intersectEqu(DataFlowSet &other);

      //! Subtract the argument from this set, store results in this set
      DataFlowSetImpl<T>& minusEqu(DataFlowSet &other);

      //! is this set a subset of the argument
      bool isSubset(DataFlowSetImpl<T> &rhs);
      
      //! is this set a proper subset the argument
      bool isProperSubset(DataFlowSetImpl<T> &rhs);

      //! is this set a superset of the argument
      bool isSuperset(DataFlowSetImpl<T> &rhs);
       
      //! is this set a proper superset of argument
      bool isProperSuperset(DataFlowSetImpl<T> &rhs);

  // ===============================================
  // Modifier Methods
  // ===============================================

      //! Insert an item into this set
      void insert(T h) {
          // FIXME: 8/5/08, MMS, this is not right.
          // should be able to insert anything into a universal set
          // and it will stay universal
          if (mUniversal) {mUniversal = false;}
          mSet.insert(h);
      }

      //! Find and remove item from this set
      void remove(T h) {
           mSet.erase(h);
      }

  // ===============================================
  // Query Methods
  // ===============================================
    
      //! Return true if this set contains the passed item
      bool contains(const T &item) const {
           return mSet.find(item) != mSet.end();
      }

      std::set<T>& getSet() {
          return mSet;
      }
      
  //! ==============================================
  //! Output routines
  //! ==============================================

      virtual void output(IRHandlesIRInterface& ir) const;

  virtual std::string toString(IRHandlesIRInterface& ir);
    void dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> ir);


  protected:
      std::set<T> mSet;
      bool mUniversal;
      friend class DataFlowSetImplIterator<T>;
};


/*! An iterator over IRHandles in the set.  */
template <class T>
class DataFlowSetImplIterator {
  public:

    DataFlowSetImplIterator () {
        mDFSet = new DataFlowSetImpl<T>();
        mIter = mDFSet->mSet.begin();
    }

    DataFlowSetImplIterator (OA_ptr<DataFlowSetImpl<T> > DFSet)
    {
      mDFSet = (DFSet->clone()).template convert<DataFlowSetImpl<T> >();

      //----------------------
      // BK: needed the 'template ' above to tell compiler that the '<' after
      //     convert was the beginning of a template and not a less-than
      // error message was: expected primary-expression before ">" token
      //----------------------

           // If the dataflow set is universal assert
           //assert(!mDFSet->isUniversalSet());

      //------------------
      // BK: 7/3/08
      // removed assert above
      // some interfaces only have access to iterators and not the underlying
      // set, so they had no way to check for universality to avoid this
      // assert.
      //
      // added isUniversalSetIter() function below to allow for this check
      //------------------

           // have to get OA_ptr to a set because otherwise the set
           // might disappear before user of this iterator references it

      mIter = mDFSet->mSet.begin();
    }


    ~DataFlowSetImplIterator () {}

    //! Move the iterator one position down
    void operator++() {
        if (mIter != mDFSet->mSet.end()) mIter++;
    }

    //! Move the iterator one position down
    void operator++(int) {
        ++*this;
    }
    
    //! Is the iterator at the end
    bool isValid() const { return (mIter != mDFSet->mSet.end()); }
    
    //! Return current node
    T current() const { return (*mIter); }

    //! Reset iterator (move to start)
    void reset() { mIter = mDFSet->mSet.begin(); }

    //! Return true if iterator is still on first element
    bool isFirst() { return mIter == mDFSet->mSet.begin(); }

    //! Is the underlying set a universal set?
    bool isUniversalSetIter() { return (mDFSet->isUniversalSet()); }

  private:
      OA_ptr<DataFlowSetImpl<T> > mDFSet;
      typename std::set<T>::iterator mIter;
};


template<typename T>
OA_ptr<DataFlowSet> DataFlowSetImpl<T>::clone() const { 

    OA_ptr<DataFlowSetImpl<T> > retval;
    retval = new DataFlowSetImpl<T>(mSet); 

    if (mUniversal) { retval->setUniversal();}
    return retval; 
}


template<typename T>
bool DataFlowSetImpl<T>::operator==(DataFlowSet &other) const { 
    DataFlowSetImpl<T>& recastOther =
        dynamic_cast<DataFlowSetImpl<T>&>(other);
    bool retval = false;
    if (mUniversal == recastOther.mUniversal) {
      retval = (mSet == recastOther.mSet);
    }
    return retval;
}


template<typename T>
bool DataFlowSetImpl<T>::operator!=(DataFlowSet &other) const { 
    DataFlowSetImpl<T>& recastOther =
        dynamic_cast<DataFlowSetImpl<T>&>(other);
    return mSet != recastOther.mSet; 
}


template<typename T>
DataFlowSetImpl<T>& DataFlowSetImpl<T>::unionEqu(DataFlowSet &other) { 
    // first check if either side is a universal set. A universal set
    // unioned with any other set results in a universal set.
    if(isUniversalSet() || other.isUniversalSet()) {
        setUniversal();
        
    } else {

      // Insert all elements in the RHS set into this set
      DataFlowSetImpl<T>& recastOther =
        dynamic_cast<DataFlowSetImpl<T>&>(other);
      std::set<T> temp;
      std::set_union(mSet.begin(), mSet.end(), 
                     recastOther.mSet.begin(), recastOther.mSet.end(),
                     std::inserter(temp,temp.end()));
      mSet = temp;
    }

    return *this;
}


template<typename T>
DataFlowSetImpl<T>& DataFlowSetImpl<T>::intersectEqu(DataFlowSet &other) {
    // first check if either side is a universal set. A universal set
    // intersected with any other set results in the other set
    if (other.isUniversalSet()) {
        return *this;
    } else if (isUniversalSet()) {

        DataFlowSetImpl<T>& recastOther =
            dynamic_cast<DataFlowSetImpl<T>&>(other);
        mUniversal = false;
        mSet.clear(); // ?? should already be empty if isUniversalSet() is true
        mSet.insert(recastOther.mSet.begin(), recastOther.mSet.end());

        return *this;
    }

    // Perform set intersection
    DataFlowSetImpl<T>& recastOther =
        dynamic_cast<DataFlowSetImpl<T>&>(other);
    std::set<T> temp;
    std::set_intersection(mSet.begin(), mSet.end(),
    recastOther.mSet.begin(), recastOther.mSet.end(),
    std::inserter(temp,temp.end()));
    mSet = temp;
    return *this;
}


template<typename T>
DataFlowSetImpl<T>& DataFlowSetImpl<T>::minusEqu(DataFlowSet &other) {
    // TODO: Determine how to handle universal sets in this case
    if(isUniversalSet()) {
        assert(false);
    }
 
    DataFlowSetImpl<T>& recastOther =
        dynamic_cast<DataFlowSetImpl<T>&>(other);
    if (recastOther.isUniversalSet()) {
      assert(false);
    }

    std::set<T> temp;
    std::set_difference(mSet.begin(), mSet.end(),
    recastOther.mSet.begin(), recastOther.mSet.end(),
    std::inserter(temp,temp.end()));
    mSet = temp;
    return *this;
}


template<typename T>
bool DataFlowSetImpl<T>::isSubset(DataFlowSetImpl<T> &rhs) {
    /* is this set a subset of the rhs */
    if (rhs.isUniversalSet()) {
      return true;
    }

    typename std::set<T>::iterator iter;
    for(iter = mSet.begin(); iter != mSet.end(); iter++) {
        if(!rhs.contains(*iter)) {
            return false;
        }
    }

    return true;
}


template<typename T>
bool DataFlowSetImpl<T>::isProperSubset(DataFlowSetImpl<T> &rhs) {

  // because we have no use of <T> in this function, templating is hard
  // so, elected to use mUniversal rather than isUniversalSet()
  if (mUniversal && rhs.mUniversal) {
      return false;
  }
  return isSubset(rhs) && this->size() != rhs.size();
} 


template<typename T>
bool DataFlowSetImpl<T>::isSuperset(DataFlowSetImpl<T> &rhs) {
    /* is this set a superset of the rhs */

    if (isUniversalSet()) {
      return true; // universal sets are always supersets
    }

    typename std::set<T>::iterator iter;
    for(iter = rhs.mSet.begin(); iter != rhs.mSet.end(); iter++) {
        if(!contains(*iter)) {
            return false;
        }
    }

    return true;
}


template<typename T>
bool DataFlowSetImpl<T>::isProperSuperset(DataFlowSetImpl<T> &rhs) {
  if (mUniversal && rhs.mUniversal) {
    return false;
  }
  return isSuperset(rhs) && this->size() != rhs.size();
}


template<typename T>
void DataFlowSetImpl<T>::output(IRHandlesIRInterface& ir) const {
    typename std::set<T>::iterator iter;

    sOutBuild->objStart("DataFlowSet");
    
    if (mUniversal) {
      sOutBuild->field("mUniversal","true");
    } else {
      sOutBuild->listStart();
      for(iter = mSet.begin(); iter != mSet.end(); iter++) {
        sOutBuild->listItemStart();
        (*iter).output(ir);
        sOutBuild->listItemEnd();
      }
      sOutBuild->listEnd();
    }
    sOutBuild->objEnd("DataFlowSet");
}


template<typename T>
std::string  DataFlowSetImpl<T>::toString(IRHandlesIRInterface& ir)
{
    std::ostringstream os;

    typename std::set<T>::iterator iter;

    os << "{";
    if (mUniversal) {
      os << "universal";
    } else {
      os << "not-universal";
    }
    
    for(iter = mSet.begin(); iter != mSet.end(); iter++) {
      os << ", ";
      os << *iter;
    }
    os << "}";

    return os.str();
}

template<typename T>
void DataFlowSetImpl<T>::dump(std::ostream &os,
    OA_ptr<IRHandlesIRInterface> ir)
{
    typename std::set<T>::iterator iter;

    os << "{";
    if (mUniversal) {
      os << "universal, ";
    } else {
      os << "not-universal, ";
    }
    
    for(iter = mSet.begin(); iter != mSet.end(); iter++) {
      os << ", ";
      os << *iter;
    }
    os << "}";

  
}



  } // end of DataFlow namespace
} // end of OA namespace

#endif

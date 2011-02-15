//ExprDFSet.hpp

#ifndef ExprDFSet_H
#define ExprDFSet_H

#include <iostream>
#include <set>
#include <algorithm>
#include <iterator>

// abstract interface that this class implements
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>
#include <OpenAnalysis/Location/Locations.hpp>
#include <OpenAnalysis/Location/LocationVisitor.hpp>
#include <OpenAnalysis/DataFlow/CalleeToCallerVisitor.hpp>
#include <OpenAnalysis/ExprTree/EvalToMemRefVisitor.hpp>
#include <OpenAnalysis/ExprTree/MemRefsVisitor.hpp>
#include <OpenAnalysis/ExprTree/ExprTreeSetIterator.hpp>

namespace OA {
  namespace DataFlow {

//class ExprTreeIterator;

class ExprDFSet : public virtual  DataFlowSet {
public:
  ExprDFSet();
  ExprDFSet(const ExprDFSet &other);
  ~ExprDFSet() {}


  OA_ptr<DataFlowSet> clone();
  
  bool operator ==(DataFlowSet &other) const;

  bool operator !=(DataFlowSet &other) const;

  bool operator <(DataFlowSet &other) const;
 
  bool operator =(DataFlowSet &other) const;

  ExprDFSet& setUnion(DataFlowSet &other);

  ExprDFSet& setIntersect(DataFlowSet &other);

  ExprDFSet& setDifference(ExprDFSet &other);

  void dump(std::ostream &os);

  void dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> ir);
  
  void insert(OA_ptr<ExprTree> h);
  void remove(OA_ptr<ExprTree> h);

  //! whether or not the set is empty
  bool empty();
  
  //! whether or not the set contains the given stmt

  bool hasExprTree(OA_ptr<ExprTree> h);

 protected:

   OA_ptr<ExprTreeSet> mSetPtr;

  friend class ExprDFSetIterator;

};
 
 class ExprDFSetIterator : public ExprTreeSetIterator {
 public:
   ExprDFSetIterator(ExprDFSet& pDFSet)
      : ExprTreeSetIterator(pDFSet.mSetPtr) { }
  ~ExprDFSetIterator() { }
};

//! general iterator for ExprTree
/*class ExprTreeSetIterator {
public:

    ExprTreeSetIterator (OA_ptr<ExprDFSet> DFSet) : mDFSet(DFSet)
    { hIter = mDFSet->mSet.begin(); }
    ~ExprSetTreeIterator () {}

    void operator ++ () { if (hIter != mDFSet->mSet.end()) hIter++; }
    void operator++(int) { ++*this; }  // postfix

    //! is the iterator at the end
    bool isValid() const { return (hIter != mDFSet->mSet.end()); }

    //! return current node
    OA_ptr<ExprTree> current() const { return (*hIter); }

private:
    OA_ptr<ExprDFSet> mDFSet;
    std::set<OA_ptr<ExprTree> >::iterator hIter;
};*/



  } // end of DataFlow namespace
} // end of OA namespace

#endif

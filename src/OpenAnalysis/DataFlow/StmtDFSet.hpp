//StmtDFSet.hpp

/*#ifndef StmtDFSet_H
#define StmtDFSet_H

#include <iostream>
#include <set>
#include <algorithm>
#include <iterator>

// abstract interface that this class implements
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>
#include <OpenAnalysis/Location/Locations.hpp>
#include <OpenAnalysis/Location/LocationVisitor.hpp>
#include <OpenAnalysis/DataFlow/CalleeToCallerVisitor.hpp>
#include <OpenAnalysis/ExprTree/EvalToMemRefVisitor.hpp>
#include <OpenAnalysis/ExprTree/MemRefsVisitor.hpp>

namespace OA {
  namespace DataFlow {

class StmtDFSet : public virtual  DataFlowSet{
public:
  StmtDFSet();
  StmtDFSet(const StmtDFSet &other);
  ~StmtDFSet() {}


  OA_ptr<DataFlowSet> clone();
  
  bool operator ==(DataFlowSet &other) const;

  bool operator !=(DataFlowSet &other) const;
  
  bool operator <(DataFlowSet &other) const;

  StmtDFSet& setUnion(DataFlowSet &other);

  StmtDFSet& setIntersect(StmtDFSet &other);

  StmtDFSet& setDifference(DataFlowSet &other);

  void dump(std::ostream &os);

  void dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> ir);
  
  void insert(StmtHandle h);
  void remove(StmtHandle h);

  //! whether or not the set is empty
  bool empty();
  
  //! whether or not the set contains the given stmt

  bool handleInSet(StmtHandle h);

 protected:

  std::set<StmtHandle> mSet;

  friend class StmtHandleIterator;

};

//! general iterator for Locations
class StmtHandleIterator {
public:
    
    StmtHandleIterator (OA_ptr<StmtDFSet> DFSet) : mDFSet(DFSet)
    { hIter = mDFSet->mSet.begin(); }
    ~StmtHandleIterator () {}

    void operator ++ () { if (hIter != mDFSet->mSet.end()) hIter++; }
    void operator++(int) { ++*this; }  // postfix

    //! is the iterator at the end
    bool isValid() const { return (hIter != mDFSet->mSet.end()); }

    //! return current node
    StmtHandle current() const { return (*hIter); }

private:
    OA_ptr<StmtDFSet> mDFSet;
    std::set<StmtHandle>::iterator hIter;
};




  } // end of DataFlow namespace
} // end of OA namespace

#endif*/


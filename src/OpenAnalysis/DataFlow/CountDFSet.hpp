/*! \file
  
  \authors Michelle Strout 
  \version $Id: CountDFSet.hpp,v 1.2 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef CountDFSet_H
#define CountDFSet_H

#include <iostream>
#include <set>
#include <algorithm>
#include <iterator>

// abstract interface that this class implements
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>

namespace OA {
  namespace DataFlow {

//! DataflowSet that consists of a count
class CountDFSet : public virtual DataFlowSet {
public:
  //! Construct a new count set, initialize count to zero.
  CountDFSet();

  //! Construct a new count set with a specified value.
  CountDFSet(int count);

  //! Construct a new count set by copying from an existing set.
  CountDFSet(const CountDFSet &other);

  ~CountDFSet() {}
  OA_ptr<DataFlowSet> clone() const;
  
  // param for these can't be const because will have to 
  // dynamic cast to specific subclass
  bool operator ==(DataFlowSet &other) const;

  bool operator !=(DataFlowSet &other) const;

  void dump(std::ostream &os);

  void dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> ir);

  void output(OA::IRHandlesIRInterface &ir) const;

  int size() const { return 1; }

  void clear() { mCount = 0; }
  
  void setUniversal() { assert(false); }
  
  bool isEmpty() const { return mCount == 0; }

  bool isUniversalSet() const { assert(false); return false; }


protected:
  OUTPUT

  GENOUT int mCount;
};

  } // end of DataFlow namespace
} // end of OA namespace

#endif

/*! \file
  
  \brief One-to-one mapping between formal parameter symbols in callee and
         memory reference handles in caller.

  \authors Michelle Strout 
  \version $Id: ParamBindings.hpp,v 1.4 2005/08/15 19:29:55 utke Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ParamBindings_h
#define ParamBindings_h

#include <iostream>
#include <map>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>
#include <OpenAnalysis/Utils/Iterator.hpp>

namespace OA {
  namespace DataFlow {

class ParamBindIterator;
class ParamBindings;  

//! Iterator over ParamBindings
class ParamBindIterator {
  public:

    ParamBindIterator(std::list<std::pair<OA_ptr<MemRefExpr>, 
                                          OA_ptr<ExprTree> > > paramlist )
      { mPairList = paramlist; mIter = mPairList.begin(); }

    ~ParamBindIterator() {}

    //! right hand side
    OA_ptr<ExprTree> currentActual() const {
        if (isValid()) { return mIter->second; }
    }

    //! left hand side
    OA_ptr<MemRefExpr> currentFormal() const {
        if (isValid()) { return mIter->first; }
    }

    bool isValid() const { return (mIter!=mPairList.end()); }

    void operator++() { if (isValid()) { mIter++; } }

    void reset() { mIter = mPairList.begin(); }

    // construction methods
    void insertParamBindPair(CallHandle call, 
                             OA_ptr<MemRefExpr> formal,
                             OA_ptr<ExprTree> actual)
    {
        mPairList.push_back(std::pair<OA_ptr<MemRefExpr>,
                                      OA_ptr<ExprTree> >(formal, actual));
        reset();
    }

  private:
    std::list<std::pair<OA_ptr<MemRefExpr>, OA_ptr<ExprTree> > > mPairList;
    std::list<std::pair<OA_ptr<MemRefExpr>, OA_ptr<ExprTree> > >::iterator mIter;
};


class ParamBindings : public virtual Annotation {
  
public:
  ParamBindings() { }
  ~ParamBindings() {}

  OA_ptr<ParamBindIterator> getParamBindIterator(CallHandle call);

  void mapFormalToActualForCall(CallHandle call, OA_ptr<MemRefExpr> formal, 
                                OA_ptr<ExprTree> actual);

  void output(IRHandlesIRInterface& ir) const;

private:

  std::map<CallHandle, std::list<std::pair<OA_ptr<OA::MemRefExpr>,
           OA_ptr<OA::ExprTree> > > > mCallToFormalToActual;

  friend class ParamBindIterator;
  
};


  } // end of DataFlow namespace
} // end of OA namespace

#endif

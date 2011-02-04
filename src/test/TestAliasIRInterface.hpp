/*! \file
    
  \brief Test IR interface implementation for Alias analysis.  
        
  \authors Michelle Strout, Andy Stone
  \version $Id: TestAliasIRInterface.hpp,v 1.1.2.2 2006/01/05 22:52:25 mstrout Exp $
 
   Copyright (c) 2005, Colorado State University
   All rights reserved.
 
  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef TestALIASIRInterface_HPP
#define TestALIASIRInterface_HPP

#include <string>
#include <list>
#include <map>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Location/Location.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>

#include <OpenAnalysis/IRInterface/AliasIRInterfaceDefault.hpp>
#include "SubsidiaryIR.hpp"
#include "IRHandleIterators.hpp"

using namespace OA;
using namespace Alias;
using namespace std;

class SubsidiaryIRPtrAssignPairStmtIterator;

/*! 
    Implements the AliasIRInterface using the data stored in SubsidiaryIR.
*/
class TestAliasIRInterface 
    : public virtual AliasIRInterfaceDefault
{ 
  public:
    TestAliasIRInterface(OA_ptr<SubsidiaryIR> subIR) : mSubIR(subIR) {}
    ~TestAliasIRInterface() {}

    //*****************************************************************
    // Implementation of IRHandlesIRInterface
    //*****************************************************************
    virtual string toString(ProcHandle h) 
    { return mSubIR->toString(h); }

    virtual string toString(StmtHandle h) 
    { return mSubIR->toString(h); }

    virtual string toString(MemRefHandle h) 
    { return mSubIR->toString(h); }

    virtual string toString(SymHandle h) 
    { return mSubIR->toString(h); }

    virtual string toString(ExprHandle h) 
    { return mSubIR->toString(h); }

    virtual string toString(ConstSymHandle h) 
    { return mSubIR->toString(h); }

    virtual string toString(ConstValHandle h) 
    { return mSubIR->toString(h); }

    virtual string toString(OpHandle h) 
    { return mSubIR->toString(h); }

    virtual string toString(CallHandle h)
    { return mSubIR->toString(h); }

    virtual void dump(MemRefHandle h, ostream& os)
    { return mSubIR->dump(h,os); }

    virtual void dump(StmtHandle h, ostream& os)
    { mSubIR->dump(h,os); }

    //*****************************************************************
    // Implementation of AliasIRInterface
    //*****************************************************************
    
    //! Given a subprogram return an IRStmtIterator for the entire
    //! subprogram
    OA_ptr<IRStmtIterator> getStmtIterator(ProcHandle h); 
  
    //! Return an iterator over all the memory reference handles that appear
    //! in the given statement.
    OA_ptr<MemRefHandleIterator> getAllMemRefs(StmtHandle stmt);
  
    //! Given a statement, return its IRStmtType
    IRStmtType getAliasStmtType(StmtHandle stmt);

    //! return an iterator over MemRefHandle
    //! pairs where there is a source and target such that target = source
    //! and both reference pointers
    OA_ptr<PtrAssignPairStmtIterator> 
        getPtrAssignStmtPairIterator(StmtHandle stmt);

    //! Given a procedure return associated SymHandle
    SymHandle getSymHandle(ProcHandle h);
  
    //! For the given symbol create a Location that indicates statically
    //! overlapping locations and information about whether the location
    //! is local or not
    OA_ptr<Location::Location> getLocation(ProcHandle proc,
                                           SymHandle s);

    //! Given a MemRefHandle return a list of all 
    //! MemRefExprs that describe this memory reference
    OA_ptr<MemRefExprIterator>
        getMemRefExprIterator(MemRefHandle h);

    OA_ptr<IRProcIterator> getProcIterator();

    void dump(ostream& os);

//! Temporarily added in order to get compilation working
    SymHandle getFormalSym(ProcHandle,int);
    OA_ptr<IRCallsiteIterator> getCallsites(StmtHandle h);
    OA_ptr<MemRefExpr> getCallMemRefExpr(CallHandle h);
    OA_ptr<ParamBindPtrAssignIterator>
      getParamBindPtrAssignIterator(CallHandle call);
    ProcHandle getProcHandle(SymHandle sym);

  private:
    OA_ptr<SubsidiaryIR> mSubIR;
};

class TestIRParamBindPtrAssignIterator
    : public ParamBindPtrAssignIterator
{
  public:
    TestIRParamBindPtrAssignIterator(
        OA_ptr<map<int, OA::OA_ptr<OA::MemRefExpr> > > map)
        : mMap(map) { reset(); }
    ~TestIRParamBindPtrAssignIterator() {}

    OA_ptr<MemRefExpr> currentActual() const {
        assert(isValid());
        return mIter->second;
    }

    int currentFormalId() const {
        assert(isValid());
        return mIter->first;
    }

    void operator++() { if (isValid()) { mIter++; } }

    bool isValid() const {
        return mIter != mMap->end();
    }

    void reset() { mIter = mMap->begin(); }

  private:
    map<int, OA::OA_ptr<OA::MemRefExpr> >::iterator mIter;
    OA_ptr<map<int, OA::OA_ptr<OA::MemRefExpr> > > mMap;
};

class TestIRCallSiteIterator
    : public IRCallsiteIterator
{
  public:
    TestIRCallSiteIterator(OA_ptr<set<OA::CallHandle> > set)
        : mSet(set) { reset(); }
    ~TestIRCallSiteIterator() {}

    OA::CallHandle current() const {
        return *mIter;
    }

    void operator++() { if (isValid()) { mIter++; } }

    bool isValid() const {
        return mIter != mSet->end();
    }

    void reset() { mIter = mSet->begin(); }

  private:
    OA_ptr<set<OA::CallHandle> > mSet;
    set<OA::CallHandle>::iterator mIter;
};

//! Iterator over PtrAssignPairs for the Stmt
class TestIRPtrAssignPairStmtIterator
    : public PtrAssignPairStmtIterator 
{
  public:
    TestIRPtrAssignPairStmtIterator(OA_ptr<set<PtrPair> > aSet) 
        : mSet(aSet) { reset(); }
    ~TestIRPtrAssignPairStmtIterator() {}

    //! right hand side
    OA_ptr<MemRefExpr> currentSource() const { 
        assert(isValid());
        PtrPair pair = *mIter;
        return pair.getSource();
    }

    //! left hand side
    OA_ptr<MemRefExpr> currentTarget() const { 
        assert(isValid());
        PtrPair pair = *mIter;
        return pair.getTarget();
    }

    bool isValid() const {
        return mIter != mSet->end();
    }
    
    void operator++() { if (isValid()) { mIter++; } }

    void reset() { mIter = mSet->begin(); }

  private:
    OA_ptr<set<PtrPair> > mSet;
    set<PtrPair>::iterator mIter;
};

#endif

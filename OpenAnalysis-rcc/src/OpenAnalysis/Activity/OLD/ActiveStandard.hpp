/*! \file
  
  \brief Maps procedures to a set of locations, statements, and memory
         references in the stmt that are active.

  \authors Michelle Strout
  \version $Id: ActiveStandard.hpp,v 1.6 2005/06/10 02:32:01 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ActiveStandard_hpp
#define ActiveStandard_hpp

#include <cassert>
#include <iostream>
#include <list>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
//#include <OpenAnalysis/ReachDefs/Interface.hpp>
//#include <OpenAnalysis/DataFlow/LocDFSet.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSetImpl.hpp>

namespace OA {
  namespace Activity {


class ActiveStandard {
  public:
    ActiveStandard(ProcHandle p);
    ~ActiveStandard() {}

    //! Return an iterator for set of active symbols
    //!FIXME: not really possible unless we know all visible symbols
    //!because could have the Unknown location
    //!be active and under that circumstance all symbols are active
    OA_ptr<SymHandleIterator> getActiveSymIterator();

    //! FIXME: will eventually need a routine to indicate what statements
    //! dynamically allocate memory that is active
    //getActiveUnnamedIterator

    //! Indicate whether the given symbol is active or not
    bool isActive(SymHandle sym);

    //! Indicate whether the given stmt is active or not
    bool isActive(StmtHandle stmt);

    //! Indicate whether the given memref is active or not
    bool isActive(MemRefHandle memref);
    
    //! Return an iterator for set of active stmts
    OA_ptr<StmtHandleIterator> getActiveStmtIterator();

    //! Return an iterator for set of active memory references
    OA_ptr<MemRefHandleIterator> getActiveMemRefIterator();

    //! Get an iterator over active locations
    //OA_ptr<LocIterator> getActiveLocsIterator();
    OA_ptr<OA::DataFlow::DataFlowSetImplIterator<int> > getActiveTagsIterator();

    //*****************************************************************
    // Construction methods 
    //*****************************************************************

    //! insert an active location
    //void insertLoc(OA_ptr<Location> loc);
    void insertTag(int tag);

    //! insert an active Stmt
    void insertStmt(StmtHandle stmt); 

    //! insert an active MemRef
    void insertMemRef(MemRefHandle memref);
    
    //! For construction of InterActive
    //bool getUnknownLocActive() { return mUnknownLocActive; }

    //*****************************************************************
    // Output
    //*****************************************************************

    //! incomplete output of info for debugging, just lists 
    //! the contents of the three active sets
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

  private:
    // data members
    //OA_ptr<DataFlow::LocDFSet>  mActiveLocSet;
    OA_ptr<OA::DataFlow::DataFlowSetImpl<int> > mActiveTagSet;
    OA_ptr<std::set<SymHandle> > mActiveSymSet;
    bool mUnknownLocActive;

    OA_ptr<std::set<StmtHandle> > mActiveStmtSet;
    OA_ptr<std::set<MemRefHandle> > mActiveMemRefSet;

};

//! An iterator over Symbols
class ActiveSymIterator : public virtual OA::SymHandleIterator,
                          public OA::IRHandleSetIterator<OA::SymHandle>
{
public:
  ActiveSymIterator(OA::OA_ptr<std::set<OA::SymHandle> > pList) 
    : OA::IRHandleSetIterator<OA::SymHandle>(pList) {}
  ~ActiveSymIterator() {}

  void operator++() { OA::IRHandleSetIterator<OA::SymHandle>::operator++(); }
  bool isValid() const
    { return OA::IRHandleSetIterator<OA::SymHandle>::isValid(); }
  OA::SymHandle current() const
    { return OA::IRHandleSetIterator<OA::SymHandle>::current(); }
  void reset() { OA::IRHandleSetIterator<OA::SymHandle>::current(); }
};


//! An iterator over Memory References 
class ActiveMemRefIterator : public virtual OA::MemRefHandleIterator,
                             public OA::IRHandleSetIterator<OA::MemRefHandle>
{
public:
  ActiveMemRefIterator(OA::OA_ptr<std::set<OA::MemRefHandle> > pList) 
    : OA::IRHandleSetIterator<OA::MemRefHandle>(pList) {}
  ~ActiveMemRefIterator() {}

  void operator++() { OA::IRHandleSetIterator<OA::MemRefHandle>::operator++(); }
  bool isValid() const
    { return OA::IRHandleSetIterator<OA::MemRefHandle>::isValid(); }
  OA::MemRefHandle current() const
    { return OA::IRHandleSetIterator<OA::MemRefHandle>::current(); }
  void reset() { OA::IRHandleSetIterator<OA::MemRefHandle>::current(); }
};


//! An iterator over stmts 
class ActiveStmtIterator : public virtual OA::IRStmtIterator,
                           public OA::IRHandleSetIterator<OA::StmtHandle>
{
public:
  ActiveStmtIterator(OA::OA_ptr<std::set<OA::StmtHandle> > pList) 
    : OA::IRHandleSetIterator<OA::StmtHandle>(pList) {}
  ~ActiveStmtIterator() {}

  void operator++() { OA::IRHandleSetIterator<OA::StmtHandle>::operator++(); }
  bool isValid() const
    { return OA::IRHandleSetIterator<OA::StmtHandle>::isValid(); }
  OA::StmtHandle current() const
    { return OA::IRHandleSetIterator<OA::StmtHandle>::current(); }
  void reset() { OA::IRHandleSetIterator<OA::StmtHandle>::current(); }
};



  } // end of Activity namespace
} // end of OA namespace

#endif


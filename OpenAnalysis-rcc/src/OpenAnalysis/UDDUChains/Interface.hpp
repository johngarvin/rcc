/*! \file
  
  \brief Contains definition of abstract interface for UDDUChains results.

  \authors Michelle Strout
  \version $Id: Interface.hpp,v 1.8 2004/11/24 03:04:30 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef UDDUChainsInterface_hpp
#define UDDUChainsInterface_hpp

#include <set>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>


namespace OA {
  namespace UDDUChains {

typedef std::set<StmtHandle> StmtSet;

//class StmtUDDUChainsIterator;
//class MemRefUDDUChainsIterator;

//! Abstract interface for UDDUChains results.
class Interface {
  public:
    Interface() {}
    virtual ~Interface() {}

    typedef StmtHandleIterator ChainStmtIterator;
    typedef StmtHandleIterator StmtsWithUDChainIterator;
    typedef StmtHandleIterator StmtsWithDUChainIterator;
    typedef MemRefHandleIterator MemRefsWithUDChainIterator;
    typedef MemRefHandleIterator MemRefsWithDUChainIterator;

    //! Return StmtHandle for a particular MemRefHandle
    //! FIXME: not sure this belongs here, put this in so that
    //! ManagerUDDUChainsXAIF does not need an IRInterface
    //virtual StmtHandle getStmtHandle(MemRefHandle ref) = 0;
    
    
    //! Return an iterator over all statements in this procedure
    //! that may define a location used in the given statement
    virtual OA_ptr<ChainStmtIterator> getUDChainStmtIterator(StmtHandle s) = 0;

    //! Return an iterator over all statements for which UDChain info
    //! is available
    virtual OA_ptr<StmtsWithUDChainIterator> getStmtsWithUDChainIterator() = 0;

    //! Return an iterator over all statements in this procedure
    //! that may use a location defined in the given statement
    virtual OA_ptr<ChainStmtIterator> getDUChainStmtIterator(StmtHandle s) = 0;
    
    //! Return an iterator over all statements for which DUChain info
    //! is available
    virtual OA_ptr<StmtsWithDUChainIterator> getStmtsWithDUChainIterator() = 0;

    //! Return an iterator over all statements in this procedure
    //! that may define a location used by the given memory reference
    virtual OA_ptr<ChainStmtIterator> getUDChainStmtIterator(MemRefHandle s) = 0;

    //! Return an iterator over all use memory references for which UDChain
    //! info is available
    virtual OA_ptr<MemRefsWithUDChainIterator> getMemRefsWithUDChainIterator()=0;

    //! Return an iterator over all statements in this procedure
    //! that may use a location defined by the given memory reference
    virtual OA_ptr<ChainStmtIterator> getDUChainStmtIterator(MemRefHandle s) = 0;

    //! Return an iterator over all def memory references for which DUChain
    //! info is available
    virtual OA_ptr<MemRefsWithDUChainIterator> getMemRefsWithDUChainIterator()=0;

    //*****************************************************************
    // Output
    //*****************************************************************

    virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir) = 0;

};

/*! Iterator over map entries where key is a statement and value
    is a set of statements.
*/
/*
class StmtUDDUChainsIterator {
public:
  StmtUDDUChainsIterator() { }
  virtual ~StmtUDDUChainsIterator() { };

  virtual StmtHandle currentKey() = 0;  // Returns the current key
  virtual OA_ptr<StmtSet> currentVal() = 0;    // Returns the current val
  virtual bool isValid() = 0;           // False when all items are exhausted.
        
  virtual void operator++() = 0;

  virtual void reset() = 0;
};
*/

/*! Iterator over map entries where key is a memory reference and value
    is a set of statements.
*/
/*
class MemRefUDDUChainsIterator {
public:
  MemRefUDDUChainsIterator() { }
  virtual ~MemRefUDDUChainsIterator() { };

  virtual MemRefHandle currentKey() = 0;  // Returns the current key
  virtual OA_ptr<StmtSet> currentVal() = 0;    // Returns the current val
  virtual bool isValid() = 0;           // False when all items are exhausted.
        
  virtual void operator++() = 0;

  virtual void reset() = 0;
};
*/


  } // end of UDDUChains namespace
} // end of OA namespace

#endif


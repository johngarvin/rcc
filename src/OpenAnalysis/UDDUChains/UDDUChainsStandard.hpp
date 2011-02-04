/*! \file
  
  \brief Definition of UDDUChainsStandard
  
  \authors Michelle Strout
  \version $Id: UDDUChainsStandard.hpp,v 1.13 2004/11/25 04:22:10 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef UDDUChainsStandard_hpp
#define UDDUChainsStandard_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/UDDUChains/Interface.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>



namespace OA {
  namespace UDDUChains {

//typedef std::set<StmtHandle> StmtSet;


/*!
  UD chains map each StmtHandle X to the set of StmtHandle's that
  may include a define for a use within X.
  DU chains map each StmtHandle Y to the set of StmtHandle's that
  may include a use of a define in Y.
*/
class UDDUChainsStandard : public virtual UDDUChains::Interface, public virtual Annotation
{

public:
    UDDUChainsStandard(ProcHandle p) 
        { mUDChainForStmt = new std::map<StmtHandle,OA_ptr<StmtSet> >;
          mDUChainForStmt = new std::map<StmtHandle,OA_ptr<StmtSet> >;
          mUDChainForMemRef = new std::map<MemRefHandle,OA_ptr<StmtSet> >;
          mDUChainForMemRef = new std::map<MemRefHandle,OA_ptr<StmtSet> >;
        } 
    ~UDDUChainsStandard() {}

    //! Return StmtHandle for a particular MemRefHandle
    //! FIXME: not sure this belongs here, put this in so that
    //! ManagerUDDUChainsXAIF does not need an IRInterface
    //StmtHandle getStmtHandle(MemRefHandle ref)
    //{ return mMemRefToStmtMap[ref]; }
    
    //! Return an iterator over all statements in this procedure
    //! that may define a location used in the given statement
    OA_ptr<Interface::ChainStmtIterator> getUDChainStmtIterator(StmtHandle s);

    //! Return an iterator over all statements for which UDChain info
    //! is available
    OA_ptr<Interface::StmtsWithUDChainIterator> getStmtsWithUDChainIterator();

    //! Return an iterator over all statements in this procedure
    //! that may use a location defined in the given statement
    OA_ptr<Interface::ChainStmtIterator> getDUChainStmtIterator(StmtHandle s);

    //! Return an iterator over all statements for which DUChain info
    //! is available
    OA_ptr<Interface::StmtsWithDUChainIterator> getStmtsWithDUChainIterator();

    //! Return an iterator over all statements in this procedure
    //! that may define a location used by the given memory reference
    OA_ptr<Interface::ChainStmtIterator> getUDChainStmtIterator(MemRefHandle h);

    //! Return an iterator over all memory references for which UDChain
    //! info is available
    OA_ptr<Interface::MemRefsWithUDChainIterator> 
    getMemRefsWithUDChainIterator();

    //! Return an iterator over all statements in this procedure
    //! that may use a location defined by the given memory reference
    OA_ptr<Interface::ChainStmtIterator> getDUChainStmtIterator(MemRefHandle h);

    //! Return an iterator over all memory references for which DUChain
    //! info is available
    OA_ptr<Interface::MemRefsWithDUChainIterator> 
    getMemRefsWithDUChainIterator();

    //*****************************************************************
    // Construction methods 
    // FIXME: the relationships between these aren't being verified
    // or asserted
    //*****************************************************************

    //! insert a memref, the memref may end up having an empty
    //! chains so must make sure chain is initialized
    void insertMemRefDef(MemRefHandle ref);
    void insertMemRefUse(MemRefHandle ref);

    //! insert a define use relationship
    void insertDefUse(StmtHandle def, StmtHandle use);

    //! insert a define use relationship with the specific def
    //! memory reference
    void insertMemRefDefStmtUse(MemRefHandle def, StmtHandle use);

    //! insert a define use relationship with the specific use
    //! memory reference
    void insertStmtDefMemRefUse(StmtHandle def, MemRefHandle use);

    //! insert a mem ref to stmt relationship
    //void insertMemRefToStmtMapping(MemRefHandle ref, StmtHandle stmt)
    //{ mMemRefToStmtMap[ref] = stmt; }

    //*****************************************************************
    // Output
    //*****************************************************************

    //! will use OutputBuilder to generate output
    void output(IRHandlesIRInterface& pIR) const;
    
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

  private:
    // data members
    OUTPUT
    
    GENOUT OA_ptr<std::map<StmtHandle,OA_ptr<StmtSet> > > mUDChainForStmt;
    GENOUT OA_ptr<std::map<StmtHandle,OA_ptr<StmtSet> > > mDUChainForStmt;
    GENOUT OA_ptr<std::map<MemRefHandle,OA_ptr<StmtSet> > > mUDChainForMemRef;
    GENOUT OA_ptr<std::map<MemRefHandle,OA_ptr<StmtSet> > > mDUChainForMemRef;
    GENOUT std::map<MemRefHandle,StmtHandle>  mMemRefToStmtMap;

  public:

    //*****************************************************************
    // Iterators
    //*****************************************************************
    /*! Inherits interface and postfix implementation from 
        Interface::ChainStmtIterator and gets rest of implementation
        from IRHandleSetIterator<StmtHandle>
    */
    class ChainStmtIterator : public IRHandleSetIterator<StmtHandle>,
                              public Interface::ChainStmtIterator
    {
      public:
        ChainStmtIterator(OA_ptr<std::set<StmtHandle> > pSet) 
            : IRHandleSetIterator<StmtHandle>(pSet) {}
        ~ChainStmtIterator() {}

        void operator++() { IRHandleSetIterator<StmtHandle>::operator++(); }
        bool isValid() const
          { return IRHandleSetIterator<StmtHandle>::isValid(); }
        StmtHandle current() const
          { return IRHandleSetIterator<StmtHandle>::current(); }
        void reset() { IRHandleSetIterator<StmtHandle>::current(); }
    };

    class StmtsWithUDChainIterator : public Interface::StmtsWithUDChainIterator 
    {
      public:
        StmtsWithUDChainIterator(OA_ptr<std::map<StmtHandle,
                                                OA_ptr<StmtSet> > > pMap) 
            : mMap(pMap), mIter(pMap->begin()) { }
        ~StmtsWithUDChainIterator() { }

        StmtHandle current() const { return mIter->first; }
        bool isValid() const { return (mIter != mMap->end()); }
        void operator++() { if (isValid()) mIter++; }
        void reset() { mIter = mMap->begin(); }

      private:
        OA_ptr<std::map<StmtHandle,OA_ptr<StmtSet> > > mMap;
        std::map<StmtHandle,OA_ptr<StmtSet> >::iterator mIter;

    };

    //! has same implementation just different name to indicate semantics
    typedef StmtsWithUDChainIterator StmtsWithDUChainIterator;

    class MemRefsWithUDChainIterator 
        : public Interface::MemRefsWithUDChainIterator 
    {
      public:
        MemRefsWithUDChainIterator(OA_ptr<std::map<MemRefHandle,
                                                OA_ptr<StmtSet> > > pMap) 
            : mMap(pMap), mIter(pMap->begin()) { }
        ~MemRefsWithUDChainIterator() { }

        MemRefHandle current() const { return mIter->first; }
        bool isValid() const { return (mIter != mMap->end()); }
        void operator++() { if (isValid()) mIter++; }
        void reset() { mIter = mMap->begin(); }

      private:
            
        OA_ptr<std::map<MemRefHandle,OA_ptr<StmtSet> > > mMap;
        std::map<MemRefHandle,OA_ptr<StmtSet> >::iterator mIter;
    };

    //! has same implementation just different name to indicate semantics
    typedef MemRefsWithUDChainIterator MemRefsWithDUChainIterator;
};

  } // end of UDDUChains namespace
} // end of OA namespace

#endif


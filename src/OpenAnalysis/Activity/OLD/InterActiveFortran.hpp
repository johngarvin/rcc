/*! \file
  
  \brief Maps procedures to a set of locations, statements, and memory
         references in the stmt that are active.

  \authors Michelle Strout
  \version $Id: InterActive.hpp,v 1.6 2005/06/10 02:32:02 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef InterActiveFortran_hpp
#define InterActiveFortran_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/DataFlow/LocDFSet.hpp>
#include <OpenAnalysis/Activity/ActiveStandard.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>

namespace OA {
  namespace Activity {


    class InterActiveFortran : public virtual Annotation {
  public:
    InterActiveFortran(); 
    
    ~InterActiveFortran();

    //! Return an iterator over all procedures for which activity
    //! analysis is available
    OA_ptr<ProcHandleIterator> getKnownProcIterator();

    //! Indicate whether the given symbol is active or not within any procedure
    bool isActive(SymHandle sym);


    //! Indicate whether the given stmt is active or not within given proc
    bool isActive(ProcHandle proc, StmtHandle stmt);

    //! Indicate whether the given memref is active or not within given proc
    bool isActive(ProcHandle proc, MemRefHandle memref);


    //! Set the given symbol as active
    void setActive(SymHandle sym);

    //! Return an iterator for set of active symbols
    //! Used to determine size in bytes of active symbols
    //! WARNING: not list of all active symbols because if unknown loc is active
    //! then all symbols are active but won't all show up in this iterator
    OA_ptr<SymHandleIterator> getActiveSymIterator();

    OA_ptr<MemRefHandleIterator> getActiveMemRefIterator(ProcHandle proc);

    OA_ptr<StmtHandleIterator> getActiveStmtIterator(ProcHandle proc);

    //! Associate the given procedure with the given Activity info
    void mapProcToActive(ProcHandle p, OA_ptr<Activity::ActiveStandard> active);

    //! Return the number of iterations over nodes in Useful
    int getNumIterUseful();

    //! Return the number of iterations over nodes in Vary
    int getNumIterVary();

    //! Return the number of iterations over nodes in Active
    int getNumIterActive();

    //! Return the size of active symbols in bytes
    int getActiveSizeInBytes();

    //! Set the number of iterations over nodes in Useful
    void setNumIterUseful(int n);

    //! Set the number of iterations over nodes in Useful
    void setNumIterVary(int n);

    //! Set the number of iterations over nodes in Active
    void setNumIterActive(int n);

    //! Set the size of active symbols in bytes
    void setActiveSizeInBytes(int size);

    //*****************************************************************
    // Output
    //*****************************************************************
    void output(IRHandlesIRInterface &ir);

    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

  private:
    // mapping of procedures to activity results
    std::map<ProcHandle,OA_ptr<Activity::ActiveStandard> > mProcToActiveMap;

    // set of active symbols from all procedures
    std::set<SymHandle> mActiveSymSet;
    bool mUnknownLocActive;
    int mSizeInBytes;

    // numIters:  number of iterations over nodes
    int mNumIterUseful;
    int mNumIterVary;
    int mNumIterActive;

    //! Iterator over procedures in the mProcToActiveMap
    class ActivityProcIter : public virtual ProcHandleIterator,
                             public IRHandleSetIterator<ProcHandle>
    {
      public:
        ActivityProcIter(OA_ptr<std::set<ProcHandle> > pSet )
            : IRHandleSetIterator<ProcHandle>(pSet) {}
        ~ActivityProcIter() { }
    
        ProcHandle current() const
          { return IRHandleSetIterator<ProcHandle>::current(); }
        bool isValid() const  
          { return IRHandleSetIterator<ProcHandle>::isValid(); }
        void operator++() { IRHandleSetIterator<ProcHandle>::operator++(); }
        void reset() { IRHandleSetIterator<ProcHandle>::reset(); }
    };


};

  } // end of Activity namespace
} // end of OA namespace

#endif


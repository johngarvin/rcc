/*! \fil
  
  \brief Contains definition of abstract interface for Interprocedural
         SideEffect results.

  \authors Michelle Strout, Andy Stone (alias tag update)
  \version $Id: InterSideEffectInterface.hpp,v 1.4 2005/03/17 21:47:46 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef InterSideEffectInterface_hpp
#define InterSideEffectInterface_hpp

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/SideEffect/SideEffectInterface.hpp>


namespace OA {
  namespace SideEffect {

typedef ProcHandleIterator ProcIterator;

//! Abstract interface for InterSideEffect results.
class InterSideEffectInterface {
  public:
    InterSideEffectInterface() {}
    virtual ~InterSideEffectInterface() {}

    //! Return an iterator over all procedures for which side
    //! effect information is available 
    virtual OA_ptr<ProcIterator> getKnownProcIterator() = 0;

    //*************************************************************************
    
    //! Return an iterator over all alias tags that may be
    //! modified locally within the called procedure.  
    //! tags modified directly in the called procedure and not
    //! by a function call in the called procedure.
    virtual OA_ptr<Alias::AliasTagIterator> getLMODIterator(CallHandle p) = 0;

    //! Return an iterator over all tags that are may be
    //! modified within the called procedure or within a procedure
    //! called by the called procedure.  
    virtual OA_ptr<Alias::AliasTagIterator> getMODIterator(CallHandle p) = 0;

    //! Return an iterator over all tags that are definitely
    //! modified locally within the called procedure.  
    //! tags modified directly in the procedure and not
    //! by a function call in the procedure.
    virtual OA_ptr<Alias::AliasTagIterator> getLDEFIterator(CallHandle p) = 0;

    //! Return an iterator over all tags that are definitely
    //! modified within the called procedure or within a procedure
    //! called by the called procedure.  
    virtual OA_ptr<Alias::AliasTagIterator> getDEFIterator(CallHandle p) = 0;

    //! Return an iterator over all tags that are 
    //! used locally within the called procedure before being
    //! definitely modified.  
    //! tags used directly in the procedure and not
    //! by a function call in the procedure.
    virtual OA_ptr<Alias::AliasTagIterator> getLUSEIterator(CallHandle p) = 0;

    //! Return an iterator over all tags that may be 
    //! used before being defined within the called procedure or 
    //! within a procedure called by the called procedure.  
    virtual OA_ptr<Alias::AliasTagIterator> getUSEIterator(CallHandle p) = 0;

    //! Return an iterator over all tags that are 
    //! used locally within the called procedure.
    //! tags used directly in the procedure and not
    //! by a function call in the procedure.
    virtual OA_ptr<Alias::AliasTagIterator> getLREFIterator(CallHandle p) = 0;

    //! Return an iterator over all tags that are 
    //! used within the called procedure or within a procedure
    //! called by the called procedure.  
    virtual OA_ptr<Alias::AliasTagIterator> getREFIterator(CallHandle p) = 0;

    //*************************************************************************
    
    //! Return an iterator over all tags that are may be
    //! modified locally within the given procedure.  
    //! tags modified directly in the procedure and not
    //! by a function call in the procedure.
    virtual OA_ptr<Alias::AliasTagIterator> getLMODIterator(ProcHandle p) = 0;

    //! Return an iterator over all tags that are may be
    //! modified within the given procedure or within a procedure
    //! called by the given procedure.  
    virtual OA_ptr<Alias::AliasTagIterator> getMODIterator(ProcHandle p) = 0;

    //! Return an iterator over all tags that are definitely
    //! modified locally within the given procedure.  
    //! tags modified directly in the procedure and not
    //! by a function call in the procedure.
    virtual OA_ptr<Alias::AliasTagIterator> getLDEFIterator(ProcHandle p) = 0;

    //! Return an iterator over all tags that are definitely
    //! modified within the given procedure or within a procedure
    //! called by the given procedure.  
    virtual OA_ptr<Alias::AliasTagIterator> getDEFIterator(ProcHandle p) = 0;

    //! Return an iterator over all tags that are 
    //! used locally within the given procedure before being
    //! definitely modified.  
    //! tags used directly in the procedure and not
    //! by a function call in the procedure.
    virtual OA_ptr<Alias::AliasTagIterator> getLUSEIterator(ProcHandle p) = 0;

    //! Return an iterator over all locations that may be 
    //! used before being defined within the given procedure or 
    //! within a procedure called by the given procedure.  
    virtual OA_ptr<Alias::AliasTagIterator> getUSEIterator(ProcHandle p) = 0;

    //! Return an iterator over all tags that are 
    //! used locally within the given procedure.
    //! tags used directly in the procedure and not
    //! by a function call in the procedure.
    virtual OA_ptr<Alias::AliasTagIterator> getLREFIterator(ProcHandle p) = 0;

    //! Return an iterator over all tags that are 
    //! used within the given procedure or within a procedure
    //! called by the given procedure.  
    virtual OA_ptr<Alias::AliasTagIterator> getREFIterator(ProcHandle p) = 0;

    //*****************************************************************
    // Output
    //*****************************************************************

    virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir) = 0;

};


  } // end of SideEffect namespace
} // end of OA namespace

#endif


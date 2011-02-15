/*! \file
  
  \brief Contains definition of abstract interface for SideEffect results.

  \authors Michelle Strout, Andy Stone (alias tag update)
  \version $Id: SideEffectInterface.hpp,v 1.4 2005/03/17 21:47:46 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef SideEffectInterface_hpp
#define SideEffectInterface_hpp

#include <set>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Alias/AliasTagSet.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>

namespace OA {
  namespace SideEffect {

//! Abstract interface for SideEffect results.
class Interface {
  public:
    Interface() {}
    virtual ~Interface() {}

    //*****************************************************************
    // Iterators
    //*****************************************************************

    //! Return an iterator over all tags that are may be
    //! modified locally.
    //! tags modified directly in the procedure and not
    //! by a function call in the procedure.
    virtual OA_ptr<Alias::AliasTagIterator> getLMODIterator() = 0;

    //! Return an iterator over all tags that are may be
    //! modified within the procedure or within a procedure
    //! called by the procedure.  
    virtual OA_ptr<Alias::AliasTagIterator> getMODIterator() = 0;

    //! Return an iterator over all tags that are definitely
    //! modified locally within the procedure.  
    //! tags modified directly in the procedure and not
    //! by a function call in the procedure.
    virtual OA_ptr<Alias::AliasTagIterator> getLDEFIterator() = 0;

    //! Return an iterator over all tags that are definitely
    //! modified within the procedure or within a procedure
    //! called by the procedure.  
    virtual OA_ptr<Alias::AliasTagIterator> getDEFIterator() = 0;

    //! Return an iterator over all tags that are 
    //! used locally within the procedure before being
    //! definitely modified.  
    //! tags used directly in the procedure and not
    //! by a function call in the procedure.
    virtual OA_ptr<Alias::AliasTagIterator> getLUSEIterator() = 0;

    //! Return an iterator over all tags that may be 
    //! used before being defined within the procedure or 
    //! within a procedure called by the procedure.  
    virtual OA_ptr<Alias::AliasTagIterator> getUSEIterator() = 0;

    //! Return an iterator over all tags that are 
    //! used locally within the procedure.
    //! tags used directly in the procedure and not
    //! by a function call in the procedure.
    virtual OA_ptr<Alias::AliasTagIterator> getLREFIterator() = 0;

    //! Return an iterator over all tags that are 
    //! used within the procedure or within a procedure
    //! called by the procedure.  
    virtual OA_ptr<Alias::AliasTagIterator> getREFIterator() = 0;

    virtual OA_ptr<MemRefExprIterator> getLMODMREIterator() = 0;
    virtual OA_ptr<MemRefExprIterator> getMODMREIterator() = 0;
    virtual OA_ptr<MemRefExprIterator> getLDEFMREIterator() = 0;
    virtual OA_ptr<MemRefExprIterator> getDEFMREIterator() = 0;
    virtual OA_ptr<MemRefExprIterator> getLUSEMREIterator() = 0;
    virtual OA_ptr<MemRefExprIterator> getUSEMREIterator() = 0;
    virtual OA_ptr<MemRefExprIterator> getLREFMREIterator() = 0;
    virtual OA_ptr<MemRefExprIterator> getREFMREIterator() = 0;

    //*****************************************************************
    // Output
    //*****************************************************************
    virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir) = 0;
};


  } // end of SideEffect namespace
} // end of OA namespace

#endif


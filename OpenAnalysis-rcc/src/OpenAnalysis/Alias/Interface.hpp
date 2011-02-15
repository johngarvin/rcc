/*! \file
  
  \brief Declarations for the abstract Alias interface.

  \authors Michelle Strout

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef AliasInterface_H
#define AliasInterface_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/Alias/AliasTagSet.hpp>

namespace OA {
  namespace Alias {

typedef enum {
    NOALIAS,     
    MAYALIAS,  
    MUSTALIAS   
} AliasResultType;


class Interface {
  public:
    Interface() {}
    virtual ~Interface() {};

    //! indicate the aliasing relationship between two memory references
    virtual AliasResultType alias(MemRefHandle ref1, 
                                  MemRefHandle ref2) = 0;
       
    //! Return an iterator over alias tags that represent locations the 
    //! given memory reference or memory reference expression may reference
    
    //virtual OA_ptr<TagIterator> getAliasTags( OA_ptr<MemRefExpr> mre, 
    //                    CallContext context, SymHandle stmt ) = 0;
    
    virtual OA_ptr<AliasTagSet> getAliasTags( OA_ptr<MemRefExpr> mre )=0;

    //virtual OA_ptr<TagIterator> getAliasTags( MemRefHandle memref,
    //                    CallContext context, SymHandle stmt ) = 0;
                
    virtual OA_ptr<AliasTagSet> getAliasTags( MemRefHandle memref )=0;

    //! Given an AliasTag, returns an iterator over MemRefExprs that 
    //! may access the given tag.
    virtual OA_ptr<MemRefExprIterator>
        getMemRefExprIterator(AliasTag tag) const = 0;

    virtual AliasTag getMaxAliasTag()=0;

    //virtual OA_ptr<MemRefExpr> getMemRefExpr(AliasTag tag)=0;
   
    //! get iterator over all memory references that information is
    //! available for
    // FIXME: MMS 12/16/07, is this used anymore?
    //virtual OA_ptr<MemRefIterator> getMemRefIter() = 0;

    virtual void output(OA::IRHandlesIRInterface& ir) const = 0;
    
};

//typedef MemRefHandleIterator MemRefIterator;

  } // end of Alias namespace
} // end of OA namespace

#endif


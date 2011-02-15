/*! \file
  
  \brief Declarations for the abstract Alias interface.

  \authors Michelle Strout
  \version $Id: Interface.hpp,v 1.12.10.1 2005/08/23 18:19:14 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef AliasInterface_H
#define AliasInterface_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Location/Locations.hpp>

namespace OA {
  namespace Alias {

/* Mapping of Location to following enum sets
 * LocSubSet                        => 1
 * InvisibleLoc(MRE: not SubSetRef) => 2
 * InvisibleLoc(MRE: SubSetRef)     => 1 
 * NamedLoc                         => 2
 * UnnamedLoc                       => 1
 * Multiple Locations mapped 
   to the same set                  => 1
*/
      
typedef enum {
    NOALIAS,     
    MAYALIAS,  
    MUSTALIAS   
} AliasResultType;

typedef MemRefHandleIterator MemRefIterator;


class Interface {
  public:
    Interface() {}
    virtual ~Interface() {};

    //! indicate the aliasing relationship between two memory references
    virtual AliasResultType alias(MemRefHandle ref1, 
                                  MemRefHandle ref2) = 0;
       
    //! iterator over locations that a memory reference may reference
    virtual OA_ptr<LocIterator> getMayLocs(MemRefHandle ref) = 0;

    //! the location a memory reference must reference, if there is 
    //! any ambiguity a NULL location will be returned
    virtual OA_ptr<LocIterator> getMustLocs(MemRefHandle ref) = 0;

    //! iterator over locations that a memory refer expression may reference
    //! why do I need proc here?
    virtual OA_ptr<LocIterator> getMayLocs(MemRefExpr &ref, ProcHandle proc) = 0;

    //! iterator over locations that a memory refer expression may reference
    virtual OA_ptr<LocIterator> getMustLocs(MemRefExpr &ref, ProcHandle proc) = 0;

    //! get iterator over all must aliases for a specific mem ref
    //FIXME: Deprecate
    //virtual OA_ptr<MemRefIterator> getMustAliases(MemRefHandle ref) = 0;

    //! get iterator over all may aliases for a specific mem ref
    //FIXME: Deprecate
    //virtual OA_ptr<MemRefIterator> getMayAliases(MemRefHandle ref) = 0;
    
    //! get iterator over all must aliases for a specific location
    //FIXME: Deprecate
    //virtual OA_ptr<MemRefIterator> 
    //    getMustAliases(OA_ptr<Location> loc) = 0;

    //! get iterator over all may aliases for a specific location
    //FIXME: Deprecate
    //virtual OA_ptr<MemRefIterator> 
    //    getMayAliases(OA_ptr<Location> loc) = 0;
    
    //! get iterator over all memory references that information is
    //! available for
    virtual OA_ptr<MemRefIterator> getMemRefIter() = 0;

};

  } // end of Alias namespace
} // end of OA namespace

#endif


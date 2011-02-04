/*! \file
  
  \brief The AnnotationManager that generates EquivSets by putting
         all local varials that don't have their address taken into
         their own equivalence set and by putting all other memory references
         into one equivalence set.

  \authors Michelle Strout
  \version $Id: ManagerNoAddressOf.cpp,v 1.19 2005/03/17 21:47:45 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerNoAddressOf.hpp"


namespace OA {
  namespace Alias {

static bool debug = false;

/*!
*/
OA_ptr<Alias::EquivSets> ManagerNoAddressOf::performAnalysis(ProcHandle proc) 
{
  if (debug) {
      std::cout << "In ManagerNoAddressOf::performAnalysis:" << std::endl;
  }

  // Initialize EquivSets by creating an empty set that will contain
  // all memory locations but the locals
  OA_ptr<EquivSets> retEquivSets; retEquivSets = new Alias::EquivSets(proc);
  int bigSetID = retEquivSets->makeEmptySet();

  // FIXME: here need to call AccessLoc::getAccessibleLocIterator
  // on a passed in OA_ptr<AccessLoc> and put all accessible
  // locations that are not local into the big equivalence set
  // Put each local in own equivalence set.

  if (debug) {
    std::cout << "\tequivSets so far = ";
    retEquivSets->dump(std::cout,mIR);
  }

  // for each statement stmt
  OA_ptr<OA::IRStmtIterator> sItPtr = mIR->getStmtIterator(proc);
  for ( ; sItPtr->isValid(); (*sItPtr)++) {
    OA::StmtHandle stmt = sItPtr->current();

    if (debug) {
        std::cout << "\tstmt = ";
        mIR->dump(stmt,std::cout);
    }
    
    //   for each memory reference in the stmt
    OA_ptr<MemRefHandleIterator> mrIterPtr = mIR->getAllMemRefs(stmt);
    for (; mrIterPtr->isValid(); (*mrIterPtr)++ )
    {
      MemRefHandle memref = mrIterPtr->current();
      if (debug) {
        std::cout << "\tmemref = ";
        mIR->dump(memref,std::cout);
      }

      // get the memory reference expressions for this handle
      OA_ptr<MemRefExprIterator> mreIterPtr 
          = mIR->getMemRefExprIterator(memref);
      
      // for each mem-ref-expr associated with this memref
      for (; mreIterPtr->isValid(); (*mreIterPtr)++) {
        OA_ptr<OA::MemRefExpr> mre = mreIterPtr->current();
        if (debug) {
            std::cout << "\tmre = ";
            mre->dump(std::cout,mIR);
        }

        // see if we have not already mapped an equivalent memrefexpr
        int setId = retEquivSets->getEquivSetId(mre);
        if (debug) {
            std::cout << "\tsetId after getEquivSetId(mre) = " << setId 
                      << std::endl;
        }

        // need to determine what location mre maps to 
        if (setId == EquivSets::SET_ID_NONE ) {
            
            setId = bigSetID;

            // only handling case where is a named local with  full or
            // partial accuracy, trivialMREToLoc may return
            // a LocSubSet
            // FIXME: make trivialMREToLoc a method of the Manager as
            // was done in ManagerAliasMapBasic, because the Manager
            // can query the AliasIRInterface for the Location associated
            // with a symbol and we won't have to save them separately
            OA_ptr<Location> loc 
                = retEquivSets->trivialMREToLoc(mre);
            if (debug) {
              std::cout << "\tafter trivialMREToLoc, loc = ";
              loc->dump(std::cout,mIR);
            }

            if (!loc.ptrEqual(NULL) && loc->isLocal()) {
                setId = retEquivSets->getEquivSetId(loc);
                if (debug) {
                    std::cout << "\tsetId after getEquivSetId(loc) = " 
                              << setId << std::endl;
                }


       
                // if we didn't find the location in the equiv
                // sets then we need to put the location in an
                // existing equiv set with the same base location
                if (setId==EquivSets::SET_ID_NONE && loc->isaSubSet()) {
                    // get base location from locsubset
                    OA_ptr<Location> base = loc->getBaseLoc();

                    // determine which equivSet that base is in
                    setId = retEquivSets->getEquivSetId(base);
                    assert(setId!=EquivSets::SET_ID_NONE);

                    // put the subset into the same equiv set
                    retEquivSets->addLocation(loc,setId);

                } else if (setId==EquivSets::SET_ID_NONE) {
                    setId = bigSetID;
                }

            } 
        }
          
        // setId at this point is where the location currently is
        // if the memory reference involves an addressOf and
        // the location is in a set other than the bigSetID
        // then we need to merge setId with bigSetID and
        // remap all memory references that pointed to setId
        // to bigSetID
        if (setId!=bigSetID && mre->hasAddressTaken() )
        {
            // merge the setId with the bigset
            retEquivSets->mergeInto(setId,bigSetID);
                    
            // FIXME: find all memrefs and mre's that map to the
            // setId and make them map to bigSetID

        }
        // map mre to set
        retEquivSets->mapMemRefToEquivSet(mre,setId);
        // map memref to mre and memref to set
        retEquivSets->mapMemRefToMemRefExpr(memref,mre);
        retEquivSets->mapMemRefToEquivSet(memref,setId);

      } // loop over memref expressions
    } // loop over memory references
  } // loop over statements

  return retEquivSets;

}
   

  } // end of namespace Alias
} // end of namespace OA

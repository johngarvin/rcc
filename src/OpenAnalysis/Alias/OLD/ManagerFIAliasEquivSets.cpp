/*! \file
  
  \brief The AnnotationManager that generates EquivSets in a
         context and flow insensitive way

  \authors Michelle Strout, Brian White
  \version $Id: ManagerFIAlias.cpp,v 1.1.2.20 2006/01/18 15:38:41 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerFIAliasEquivSets.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace Alias {

static bool debug = false;

/*!
*/
ManagerFIAliasEquivSets::ManagerFIAliasEquivSets(OA_ptr<AliasIRInterface> _ir) 
   : ManagerFIAlias(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerFIAliasEquivSets:ALL", debug);
}

OA_ptr<Alias::EquivSets> 
ManagerFIAliasEquivSets::performAnalysis( OA_ptr<IRProcIterator> procIter)
{

    // Invoke the ManagerFIAlias routine to create the alias sets
    // within the union-find universe.
    OA_ptr<UnionFindUniverse> ufset = performFIAlias(procIter);

    // Convert the uion-find universe to equiv sets.
    return buildEquivSets(ufset,procIter);
}

OA_ptr<Alias::EquivSets> 
ManagerFIAliasEquivSets::buildEquivSets( OA_ptr<UnionFindUniverse> ufset,
                                         OA_ptr<IRProcIterator> procIter )
{
    if (debug) { std::cout << "=========== Building EquivSets ========" 
                           << std::endl; }

    // create an empty EquivSets
    OA_ptr<EquivSets> equivSets;
    equivSets = new EquivSets();

    /////////////// build EquivSets
    // loop over all the mres and get a set of mres for each equiv set
    std::map<int,int> equivSetIdToNumLocs;
    std::map<int,bool> equivSetIdHasMemRefHandle; // do any map to equiv set 
    std::map<int,std::set<OA_ptr<MemRefExpr> > > ufSetToMREs;
    std::map<OA_ptr<MemRefExpr>,int>::iterator mreMapIter;
    for (mreMapIter=mMREToID.begin(); mreMapIter!=mMREToID.end(); mreMapIter++ )
    {
        OA_ptr<MemRefExpr> mre = mreMapIter->first;

        // only map those MREs that do not involve an addressOf operation
        if(mre->isaRefOp()) {
           OA_ptr<RefOp> refop = mre.convert<RefOp>(); 
           if(refop->isaAddressOf()) { continue; }
        }   

        int equivSetID = ufset->Find(mMREToID[mre]);
        ufSetToMREs[equivSetID].insert(mre);
        equivSetIdToNumLocs[equivSetID] = 0;
        equivSetIdHasMemRefHandle[equivSetID] = false; // init, see below
    }

    // iterate over all of the mem ref handles
    // with the goal being to map the mem ref handles to equiv sets
    // and a procedure
    std::map<MemRefHandle,ProcHandle>::iterator refprocIter;
    for (refprocIter=mMemRefHandleToProc.begin();
         refprocIter!=mMemRefHandleToProc.end(); refprocIter++ )
    {
        MemRefHandle memref = refprocIter->first;
        ProcHandle proc = refprocIter->second;

        equivSets->mapMemRefToProc(memref,proc);

        // loop over MREs for the given memref
        OA_ptr<MemRefExprIterator> mreIterPtr 
            = mIR->getMemRefExprIterator(memref);
      
        // for each mem-ref-expr associated with this memref
        for (; mreIterPtr->isValid(); (*mreIterPtr)++) {
            OA_ptr<OA::MemRefExpr> mre = mreIterPtr->current();

            // only map those MREs that do not involve an addressOf operation

            if(mre->isaRefOp()) {
                  OA_ptr<RefOp> refop = mre.convert<RefOp>();
                 if(refop->isaAddressOf()) { continue; }
            }
            
            // map the mem ref handle to the equivset that mre is in
            int ufsetID = ufset->Find(mMREToID[mre]);
            equivSets->mapMemRefToEquivSet(memref, ufsetID);

            // also assign the mre to the set within the alias map.
            // we do this so we can efficiently implement getMayLocs(MRE).
            equivSets->mapMemRefToEquivSet(mre, ufsetID);
            
            // indicate that this equiv set has at least one
            // memref mapping to it
            equivSetIdHasMemRefHandle[ufsetID] = true;
            if (debug) {
                std::cout << "ufsetID = " << ufsetID 
                          << " is mapped to with " << mIR->toString(memref)
                          << std::endl;
            }
        }
    }

    // iterate over all procedures and determine the set of fixed
    // locations for each equiv set within each procedure
    for (procIter->reset();  procIter->isValid(); (*procIter)++ ) {
      ProcHandle proc = procIter->current();
      if (debug) {
        std::cout << "proc = " << mIR->toString(proc) << std::endl;
      }

      std::map<int,std::set<OA_ptr<MemRefExpr> > >::iterator ufsetIter;
      for (ufsetIter=ufSetToMREs.begin(); ufsetIter!=ufSetToMREs.end();
           ufsetIter++)
      {
        int equivSetID = ufsetIter->first;
        if (debug) {
          std::cout << "\tequivSetID = " << equivSetID << std::endl;
        }

        // if no MemRefHandles map to this equivSet, then we do
        // not want to generate a set of locations for it
        if (equivSetIdHasMemRefHandle[equivSetID]==false) {
            continue;
        }

        // loop through all MREs for this ufset/equivset and determine
        // if any of the MREs appear in the current procedure
        // if not then do not want to make a set of locations
        // associated with this procedure and equivset
        bool foundProc = false;
        std::set<OA_ptr<MemRefExpr> > mreSet = ufsetIter->second;
        std::set<OA_ptr<MemRefExpr> >::iterator mreSetIter;
        for (mreSetIter=mreSet.begin(); mreSetIter!=mreSet.end();
             mreSetIter++ )
        {
          OA_ptr<MemRefExpr> mre; mre = *mreSetIter;
          if (debug) {
            std::cout << "\t\tmre = ";
            mre->output(*mIR);
          }
          if (mMREToProcs[mre].find(proc) != mMREToProcs[mre].end() ) {
            foundProc = true;
            break;
          }
        }
           
        // determine the set of locs relevant to this proc
        if (foundProc) {

          bool changed = true;

          while(changed) {

            changed = false;

            for (mreSetIter=mreSet.begin(); mreSetIter!=mreSet.end();
                 mreSetIter++ )
            {
              OA_ptr<MemRefExpr> mre; mre = *mreSetIter;
              if (debug) {
                std::cout << "\t\t\tfoundProc, mre = ";
                mre->output(*mIR);
              }
  
              OA_ptr<LocIterator> locIterator = 
                equivSets->getLocIterator(proc, equivSetID);

              // if the memory reference does not involve a dereference or an
              // address of then add the location it maps directly to to the set
              // as long as it is visible within the procedure
              FixedLocationVisitor visitor(mIR, proc, equivSets);
              mre->acceptVisitor(visitor);
              OA_ptr<LocSetIterator> locSetIterPtr = 
                visitor.getDirectRefLocIterator();
              for (; locSetIterPtr->isValid(); ++(*locSetIterPtr) ) {
                OA_ptr<Location> directRefLoc = (locSetIterPtr->current());
                if (debug) {
                    std::cout << "IsDirectRef: directRefLoc = ";
                    directRefLoc->output(*mIR);
                }

                bool foundloc = false;
                for(locIterator->reset(); locIterator->isValid(); ++(*locIterator)) {
		  OA_ptr<Location> loc = locIterator->current();
                  if (*loc == *directRefLoc) {
                    foundloc = true;
                    break;
                  }
		}

                if (!foundloc) {
                  equivSets->addLocation( directRefLoc, proc, 
                                          equivSetID );
                  // keep track of how many locs we have
                  equivSetIdToNumLocs[equivSetID]++;
                  changed = true;
                }
              }
  
              // determine if this mre needs an invisible location
              InvisibleLocationVisitor invVisitor(mIR, proc, 
                                                  mProcToFormalSet[proc]);
              mre->acceptVisitor(invVisitor);
              if (invVisitor.isInvisibleRef()) {
                if (debug) {
                    std::cout << "IsInvisibleRef: invisibleLoc = ";
                    OA_ptr<Location> loc = invVisitor.getInvisibleRefLoc();
                    loc->output(*mIR);
                }

                bool foundloc = false;
                for(locIterator->reset(); locIterator->isValid(); 
                        ++(*locIterator)) {
   		          OA_ptr<Location> loc = locIterator->current();
                  if (*loc == *invVisitor.getInvisibleRefLoc()) {
                    foundloc = true;
                    break;
                  }
		}

                if (!foundloc) {
                  equivSets->addLocation( invVisitor.getInvisibleRefLoc(), 
                                          proc, equivSetID );
                  // keep track of how many locs we have
                  equivSetIdToNumLocs[equivSetID]++;
                  changed = true;
                }
              }
            } // loop over MREs
	  } // while(changed)
        } // if foundProc

      } // over equiv sets
    } // over procedures

    // loop through all the equivSets and if a set doesn't
    // have at least one fixed location then map everything to UnknownLoc
    // because all of EquivSets are supposed to have locations that don't 
    // overlap and UnknownLoc overlaps with everything
    std::map<int,int>::iterator equivSetIter;
    OA_ptr<UnknownLoc> unknown;  unknown = new UnknownLoc();
    for (equivSetIter = equivSetIdToNumLocs.begin();
         equivSetIter != equivSetIdToNumLocs.end();
         equivSetIter++ ) 
    {
        if (equivSetIter->second == 0 
            && equivSetIdHasMemRefHandle[equivSetIter->first]==true) 
        {
            if (debug) {
              std::cout << "Found empty equiv set, equivSetID = "
                        << equivSetIter->first << std::endl;
            }
            equivSets->mapAllToUnknown();
            break;
        }
    }

    return equivSets;
}

  } // end of namespace Alias
} // end of namespace OA

/*! \file
  
  \brief The AnnotationManager that generates AliasMaps in a
         context and flow insensitive way

  \authors Michelle Strout, Brian White
  \version $Id: ManagerFIAlias.cpp,v 1.1.2.20 2006/01/18 15:38:41 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerFIAliasAliasMap.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace Alias {

static bool debug = false;

/*!
*/
ManagerFIAliasAliasMap::ManagerFIAliasAliasMap(OA_ptr<AliasIRInterface> _ir) 
   : ManagerFIAlias(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerFIAliasAliasMap:ALL", debug);
}

OA_ptr<Alias::InterAliasMap> 
ManagerFIAliasAliasMap::performAnalysis( OA_ptr<IRProcIterator> procIter,
                                         FIAliasImplement implement )
{
    // create an empty InterAliasMap
    mInterAliasMap = new InterAliasMap();

    // Invoke the ManagerFIAlias routine to create the alias sets
    // within the union-find universe.
    OA_ptr<UnionFindUniverse> ufset = performFIAlias(procIter, implement);

    // Depending on the implementation of FIAlias, only a 
    // subset of the defined procedures may have been analyzed.
    OA_ptr<IRProcIterator> analyzedProcIter = getAnalyzedProcIter();

    // Convert the uion-find universe to equiv sets.
    buildAliasMaps(ufset,analyzedProcIter);

    return mInterAliasMap;
}


void
ManagerFIAliasAliasMap::buildAliasMaps( OA_ptr<UnionFindUniverse> ufset,
                                        OA_ptr<IRProcIterator> procIter )
{
    if (debug) { std::cout << "=========== Building AliasMap ========" 
                           << std::endl; }

    /////////////// build AliasMap
    std::map<ProcHandle, std::map<int,int> > setIdToAliasMapId;
    std::set<int> allufsetIDs;

    // loop all the mres and get a set of mres for each ufset
    std::map<OA_ptr<MemRefExpr>,int>::iterator mreMapIter;
    for (mreMapIter=mMREToID.begin(); mreMapIter!=mMREToID.end();
         mreMapIter++ )
    {
        OA_ptr<MemRefExpr> mre = mreMapIter->first;

        // only map those MREs that do not involve an addressOf operation
        if(mre->isaRefOp()) {
           OA_ptr<RefOp> refop = mre.convert<RefOp>();
           if(refop->isaAddressOf()) { continue; }
        }
 
        int setID = ufset->Find(mMREToID[mre]);
        // record which mres are associated with this ufset
        allufsetIDs.insert(setID);
        
    }

    // iterate over all procedures and determine the set of fixed
    // locations for each set within each procedure
    for (procIter->reset();  procIter->isValid(); (*procIter)++ ) {
      ProcHandle proc = procIter->current();
      if (debug) {
        std::cout << "proc = " << mIR->toString(proc) << std::endl;
      }
      std::map<int,bool> foundNonVisibleRef;
      std::map<int,bool> foundVisibleFixedLoc;
      std::map<int,int>  LocsetIdToAliasMapId;

      // create an alias map for this procedure
      OA_ptr<AliasMap> aliasMap;
      aliasMap = new AliasMap(proc);

      // visit all of the MREs in the program being analyzed
      for (mreMapIter=mMREToID.begin(); mreMapIter!=mMREToID.end();
           mreMapIter++ )
      {
          OA_ptr<MemRefExpr> mre = mreMapIter->first;
          if (debug) {
              std::cout << "\tmre = ";
              mre->dump(std::cout);
              mre->output(*mIR);
          }
          
          if(mre->isaAddressOf()) {
              if (debug) {
                  std::cout << "\taddressOf so skipping this MRE" << std::endl;
              }
              continue; 
          }
 
          int ufsetID = ufset->Find(mMREToID[mre]);
          if (debug) {
              std::cout << "\tufsetID = " << ufsetID << std::endl;
          }

          // if the base is visible then could be a FixedLoc,
          // InvisibleLoc, or just a MRE involving a deref to a
          // visible variable or UnnamedLoc
          VisibleBaseVisitor mreVisitor(mIR, proc);
          mre->acceptVisitor(mreVisitor);
          if (mreVisitor.isBaseVisible()) {

            if (debug) {
              std::cout << "\tmre has a visible base" << std::endl;
            }
            
            if (LocsetIdToAliasMapId.find(ufsetID)
                ==LocsetIdToAliasMapId.end() )
            {
              LocsetIdToAliasMapId[ufsetID] = aliasMap->makeEmptySet();
            }
            int aliasMapSetId = LocsetIdToAliasMapId[ufsetID];
            // Determine if the MRE is a fixed location
            FixedLocationVisitor visitor(mIR, proc, aliasMap);
            mre->acceptVisitor(visitor);
            OA_ptr<LocSetIterator> locSetIterPtr = 
                visitor.getDirectRefLocIterator();

            // if found a fixed location 
            if (locSetIterPtr->isValid()) {
                foundVisibleFixedLoc[ufsetID] = true;

                // put all the fixed locations into the locset
                for (; locSetIterPtr->isValid(); ++(*locSetIterPtr) ) {
                  OA_ptr<Location> directRefLoc = (locSetIterPtr->current());
                  if (debug) {
                      std::cout << "Location: directRefLoc = ";
                      directRefLoc->output(*mIR);
                  }
          
                  aliasMap->addLocation( directRefLoc, aliasMapSetId );
                  
                  if(directRefLoc->isaSubSet()) {
                     OA_ptr<LocSubSet> locSubSet;
                     locSubSet = directRefLoc.convert<LocSubSet>();          
                     OA_ptr<Location> baseLoc;
                     baseLoc = locSubSet->getBaseLoc();
                     aliasMap->removeBaseLoc(baseLoc, aliasMapSetId );
                  } 
                }

            // did not find a fixed location so see if we have an invisible
            } else {
                InvisibleLocationVisitor invVisitor(mIR, proc, 
                                                    mProcToFormalSet[proc]);
                mre->acceptVisitor(invVisitor);
                if (invVisitor.isInvisibleRef()) {
                    OA_ptr<Location> visitorloc 
                         = invVisitor.getInvisibleRefLoc();
                    if (debug) {
                        std::cout << "Location: invloc = ";
                        visitorloc->output(*mIR);
                    }
                    aliasMap->addLocation( visitorloc, aliasMapSetId );
                }
            } 

            // whether we found a location for this mre or not
            // map this mre to the aliasMapSet 
            // the base of the memrefexpr is visible in this procedure
            if (LocsetIdToAliasMapId.find(ufsetID)
                !=LocsetIdToAliasMapId.end() )
            {
                int aliasMapSetId = LocsetIdToAliasMapId[ufsetID];
                aliasMap->mapMemRefToMapSet(mre,
                                            LocsetIdToAliasMapId[ufsetID]);
            }

          // If the base is not visible then we need to keep any invisible
          // locs that are found in the same ufset
          } else {

              foundNonVisibleRef[ufsetID] = true;
          }

      } // loop over MREs in program being analyzed

      // do some cleanup wrt to InvisibleLocs
      // for each ufset, remove all of the InvisibleLocs
      // if there was no invisible references but
      // there was a visible fixed location
      for (std::set<int>::iterator setIter=allufsetIDs.begin();
           setIter!=allufsetIDs.end(); setIter++)
      {
          if (foundVisibleFixedLoc[*setIter] == true
              && foundNonVisibleRef[*setIter] == false )
          {
            int aliasMapSetId = LocsetIdToAliasMapId[*setIter];
            aliasMap->removeInvisibleLocs(aliasMapSetId);
          }
      }
      setIdToAliasMapId[proc] = LocsetIdToAliasMapId;
      // store off the aliasmap for this procedure
      mInterAliasMap->mapProcToAliasMap(proc,aliasMap);

    } // iterate over all procedures

    // iterate over all of the mem ref handles
    // with the goal being to map the mem ref handles to alias sets
    std::set<OA_ptr<MemRefExpr> > multiFixLocMRESet;
    std::map<MemRefHandle,ProcHandle>::iterator refprocIter;
    for (refprocIter=mMemRefHandleToProc.begin();
         refprocIter!=mMemRefHandleToProc.end(); refprocIter++ )
    {
        MemRefHandle memref = refprocIter->first;
        ProcHandle proc = refprocIter->second;

        if (debug) {
          std::cout << "\nMapping MemRefHandle: " << mIR->toString(memref)
                    << std::endl;
        }

        // get alias map for this procedure
        OA_ptr<AliasMap> aliasMap = mInterAliasMap->getAliasMapResults(proc);

        // loop over MREs for the given memref
        OA_ptr<MemRefExprIterator> mreIterPtr 
            = mIR->getMemRefExprIterator(memref);
      
        // for each mem-ref-expr associated with this memref
        for (; mreIterPtr->isValid(); (*mreIterPtr)++) {
            OA_ptr<OA::MemRefExpr> mre = mreIterPtr->current();

            if (debug) {
              std::cout << "\n\tFound MemRefExpr: ";
              mre->output(*mIR);
              std::cout << std::endl;
            }

            // only map those MREs that do not involve an addressOf operation
            // if (mre->hasAddressTaken()) { continue; }
            if(mre->isaRefOp()) {
               OA_ptr<RefOp> refop = mre.convert<RefOp>();
               if(refop->isaAddressOf()) { continue; }
            }

            // map the mem ref handle to the set that mre is in
            int ufsetID = ufset->Find(mMREToID[mre]);

            // if we have no location associated with this
            // ufsetID within this proc, then there won't be an
            // alias map associated with it yet
            // therefore we need to map this mre and memrefhandle
            // to the zeroth AliasMap set and store that information
            // for this ufsetID and proc
            if (setIdToAliasMapId[proc].find(ufsetID)
                ==setIdToAliasMapId[proc].end() )
            {
                setIdToAliasMapId[proc][ufsetID] = 0;
            }

            // now assign the memref to the set within the alias map.
            aliasMap->mapMemRefToMapSet(memref, 
                                        setIdToAliasMapId[proc][ufsetID] );

            // BK (7/26/07) making NamedRef's and local UnnamedRefs
            // be within a MUSTALIAS set

            if ( multiFixLocMRESet.find(mre) !=  multiFixLocMRESet.end()) {
              // cannot make a new aliasMap set for an mre with multiple
              // fixed locations, because more than one location added to
              // an AliasMap set will cause the set to be MayAlias anyway
              if (debug) {
                std::cout << "\n\t\tFound multiFixLocMRE, continuing ...\n";
              }
              continue;
            }

            bool isLocalUnnamed = false;
            if (mre->isaUnnamed()) {
              if (debug) {
                std::cout << "\t\tUnnamedRef";
              }
              OA_ptr<UnnamedRef> uRef = mre.convert<UnnamedRef>();
              if (uRef->isLocal() && (uRef->getProcHandle()==proc)) {
                isLocalUnnamed = true;
                if (debug) {
                  if (isLocalUnnamed) {
                    std::cout << " Local\n";
                  } else {
                    std::cout << " non-Local\n";
                  }
                }
              }
            }
            if (mre->isaNamed() || isLocalUnnamed) {
              // get current aliasMap setId for this mre
              int aliasSetMapId = aliasMap->getMapSetId(mre);
              
              if (debug) {
                if (!isLocalUnnamed) {
                  std::cout << "\t\tNamedRef\n";
                }
                std::cout << "\t\t\tMapSetId = " << aliasSetMapId
                          << "  isMust = " << aliasMap->isMust(aliasSetMapId)
                          << std::endl;
              }

              // if not MUSTALIAS, create new aliasMap set and populate it
              if (!aliasMap->isMust(aliasSetMapId)) {
                if (debug) {
                  std::cout << "Found NamedRef or local UnnamedRef as MayAlias: ";
                  mre->output(*mIR);
                  std::cout << std::endl;
                }
                
                // 1) add any fixed locs (only if there is exactly one fixed loc)
                FixedLocationVisitor visitor(mIR, proc, aliasMap);
                mre->acceptVisitor(visitor);
                OA_ptr<LocSetIterator> locSetIterPtr = 
                  visitor.getDirectRefLocIterator();
                
                // if found a fixed location 
                if (locSetIterPtr->isValid()) {
                  
                  // get first fixed loc and look for more
                  OA_ptr<Location> directRefLoc = locSetIterPtr->current();
                  // incrememt fixedLocIter
                  ++(*locSetIterPtr);
                  
                  // if there is more than one fixed location for this mre,
                  if (locSetIterPtr->isValid()) {
                    // add this mre to the multiFixLocMRESet and move on
                    multiFixLocMRESet.insert(mre);
                    if (debug) {
                      std::cout << "&*&*&*&* Warning: "
                                << "found more than one fixed loc for mre";
                    }
                    continue; // go to next mre for this memref
                  }
                  
                  // only one fixed loc, just process it:
                  // 0) get a new MustAlias set
                  aliasSetMapId = aliasMap->makeEmptySet();
                  // 1) add fixed location to new MustAlias set
                  aliasMap->addLocation( directRefLoc, aliasSetMapId);
                  if (debug) {
                    std::cout << "only MustAlias Location: directRefLoc = ";
                    directRefLoc->output(*mIR);
                    std::cout << "\tremapping to set id: " << aliasSetMapId
                              << std::endl;
                  }
                  // 2) map the mre and memref to new MustAlias set
                  aliasMap->mapMemRefToMapSet(mre, aliasSetMapId);
                  aliasMap->mapMemRefToMapSet(memref, aliasSetMapId);
                  
                } // end of if mre has a fixed location
                
              }// end of if (!isMust)
              else {

                // still need to map the memref to this aliasMap for mre
                aliasMap->mapMemRefToMapSet(memref, aliasSetMapId);
              }

            } // end of if NamedRef or local UnnamedRef   
        }  // end of mre loop
    } // over memrefhandles
}

  } // end of namespace Alias
} // end of namespace OA

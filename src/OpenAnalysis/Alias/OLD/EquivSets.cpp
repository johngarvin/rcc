/*! \file
  
  \brief Implementation of Alias::EquivSets

  \author Michelle Strout
  \version $Id: EquivSets.cpp,v 1.18.6.6 2006/01/05 20:09:34 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "EquivSets.hpp"
#include <Utils/Util.hpp>

namespace OA {
  namespace Alias {

static bool debug = false;


EquivSets::EquivSets() {
    OA_DEBUG_CTRL_MACRO("DEBUG_EquivSets:ALL", debug);
}


//! indicate the aliasing relationship between two memory references
AliasResultType EquivSets::alias(MemRefHandle ref1, 
                                 MemRefHandle ref2)
{
    // intersect the equivsets for the memrefs
    std::set<int> temp; 
    std::set_intersection(mMemRefToSets[ref1].begin(), 
                          mMemRefToSets[ref1].end(),
                          mMemRefToSets[ref2].begin(), 
                          mMemRefToSets[ref2].end(),
                          std::inserter(temp,temp.end()));

    // if the intersection is empty then they don't alias
    if (temp.empty()) {
        return NOALIAS;
    } else {
        return MAYALIAS;
    } 
}

//! iterator over locations that a memory reference may reference
OA_ptr<LocIterator> EquivSets::getMayLocs(MemRefHandle ref)
{
    OA_ptr<LocSet> locSet;
    locSet = new LocSet;
    OA_ptr<LocSetIterator> retval;

    // iterate over all the sets for this MemRefHandle and return
    // an iterator over all the locations in those sets
    std::set<int>::iterator iter;
    for (iter=mMemRefToSets[ref].begin();
         iter!=mMemRefToSets[ref].end(); iter++)
    {
      locSet = unionLocSets(*locSet, 
                            mProcNSetToLocs[mMemRefToProc[ref]][*iter]);
    }

    retval = new LocSetIterator(locSet);
    return retval;
}

//! iterator over locations that a memory reference must reference
//! these locations will all have full static overlap
//! this data structure doesn't maintain must info
OA_ptr<LocIterator> EquivSets::getMustLocs(MemRefHandle ref)
{
    OA_ptr<LocSetIterator> retval;
    OA_ptr<LocSet> emptySet; emptySet = new LocSet;
    retval = new LocSetIterator(emptySet);
    return retval;
}

//! iterator over locations that a memory refer expression may reference
OA_ptr<LocIterator> EquivSets::getMayLocs(MemRefExpr &ref, ProcHandle proc)
{
    OA_ptr<LocSet> locSet;
    locSet = new LocSet;
    OA_ptr<LocSetIterator> retval;

    OA_ptr<MemRefExpr> refPtr = ref.clone();
    if ( mMREToIdMap.find(refPtr) != mMREToIdMap.end() ) {
      int id = mMREToIdMap[refPtr];
      locSet = unionLocSets(*locSet, 
                            mProcNSetToLocs[proc][id]);
    }

    retval = new LocSetIterator(locSet);
    return retval;
}

//! iterator over locations that a memory refer expression may reference
OA_ptr<LocIterator> EquivSets::getMustLocs(MemRefExpr &ref, ProcHandle proc)
{
    OA_ptr<LocSetIterator> retval;
    OA_ptr<LocSet> emptySet; emptySet = new LocSet;
    retval = new LocSetIterator(emptySet);
    return retval;
}

//! get iterator over all memory references that alias information is
//! available for
OA_ptr<MemRefIterator> EquivSets::getMemRefIter()
{
    OA_ptr<EquivSetsMemRefIter> retval;
    OA_ptr<std::set<MemRefHandle> > memRefSet;
    memRefSet = new std::set<MemRefHandle>;

    std::map<MemRefHandle,std::set<int> >::iterator mapIter;
    for (mapIter=mMemRefToSets.begin(); mapIter!=mMemRefToSets.end(); 
         mapIter++) 
    {
        memRefSet->insert(mapIter->first);
    }
    retval = new EquivSetsMemRefIter(memRefSet);
    return retval;
}

//*****************************************************************
// Construction methods 
//*****************************************************************
   
//! associate the given location with the given procedure and equivSet
void 
EquivSets::addLocation(OA_ptr<Location> pLoc, ProcHandle proc, int equivSet)
{
    mProcNSetToLocs[proc][equivSet].insert(pLoc);
}

//! get iterator over all locations in a particular set
OA_ptr<LocIterator> EquivSets::getLocIterator(ProcHandle proc, int equivSet) 
{ 
    OA_ptr<LocSet> locSet;
    locSet = new LocSet;
    OA_ptr<LocSetIterator> retval;

    // It is important to call find rather than doing a 
    // mProcNSetToLocs[proc][equivSet] if the equivSet does not
    // exist.  The 'array index' operation will create the equivSet
    // if it does not exist; we do not want to create an empty equivSet.
    if (mProcNSetToLocs[proc].find(equivSet) == mProcNSetToLocs[proc].end()) {
      retval = new LocSetIterator(locSet);
      return retval;
    }

    locSet = unionLocSets(*locSet, 
			  mProcNSetToLocs[proc][equivSet]);

    retval = new LocSetIterator(locSet);
    return retval;
}

//! get the map from set it to location sets.
OA_ptr<std::map<int,OA_ptr<LocSet> > > EquivSets::getIdToLocSetMap(ProcHandle proc)
{
   OA_ptr<std::map<int, OA_ptr<LocSet> > > retVal;
   retVal = new std::map<int, OA_ptr<LocSet> >;

   std::map<int,std::set<OA_ptr<Location> > >::iterator it;
   for (it = mProcNSetToLocs[proc].begin(); 
        it != mProcNSetToLocs[proc].end(); ++(it) ) {
     int i = it->first;
     (*retVal)[i] = new LocSet;
     std::set<OA_ptr<Location> >::iterator setIt;
     for (setIt = it->second.begin(); setIt != it->second.end(); (setIt)++ ) {
       OA_ptr<Location> loc = (*setIt);
       (*retVal)[i]->insert(loc);
     }
   }

   return retVal;
}

//! associate a MemRefHandle with the given equivSet,
//! means that the MemRefHandle can access all of the locations
//! in the equivSet
void EquivSets::mapMemRefToEquivSet(MemRefHandle ref, int equivSetId)
{
    mMemRefToSets[ref].insert(equivSetId);
}

//! associate a MemRefExpr with the given equivSet,
void EquivSets::mapMemRefToEquivSet(OA_ptr<MemRefExpr> ref, int equivSetId)
{
    mMREToIdMap[ref] = equivSetId;
}


//! associate a MemRefHandle with a given procedure
void EquivSets::mapMemRefToProc(MemRefHandle ref, ProcHandle proc)
{
    mMemRefToProc[ref] = proc;
}

//! if any of the EquivSets end up mapping to the UnknownLoc
//! then all EquivSets must be merged
void EquivSets::mapAllToUnknown()
{
    // iterate over all mem ref handles we know about
    // and map them all to the zeroth equiv set
    OA_ptr<MemRefIterator> memrefIter = getMemRefIter();   
    for ( ; memrefIter->isValid(); (*memrefIter)++ ) {
        MemRefHandle memref = memrefIter->current();
        mMemRefToSets[memref].clear();
        mapMemRefToEquivSet(memref, 0);

        // clear out the mappings of procedures and equiv sets to locs
        mProcNSetToLocs.clear();

        // for the procedure that the memref occurs within
        // add the unknown location for the zeroth equiv set
        OA_ptr<UnknownLoc> unknown;  unknown = new UnknownLoc();
        addLocation(unknown, mMemRefToProc[memref], 0);
    }

}
  
void EquivSets::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    os << "======== mMemRefToSets ========" << std::endl;
    std::map<MemRefHandle,std::set<int> >::iterator refToSetsIter;
    std::set<int>::iterator setIter;
    for (refToSetsIter=mMemRefToSets.begin(); 
         refToSetsIter!=mMemRefToSets.end();
         refToSetsIter++ )
    {
        MemRefHandle ref = refToSetsIter->first;
        os << "\tMemRefHandle(" << ref.hval() << ") => ";
        std::set<int>& set = refToSetsIter->second;
        for (setIter = set.begin(); setIter!=set.end(); setIter++) {
            os << *setIter << " ";
        }
        os << std::endl;
    }

/*
    os << "======== mSetToLocs ========" << std::endl;
    std::map<int,std::set<OA_ptr<Location> > >::iterator setToLocsIter;
    LocSet::iterator locSetIter;
    for (setToLocsIter = mSetToLocs.begin(); setToLocsIter!=mSetToLocs.end();
         setToLocsIter++ )
    {
        int setID = setToLocsIter->first;
        os << "\tsetID = " << setID << " => {";
        std::set<OA_ptr<Location> >& set = setToLocsIter->second;
        for (locSetIter = set.begin(); locSetIter!=set.end(); locSetIter++) {
            OA_ptr<Location> loc = *locSetIter;
            loc->dump(os);
        }
        os << "\t}" << std::endl;
    }


    os << "======== Output of getMayLocs ========" << std::endl;
    OA_ptr<MemRefIterator> refIter = getMemRefIter();
    for ( ; refIter->isValid(); (*refIter)++ ) {
        MemRefHandle ref = refIter->current();
        os << "== May Locs for MemRefHandle = " << ir->toString(ref)
           << std::endl;
        OA_ptr<LocIterator> locIter = getMayLocs(ref);
        for ( ; locIter->isValid(); (*locIter)++ ) {
            OA_ptr<Location> loc = locIter->current();
            loc->dump(os,ir);
        }
    }
*/
}
  
  } // end of Alias namespace
} // end of OA namespace

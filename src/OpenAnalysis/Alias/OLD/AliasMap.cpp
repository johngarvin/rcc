/*! \file
  
  \brief Implementation of Alias::AliasMap

  \author Michelle Strout
  \version $Id: AliasMap.cpp,v 1.24.6.1 2005/09/14 16:53:39 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "AliasMap.hpp"
#include <Utils/Util.hpp>

namespace OA {
  namespace Alias {

static bool debug = false;

const int AliasMap::SET_ID_NONE = -1; 

AliasMap::AliasMap() {
    OA_DEBUG_CTRL_MACRO("DEBUG_AliasMap:ALL", debug);
}

AliasMap::AliasMap(ProcHandle p) : mProcHandle(p), mNumSets(1), mStartId(0)
{
  OA_DEBUG_CTRL_MACRO("DEBUG_AliasMap:ALL", debug);
  // default set for things MemRefHandles that we don't know enough
  // information about
  mIdToSetStatusMap[0] = MAYALIAS;
  OA_ptr<Location> loc;
  loc = dynamic_cast<Location*>(new UnknownLoc());
  mIdToLocSetMap[0] = new LocSet;
  mIdToLocSetMap[0]->insert( loc );

}

/*! 
   Copy constructor
*/
AliasMap::AliasMap(AliasMap& other)
{
    mProcHandle = other.mProcHandle;
    mNumSets = other.mNumSets;
    mStartId = other.mStartId;
    mIdToLocSetMap = other.mIdToLocSetMap;
    mIdToSetStatusMap = other.mIdToSetStatusMap;
    mIdToMemRefSetMap = other.mIdToMemRefSetMap;
    mMemRefToIdMap = other.mMemRefToIdMap;
    mMREToIdMap = other.mMREToIdMap;
    mLocToIdMap = other.mLocToIdMap;
}



/*!
   Indicate the aliasing relationship between two memory references

   It is possible for memory references that map to different
   alias map sets to must or may alias each other.  They will must
   alias each other if the associated location sets contain only one
   full location and that location must overlap.  They will may
   alias each other if any of the locations in the associated sets
   may overlap.
*/
AliasResultType AliasMap::alias(MemRefHandle ref1, MemRefHandle ref2)
{ 
    AliasResultType retval = MAYALIAS;

    // If either of the refs map to 0 (the unknown loc set), return
    // MAYALIAS.
    std::set<int>::iterator it1;
    std::set<int>::iterator it2;

    OA_ptr<std::set<int> > ids1 = getMapSetIds(ref1);
    for(it1 = ids1->begin(); it1 != ids1->end(); ++it1) {
      if (*it1 == 0) 
        return retval;
    }

    OA_ptr<std::set<int> > ids2 = getMapSetIds(ref2);
    for(it2 = ids2->begin(); it2 != ids2->end(); ++it2) {
      if (*it2 == 0) 
        return retval;
    }

    // Check to see if there is any overlap in the sets.
    std::set<int> temp; 
/*
    std::set_intersection(ids1->begin(), 
                          ids1->end(),
                          ids2->begin(), 
                          ids2->end(),
                          std::inserter(temp,temp.end()));
*/

    std::set_intersection(ids1->begin(),
                          ids1->end(),
                          ids2->begin(),
                          ids2->end(),
                          std::inserter(temp,temp.begin()));
 
 
    if (!temp.empty()) {

      // The sets do overlap.  If there is an overlapping MUST alias
      // set, return MUSTALIAS.  Else return MAYAliaS.
      for (it1 = temp.begin(); it1 != temp.end(); ++it1) {
        if (mIdToSetStatusMap[*it1]==MUSTALIAS)
          return MUSTALIAS;
      }

      return MAYALIAS;
    } 

    bool locationsOverlap = false;
    retval = MAYALIAS;

    for(it1 = ids1->begin(); it1 != ids1->end(); ++it1) {
       
      if (*it1 == 0)
      {
        return retval;
      } 
          
      for(it2 = ids2->begin(); it2 != ids2->end(); ++it2) {

       if (*it2 == 0) 
       { 
        return retval;
       }  

        if (mayOverlapLocSets(*mIdToLocSetMap[*it1],
                              *mIdToLocSetMap[*it2]) ) {
          locationsOverlap = true;
          if (isMust(*it1) && isMust(*it2)) {
            retval = MUSTALIAS;
            return retval;
          }
          // At this point, we know we have at least a MAYALIAS.
          // However, subsequent mayOverlapLocSets may determine
          // this is a MUSTALIAS, so keep looking.
        }

      }
    }

    if (locationsOverlap) {
      return retval;
    }

    // otherwise these two references do not alias
    retval = NOALIAS; 

    return retval; 
} 

//! get iterator over all locations in a particular set
OA_ptr<LocIterator> AliasMap::getLocIterator(int setId) 
{ OA_ptr<LocSetIterator> retval;
  if (mIdToLocSetMap[setId].ptrEqual(0)) {
    OA_ptr<LocSet> emptySet; emptySet = new LocSet;
    retval = new LocSetIterator(emptySet);  // empty loc set
  } else {
    retval = new LocSetIterator(mIdToLocSetMap[setId]); 
  }
  return retval;
}

//! iterator over locations that a memory reference may reference
OA_ptr<LocIterator> AliasMap::getMayLocs(MemRefHandle ref)
{

    /*
    std::cout << "reference MemRefHandle" << ref.hval() << std::endl;

    std::map<MemRefHandle, std::set<int> >::iterator reg_mMemRefToIdMap_iterator;


    std::cout << "before mMemRefToIdMap" << std::endl; 
    for(reg_mMemRefToIdMap_iterator = mMemRefToIdMap.begin();
        reg_mMemRefToIdMap_iterator != mMemRefToIdMap.end();
        reg_mMemRefToIdMap_iterator++)
    {
        const MemRefHandle &first = reg_mMemRefToIdMap_iterator->first;
        std::cout << "MemRefHandle" << first.hval() << std::endl;
    }
    std::cout << "after mMemRefToIdMap" << std::endl;
    */
    
    OA_ptr<LocSet> locSet;
    locSet = new LocSet;
    OA_ptr<LocSetIterator> retval;

    // iterate over all the sets for this MemRefHandle and return
    // an iterator over all the locations in those sets
    std::set<int>::iterator iter;
    for (iter=mMemRefToIdMap[ref].begin();
         iter!=mMemRefToIdMap[ref].end(); iter++)
    {
      if (!mIdToLocSetMap[*iter].ptrEqual(0))
      {

         locSet = unionLocSets(*locSet, 
                            *(mIdToLocSetMap[*iter]));
      }
    }

    retval = new LocSetIterator(locSet);
    return retval;
}

//! iterator over locations that a memory reference must reference
//! these locations will all have full static overlap
OA_ptr<LocIterator> AliasMap::getMustLocs(MemRefHandle ref)
{
    OA_ptr<LocSet> locSet;
    locSet = new LocSet;
    OA_ptr<LocSetIterator> retval;

    // iterate over all the sets for this MemRefHandle and return
    // an iterator over all the locations in those sets
    std::set<int>::iterator iter;
    for (iter=mMemRefToIdMap[ref].begin();
         iter!=mMemRefToIdMap[ref].end(); iter++)
    {
      if (mIdToSetStatusMap[*iter]==MUSTALIAS 
          && !mIdToLocSetMap[*iter].ptrEqual(0)) 
      {
        locSet = unionLocSets(*locSet, 
                              *(mIdToLocSetMap[*iter]));
      }
    }

    retval = new LocSetIterator(locSet);
    return retval;
}

//! iterator over locations that a memory refer expression may reference
OA_ptr<LocIterator> AliasMap::getMayLocs(MemRefExpr &ref, ProcHandle proc)
{
    OA_ptr<LocSet> locSet;
    locSet = new LocSet;
    OA_ptr<LocSetIterator> retval;

    // iterate over all the sets for this MemRefHandle and return
    // an iterator over all the locations in those sets
    OA_ptr<MemRefExpr> refPtr = ref.clone();
    if( mMREToIdMap.find(refPtr) != mMREToIdMap.end() ) {
      int id = mMREToIdMap[refPtr];
      if (!mIdToLocSetMap[id].ptrEqual(0))
      {
         locSet = unionLocSets(*locSet, *(mIdToLocSetMap[id]));
      }
    }

    // if nothing was found then return the may locs for 
    // the partially or fully accurate counterpart
    // It is necessary when
    // an aggregate is only accessed one way or another and then
    // the other MRE is used to look up mayloc info.
    OA_ptr<MemRefExpr> mreClone = refPtr->clone();

    /* PLM 1/23/07 deprecated hasFullAccuracy
    if (refPtr->hasFullAccuracy() ) {
        mreClone->setAccuracy(false);
    } else {
        mreClone->setAccuracy(true);
    }
    */

   if(!mreClone->isaSubSetRef()) {
      OA::OA_ptr<OA::SubSetRef> subset_mre;
      OA::OA_ptr<OA::MemRefExpr> nullMRE;
      OA::OA_ptr<OA::MemRefExpr> composed_mre;

      subset_mre = new OA::SubSetRef(
                                     OA::MemRefExpr::USE,
                                     nullMRE
                                    );

      mreClone = subset_mre->composeWith(mreClone->clone());
    } else {     
      mreClone = mreClone.convert<RefOp>()->getMemRefExpr();   
    }
   
    
    if( mMREToIdMap.find(mreClone) != mMREToIdMap.end() ) {
      int id = mMREToIdMap[mreClone];
      if (!mIdToLocSetMap[id].ptrEqual(0))
      {
         locSet = unionLocSets(*locSet, *(mIdToLocSetMap[id]));
      }
    }

    retval = new LocSetIterator(locSet);
    return retval;
}

//! iterator over locations that a memory refer expression may reference
OA_ptr<LocIterator> AliasMap::getMustLocs(MemRefExpr &ref, ProcHandle proc)
{
    OA_ptr<LocSet> locSet;
    locSet = new LocSet;
    OA_ptr<LocSetIterator> retval;

    OA_ptr<MemRefExpr> refPtr = ref.clone();
    if ( mMREToIdMap.find(refPtr) != mMREToIdMap.end() ) {
      int id = mMREToIdMap[refPtr];
      if (mIdToSetStatusMap[id]==MUSTALIAS 
          && !mIdToLocSetMap[id].ptrEqual(0)) 
      {
        locSet = unionLocSets(*locSet, 
                              *(mIdToLocSetMap[id]));
      }
    }

    retval = new LocSetIterator(locSet);
    return retval;
}

//! get iterator over all must aliases for a specific mem ref
OA_ptr<MemRefIterator> AliasMap::getMustAliases(MemRefHandle ref)
{ 
    // create an empty MemRefSet that will be given to iterator
    OA_ptr<MemRefSet> mustSet; mustSet = new MemRefSet;

    // determine what sets the references map to
    OA_ptr<std::set<int> > ids = getMapSetIds(ref);
    std::set<int>::iterator it;

    if (debug) {
      std::cout << "AliasMap::getMustAliases: id = ";
      for(it = ids->begin(); it != ids->end(); ++it) {
	std::cout << *it << " ";
      }
    }

    // find all the map sets that contain locations which may overlap with
    // this map set (it should find itself) and both alias map sets have
    // MUST status
    std::set<int> mustOverlapMapSetIds;
    std::map<int,OA_ptr<LocSet> >::iterator mapIter;
    for (it = ids->begin(); it != ids->end(); ++it) {

      int id = *it;

      for (mapIter = mIdToLocSetMap.begin(); mapIter != mIdToLocSetMap.end();
           mapIter++) 
      {
          int otherId = mapIter->first;
          if (mIdToSetStatusMap[otherId]==MUSTALIAS 
              && mIdToSetStatusMap[id]==MUSTALIAS
              && mayOverlapLocSets(*(mapIter->second),*mIdToLocSetMap[id])) 
          {
              mustOverlapMapSetIds.insert(otherId);
          }
      }
    }

    // find all the memory references that map to those loc sets
    // and put them in the set of must aliases
    std::set<int>::iterator setIter;
    for (setIter=mustOverlapMapSetIds.begin(); 
         setIter!=mustOverlapMapSetIds.end(); setIter++ )
    {
        int setId = *setIter;

        MemRefSet::iterator memrefSetIter;
        for (memrefSetIter=mIdToMemRefSetMap[setId].begin();
             memrefSetIter!=mIdToMemRefSetMap[setId].end();
             memrefSetIter++) 
        {
            mustSet->insert(*memrefSetIter);
        }
    }

    OA_ptr<MemRefIterator> retIter; retIter = new AliasMapMemRefIter(mustSet);
    return retIter;

}

//! get iterator over all may aliases for a specific mem ref
OA_ptr<MemRefIterator> AliasMap::getMayAliases(MemRefHandle ref)
{ 
    // create an empty MemRefSet that will be given to iterator
    OA_ptr<MemRefSet> maySet; maySet = new MemRefSet;

    // determine what sets the references map to
    OA_ptr<std::set<int> > ids = getMapSetIds(ref);
    std::set<int>::iterator it;

    if (debug) {
      std::cout << "AliasMap::getMayAliases: id = ";
      for(it = ids->begin(); it != ids->end(); ++it) {
	std::cout << *it << " ";
      }
    }

    // find all the map sets that contain locations which may overlap with
    // this map set (it should find itself)
    std::set<int> mayOverlapMapSetIds;
    std::map<int,OA_ptr<LocSet> >::iterator mapIter;
    for (it = ids->begin(); it != ids->end(); ++it) {

      int id = *it;

      for (mapIter = mIdToLocSetMap.begin(); mapIter != mIdToLocSetMap.end();
           mapIter++) 
      {
          int otherId = mapIter->first;
          if (mayOverlapLocSets(*(mapIter->second),*mIdToLocSetMap[id])) 
          {
              mayOverlapMapSetIds.insert(otherId);
          }
      }
    }

    // find all the memory references that map to those loc sets
    // and put them in the set of must aliases
    std::set<int>::iterator setIter;
    for (setIter=mayOverlapMapSetIds.begin(); 
         setIter!=mayOverlapMapSetIds.end(); setIter++ )
    {
        int setId = *setIter;

        MemRefSet::iterator memrefSetIter;
        for (memrefSetIter=mIdToMemRefSetMap[setId].begin();
             memrefSetIter!=mIdToMemRefSetMap[setId].end();
             memrefSetIter++) 
        {
            maySet->insert(*memrefSetIter);
        }
    }

    OA_ptr<MemRefIterator> retIter; retIter = new AliasMapMemRefIter(maySet);
    return retIter;
}
    
//! get iterator over all must aliases for a specific location
OA_ptr<MemRefIterator> AliasMap::getMustAliases(OA_ptr<Location> loc)
{
    // create an empty MemRefSet that will be given to iterator
    OA_ptr<MemRefSet> mustSet; mustSet = new MemRefSet;

    // find all the map sets that contain locations which may overlap with
    // this location and the alias map set is a MUST alias and this location
    // is fully accurate
    LocSet singleEntrySet;
    singleEntrySet.insert(loc);
    std::set<int> mustOverlapMapSetIds;
    std::map<int,OA_ptr<LocSet> >::iterator mapIter;
    for (mapIter = mIdToLocSetMap.begin(); mapIter != mIdToLocSetMap.end();
         mapIter++) 
    {
        int otherId = mapIter->first;

        /* PLM 1/23/07 deprecated accuracy field
        if (mIdToSetStatusMap[otherId]==MUSTALIAS 
            && loc->hasFullAccuracy()
            && mayOverlapLocSets(*(mapIter->second),
                                           singleEntrySet)) 
        {
            mustOverlapMapSetIds.insert(otherId);
        }
        */

        if (mIdToSetStatusMap[otherId]==MUSTALIAS
            && (!loc->isaSubSet())
            && mayOverlapLocSets(*(mapIter->second),
                                           singleEntrySet))
        {
            mustOverlapMapSetIds.insert(otherId);
        }

    }

    // find all the memory references that map to those loc sets
    // and put them in the set of must aliases
    std::set<int>::iterator setIter;
    for (setIter=mustOverlapMapSetIds.begin(); 
         setIter!=mustOverlapMapSetIds.end(); setIter++ )
    {
        int setId = *setIter;

        MemRefSet::iterator memrefSetIter;
        for (memrefSetIter=mIdToMemRefSetMap[setId].begin();
             memrefSetIter!=mIdToMemRefSetMap[setId].end();
             memrefSetIter++) 
        {
            mustSet->insert(*memrefSetIter);
        }
    }

    OA_ptr<MemRefIterator> retIter; retIter = new AliasMapMemRefIter(mustSet);
    return retIter;
}

//! get iterator over all may aliases for a specific location
OA_ptr<MemRefIterator> AliasMap::getMayAliases(OA_ptr<Location> loc)
{
    // create an empty MemRefSet that will be given to iterator
    OA_ptr<MemRefSet> maySet; maySet = new MemRefSet;

    // find all the map sets that contain locations which may overlap with
    // this location 
    LocSet singleEntrySet;
    singleEntrySet.insert(loc);
    std::set<int> mayOverlapMapSetIds;
    std::map<int,OA_ptr<LocSet> >::iterator mapIter;
    for (mapIter = mIdToLocSetMap.begin(); mapIter != mIdToLocSetMap.end();
         mapIter++) 
    {
        int otherId = mapIter->first;
        if (mayOverlapLocSets(*(mapIter->second),singleEntrySet)) 
        {
            mayOverlapMapSetIds.insert(otherId);
        }
    }

    // find all the memory references that map to those loc sets
    // and put them in the set of may aliases
    std::set<int>::iterator setIter;
    for (setIter=mayOverlapMapSetIds.begin(); 
         setIter!=mayOverlapMapSetIds.end(); setIter++ )
    {
        int setId = *setIter;

        MemRefSet::iterator memrefSetIter;
        for (memrefSetIter=mIdToMemRefSetMap[setId].begin();
             memrefSetIter!=mIdToMemRefSetMap[setId].end();
             memrefSetIter++) 
        {
            maySet->insert(*memrefSetIter);
        }
    }

    OA_ptr<MemRefIterator> retIter; retIter = new AliasMapMemRefIter(maySet);
    return retIter;
}

//! get iterator over all memory references that information is
//! available for
OA_ptr<MemRefIterator> AliasMap::getMemRefIter()
{
    // create an empty MemRefSet that will be given to iterator
    OA_ptr<MemRefSet> memrefSet; memrefSet = new MemRefSet;

    // put all memory references that we know about into the set
    std::map<MemRefHandle,std::set<int> >::iterator mapIter;
    for (mapIter=mMemRefToIdMap.begin(); mapIter!=mMemRefToIdMap.end();
         mapIter++) 
    {
        memrefSet->insert(mapIter->first);
    }

    OA_ptr<MemRefIterator> retIter; retIter = new AliasMapMemRefIter(memrefSet);
    return retIter;
}

//*****************************************************************
// DataFlowSet interface
//*****************************************************************
/*
bool AliasMap::operator ==(DataFlow::DataFlowSet &other) const
{
    if (mIdToLocSetMap==other.mIdToLocSetMap
        && mIdToMemRefSetMap==other.mIdToMemRefSetMap
        && mIdToMRESetMap == other.mIdToMRESetMap)
    {
        return true;
    } else {
        return false;
    }
}

bool AliasMap::operator !=(DataFlow::DataFlowSet &other) const
{
    return ! operator==(other);
}

OA_ptr<DataFlow::DataFlowSet> AliasMap::clone()
{
    OA_ptr<AliasMap> retval;
    retval = new AliasMap(*this);
    return retval;
}
*/
//*****************************************************************
// Meet routine
//*****************************************************************

/*! Read through AliasMap sets in each, alias all locations that are within
    one alias map set in the returned AliasMap set.

    Mostly copying over mappings so start with a copy of ourselves.
    Then, add any mappings from other that aren't already in retval.
*/
/*
OA_ptr<AliasMap> AliasMap::meet(AliasMap& other)
{
    OA_ptr<AliasMap> retval;

    // first make a copy of ourselves
    retval = new AliasMap(*this); 
    
    // loop through all sets in other
    OA_ptr<IdIterator> idIterPtr = getIdIterator();
    for ( ; idIterPtr->isValid(); ++(*idIterPtr) ) {
      int i = idIterPtr->current();
      // if there is more than one location
    
}
*/ 

//*****************************************************************
// Info methods unique to Alias::AliasMap
//*****************************************************************
    
//! get ids for the alias map set for this memory reference,
//! SET_ID_NONE indicates that this memory reference doesn't map to any of
//! the existing AliasMap sets
OA_ptr<std::set<int> > AliasMap::getMapSetIds(MemRefHandle ref) 
{
    std::map<MemRefHandle,std::set<int> >::iterator pos;
    pos = mMemRefToIdMap.find(ref);

    OA_ptr<std::set<int> > retval;
    retval = new std::set<int>;

    if (pos != mMemRefToIdMap.end()) {
        std::set<int>::iterator setIt;
        for(setIt = pos->second.begin(); setIt != pos->second.end(); ++setIt) {
	   retval->insert(*setIt);
        }
    } else {
        retval->insert(AliasMap::SET_ID_NONE);
    }

    return retval;
}

//! get unique id for the alias map set for this memory reference
//! expression or an equivalent memory reference expression
//! will create a new map set if this memory reference involves a 
//! full named location that doesn't have its address taken.
//! SET_ID_NONE indicates that this memory reference expression
//! doesn't map to any of the existing AliasMap sets
int AliasMap::getMapSetId(OA_ptr<MemRefExpr> mre) 
{
    int retval = AliasMap::SET_ID_NONE;

    std::map<OA_ptr<MemRefExpr>,int>::iterator pos;
    pos = mMREToIdMap.find(mre);
    if (pos != mMREToIdMap.end()) {
        return pos->second;
    } else {
        return AliasMap::SET_ID_NONE;
    }
}
    

/*! get id for an alias map set that contains
    the given location, SET_ID_NONE indicates that the given
    location isn't in any AliasMap sets 

    If more than one map set contains this location, will return
    the set with the fewest number of other locations
    because that is what is maintained in the mapping of locations
    to sets
*/
int AliasMap::getMapSetId(OA_ptr<OA::Location> pLoc)
{
    int retval = SET_ID_NONE;

    // loop through mapping of locations's to alias map ids to determine if
    // the given location is equivalent
    std::map<OA_ptr<Location>,int>::iterator mapIter;
    for (mapIter=mLocToIdMap.begin(); mapIter!=mLocToIdMap.end(); mapIter++) {
        OA_ptr<Location> knownloc = mapIter->first;
        // see if the already mapped location is equivalent to the given location
        if ( knownloc == pLoc ) {
            retval = mapIter->second;
            break;
        }
    }

    return retval;
}

//! get id for an alias map set that contains
//! an equivalent set of locations, SET_ID_NONE indicates 
//! that no equivalent location set was found in any AliasMap sets 
int AliasMap::getMapSetId(LocSet pLocSet)
{
    int retval = SET_ID_NONE;

    // loop through location sets and compare them to given location set
    std::map<int,OA_ptr<LocSet> >::iterator mapIter;
    for (mapIter=mIdToLocSetMap.begin(); mapIter!=mIdToLocSetMap.end();
         mapIter++)
    {
        OA_ptr<LocSet> mapLocSet = mapIter->second;
        if (subSetOf(pLocSet,*mapLocSet) 
            && subSetOf(*mapLocSet,pLocSet) )
        {
            return mapIter->first;
        } 
    }

    return retval;
}

OA_ptr<IdIterator> AliasMap::getIdIterator()
{
    OA_ptr<IdIterator> retval;
    retval = new IdIterator(mIdToLocSetMap);
    return retval;
}
 
/*
//! look up location associated with a symbol
//! returns NULL if one isn't found
//! user should delete location returned
OA_ptr<Location> AliasMap::getLocForSym(SymHandle sym)
{
    OA_ptr<Location> retval; retval = NULL;
    std::map<SymHandle,OA_ptr<Location> >::iterator pos;
    pos = mSymToLocMap.find(sym);
    if (pos!=mSymToLocMap.end()) {
        retval = pos->second;
    }
    return retval;
}
*/

//! create an empty alias map set and return the id
int AliasMap::makeEmptySet()
{
    int newsetId = mStartId + mNumSets;
    mIdToSetStatusMap[newsetId] = MUSTALIAS;
    mIdToLocSetMap[newsetId] = new LocSet;
    mNumSets++;
    return newsetId;
}
     

/*
//! insert the given mem ref expr instance into the alias map set
void AliasMap::insertInto(MemRefHandle ref, int setId)
{
    assert(setId < mNumSets);
    assert(setId >= 0);

    mMemRef2IdMap[ref] = setId;
}
*/

//! associate the given location with the given mapSet
void AliasMap::addLocation(OA_ptr<Location> pLoc, int setId)
{

    if (debug) {
        std::cout << "AliasMap::addLocation: setId = " << setId << ", pLoc = ";
        pLoc->dump(std::cout);
    }
    // check that an empty location set exists for this setId
    if (mIdToLocSetMap[setId].ptrEqual(0)) {
       mIdToLocSetMap[setId] = new LocSet;
    }

    // if new location doesn't have full accuracy then need
    // to give the set MAYALIAS status
    if (mIdToSetStatusMap[setId]==MUSTALIAS) {

        // subclasses of LocSubSet are fully accurate
        if(pLoc->isaSubSet()) {
           OA_ptr<LocSubSet> subLoc = pLoc.convert<LocSubSet>();
           if(!subLoc->isSubClassOfLocSubSet()) {
                mIdToSetStatusMap[setId] = MAYALIAS;
                if (debug) {
                  std::cout << "\tpLoc not fully accurate, making set " 
                            << setId << " MAYALIAS\n";
                }
           }
        } else {
            /* MMS 5/17/07, this is too expensive and only useful
             * when have NamedLocs with different SymHandles that
             * statically alias.
            // also if there are other locations in the set already 
            // and they don't all fully overlap with this location
            LocSetIterator locIter(mIdToLocSetMap[setId]);
            for (; locIter.isValid(); ++locIter) {
                OA_ptr<Location> loc = locIter.current();
                if (!loc->mustOverlap(*pLoc)) {
                    mIdToSetStatusMap[setId] = MAYALIAS;
                }
            }
            */

            // conservatively make a set MAYALIAS as soon as a second
            // location is being added, checking that this location
            // is not already in set
            // If size is greater than one, then will have already been 
            // set to MAYALIAS.
            if (!mIdToLocSetMap[setId]->empty() && 
                mIdToLocSetMap[setId]->find(pLoc)==mIdToLocSetMap[setId]->end()
               ) 
            {
                mIdToSetStatusMap[setId] = MAYALIAS;
                if (debug) {
                  std::cout << "\tmultiple locs within set " << setId 
                            << "  becoming MAYALIAS\n";
                } 
            }

            // make the set MAYALIAS if pLoc is an InvisibleLoc
            if(pLoc->isaInvisible()) {
              mIdToSetStatusMap[setId] = MAYALIAS;
              if (debug) {
                std::cout << "\npLoc is InvLoc, making set " << setId
                          << " MAYALIAS\n";
              }
            }
        }
    }

    // insert new location into set
    mIdToLocSetMap[setId]->insert(pLoc);

    // only map the location to this set if it is not already mapped
    // or if it is mapped to a set with less accuracy
    bool moreAccurate = false;
    if (mLocToIdMap.find(pLoc)!=mLocToIdMap.end() 
        && mIdToSetStatusMap[mLocToIdMap[pLoc]]==MUSTALIAS)
    {
        moreAccurate = true;
    }
    if (!moreAccurate) {
        mLocToIdMap[pLoc] = setId;
    }
}

OA_ptr<std::map<int,OA_ptr<LocSet> > > AliasMap::getIdToLocSetMap()
{
   OA_ptr<std::map<int, OA_ptr<LocSet> > > retVal;
   retVal = new std::map<int, OA_ptr<LocSet> >;

   OA_ptr<IdIterator> idIterPtr = getIdIterator();
   for ( ; idIterPtr->isValid(); ++(*idIterPtr) ) {
     int i = idIterPtr->current();
     (*retVal)[i] = new LocSet;
     OA_ptr<LocIterator> locIterPtr = getLocIterator(i);
     for (; locIterPtr->isValid(); (*locIterPtr)++ ) {
       OA_ptr<Location> loc = (locIterPtr->current());
       (*retVal)[i]->insert(loc);
     }
   }

   return retVal;
}

/*! 
   - find all existing alias map sets that involve the two locations
   - create new alias map set that includes all locations from those sets
   - map all mre's and memrefhandles from old sets to new set
   - remove old alias map sets 

   FIXME?: right now this implements transitivity of aliasing for the
   aliases in any sets with loc1 and loc2
*/
void AliasMap::aliasLocs(OA_ptr<Location> loc1, OA_ptr<Location> loc2)
{
    std::set<int> affectedSets;

    // loop through all the locations set to find all map sets
    // that contain the given locations
    OA_ptr<IdIterator> idIterPtr = getIdIterator();
    for ( ; idIterPtr->isValid(); ++(*idIterPtr) ) {
      int i = idIterPtr->current();
      if (mIdToLocSetMap[i]->find(loc1)!=mIdToLocSetMap[i]->end()) {
          affectedSets.insert(i);
      }
      if (mIdToLocSetMap[i]->find(loc2)!=mIdToLocSetMap[i]->end()) {
          affectedSets.insert(i);
      }
    }

    // if only one affected set then just put both locations in there,
    // means at least one is already in there
    if (affectedSets.size()==1) {
        std::set<int>::iterator iter;
        iter = affectedSets.begin();
        addLocation(loc1, *iter);
        addLocation(loc2, *iter);
    } else {

        // create a new alias map set
        int newSetId = makeEmptySet();

        // put all the locations from the affected sets into the new set
        // remap all the memory references in the affected sets
        std::set<int>::iterator iter;
        for (iter=affectedSets.begin(); iter!=affectedSets.end(); iter++) {
            OA_ptr<LocIterator> locIterPtr = getLocIterator(*iter);
            for (; locIterPtr->isValid(); (*locIterPtr)++ ) {
                OA_ptr<Location> loc = (locIterPtr->current());
                addLocation(loc, newSetId);
            }

            remapMemRefs(*iter, newSetId);
        }

        // make sure both locations are in new set
        addLocation(loc1, newSetId);
        addLocation(loc2, newSetId);
    }

 }

/*!
    Remaps all memrefhandles and memrefexprs to the new set
    and removes the oldSet.

    Will replace removed set with last set to maintain contiguity.
 */
void AliasMap::remapMemRefs(int oldSetId, int newSetId)
{
    ////// move all the MemRefHandles and MemRefExprs to the new map set
    // first the MemRefHandles
    MemRefSet::iterator mrIter;
    for (mrIter = mIdToMemRefSetMap[oldSetId].begin();
         mrIter != mIdToMemRefSetMap[oldSetId].end(); mrIter++ )
    {
        mMemRefToIdMap[*mrIter].erase(oldSetId);
        mMemRefToIdMap[*mrIter].insert(newSetId);
        mIdToMemRefSetMap[newSetId].insert(*mrIter);
    }
    // then the MemRefExprs
    MemRefExprSet::iterator mreIter;
    for (mreIter = mIdToMRESetMap[oldSetId].begin();
         mreIter != mIdToMRESetMap[oldSetId].end(); mreIter++ )
    {
        mMREToIdMap[*mreIter] = newSetId;
        mIdToMRESetMap[newSetId].insert(*mreIter);
    }

    ////// move last alias map set to the old spot
    int lastSetId = mStartId+mNumSets-1;

    if ( lastSetId != oldSetId ) {
       // move  the MemRefHandles
       mIdToMemRefSetMap[oldSetId] = mIdToMemRefSetMap[lastSetId];
       for (mrIter = mIdToMemRefSetMap[oldSetId].begin();
            mrIter != mIdToMemRefSetMap[oldSetId].end(); mrIter++ )
       {
           mMemRefToIdMap[*mrIter].erase(lastSetId);
           mMemRefToIdMap[*mrIter].insert(oldSetId);
       }

       // move  the MemRefExprs
       mIdToMRESetMap[oldSetId] = mIdToMRESetMap[lastSetId];
       for (mreIter = mIdToMRESetMap[oldSetId].begin();
            mreIter != mIdToMRESetMap[oldSetId].end(); mreIter++ )
       {
           mMREToIdMap[*mreIter] = oldSetId;
       }

       mIdToSetStatusMap[oldSetId] = mIdToSetStatusMap[lastSetId];
    }

    // any locations which point to the oldSetId should now
    // reference the newSetId
    LocSetIterator oldLocIter(mIdToLocSetMap[oldSetId]);
    for (; oldLocIter.isValid(); ++oldLocIter) {
        OA_ptr<Location> loc = oldLocIter.current();
        mLocToIdMap[loc] = newSetId;
    }

    if ( lastSetId != oldSetId ) {
       // any locations which point to the lastSetId should now
       // reference the oldSetId. 
       // NB:  this loop can not proceed the previous.
      if ( !mIdToLocSetMap[lastSetId].ptrEqual(0)) {
        LocSetIterator lastLocIter(mIdToLocSetMap[lastSetId]);
        for (; lastLocIter.isValid(); ++lastLocIter) {
          OA_ptr<Location> loc = lastLocIter.current();
          mLocToIdMap[loc] = oldSetId;
        }
      }
    }

    // remove the lastSetId from the various maps; it has been
    // moved to oldSetId.
    mIdToLocSetMap.erase(lastSetId);
    mIdToSetStatusMap.erase(lastSetId);
    mIdToMemRefSetMap.erase(lastSetId);
    mIdToMRESetMap.erase(lastSetId);

    // reduce the number of sets
    mNumSets--;
}

//! assign the UnknownLoc to the given alias map set and remove
//! all other locations from this set.
void AliasMap::mapToUnknown(int setId)
{
    // remove any locations which point to the setId.
    LocSetIterator oldLocIter(mIdToLocSetMap[setId]);
    for (; oldLocIter.isValid(); ++oldLocIter) {
        OA_ptr<Location> loc = oldLocIter.current();
        mLocToIdMap.erase(loc);
    }

    // Remove any locations to which setId pointed.
    mIdToLocSetMap.erase(setId);

    // And establish that it only points to one location, UnknownLoc.
    OA_ptr<Location> loc;
    loc = dynamic_cast<Location*>(new UnknownLoc());
    mIdToLocSetMap[setId] = new LocSet;
    mIdToLocSetMap[setId]->insert( loc );

    // Change the status of the map to MAYALIAS, as appropriate for unknown.
    mIdToSetStatusMap[setId] = MAYALIAS;
}

//! associate a MemRefHandle with the given mapset,
//! means that the MemRefHandle can access all of the locations
//! in the mapset, if only one full location then is a must access
void AliasMap::mapMemRefToMapSet(MemRefHandle ref, int setId)
{
    mMemRefToIdMap[ref].insert(setId);
    mIdToMemRefSetMap[setId].insert(ref);
}

//! associate a MemRefExpr with the given mapSet,
//! means that the MemRefExpr may access all of the locations
//! in the mapSet
void AliasMap::mapMemRefToMapSet(OA_ptr<MemRefExpr> ref, int setId)
{
    if (!ref.ptrEqual(0)) { 
        mMREToIdMap[ref] = setId; 
        mIdToMRESetMap[setId].insert(ref);
    }
}

//! remove all the InvisibleLocs from the specificed set
void AliasMap::removeInvisibleLocs(int setId)
{
    // check that a location set exists for this setId
    if (!mIdToLocSetMap[setId].ptrEqual(0)) {
       
        // also if there are other locations in the set already 
        // and they don't all fully overlap with this location
        std::set<OA_ptr<Location> > removeSet;
        LocSetIterator locIter(mIdToLocSetMap[setId]);
        for (; locIter.isValid(); ++locIter) {
            OA_ptr<Location> loc = locIter.current();
            if (loc->isaInvisible()) {
                removeSet.insert(loc);
            }
        }

        for (std::set<OA_ptr<Location> >::iterator setIter = removeSet.begin();
             setIter!=removeSet.end(); setIter++ )
        {
            mIdToLocSetMap[setId]->erase(*setIter);
        }
    }
}

/*********************************************************************/

//! remove the specific Inv
//! MREs are compared.
void AliasMap::removeInvisibleLocs(int setId, OA_ptr<MemRefExpr> inv_memref)
{
     std::set<OA_ptr<Location> > removeSet;
     LocSetIterator locIter(mIdToLocSetMap[setId]);

     if (debug) { 
         std::cout << "AliasMap::removeInvisibleLocs" << std::endl;
         std::cout << "\tmIdToLocSetMap[" << setId << "]->size() = "
                   << mIdToLocSetMap[setId]->size() << std::endl;
     }
     
     for (; locIter.isValid(); ++locIter) {
          OA_ptr<Location> loc = locIter.current(); 
          if (loc->isaInvisible()) {
               OA_ptr<InvisibleLoc>  temp_inv_loc 
                         = loc.convert<InvisibleLoc>();

               OA_ptr<MemRefExpr> loc_memref 
                         = temp_inv_loc->getMemRefExpr();

               if(inv_memref == loc_memref) {
                   removeSet.insert(loc);
               }
          }
     }
     if (debug) {
         std::cout << "\tremoveSet.size() = " << removeSet.size() << std::endl;
     }

     for (std::set<OA_ptr<Location> >::iterator setIter = removeSet.begin();
             setIter!=removeSet.end(); setIter++ )
     {
        OA_ptr<Location> loc = *setIter;
        if (debug) {
            std::cout << "\terasing invisible loc = " << std::endl;
            loc->dump(std::cout);
        }
        mIdToLocSetMap[setId]->erase(*setIter);
     }
}


void AliasMap::removeBaseLoc(OA_ptr<Location> baseLoc, int setId)
{
     std::set<OA_ptr<Location> > removeSet;
     LocSetIterator locIter(mIdToLocSetMap[setId]);

     for (; locIter.isValid(); ++locIter) {
          OA_ptr<Location> loc = locIter.current();
          if( loc == baseLoc ) {
              removeSet.insert(loc);
          }     
     }
 
     for (std::set<OA_ptr<Location> >::iterator setIter = removeSet.begin();
              setIter!=removeSet.end(); setIter++ )
     {
            mIdToLocSetMap[setId]->erase(*setIter);
     }
}


//! for location with fullAccuracy, check if there exists another
//! location with partial Accuracy in the mIdToLocSetMap.
bool AliasMap::isPartial(int setId, OA_ptr<MemRefExpr> inv_memref)
{
     LocSetIterator locIter(mIdToLocSetMap[setId]);
     for (; locIter.isValid(); ++locIter) {
            OA_ptr<Location> loc = locIter.current();
            if (loc->isaInvisible()) {
                OA_ptr<InvisibleLoc>  temp_inv_loc =
                             loc.convert<InvisibleLoc>();
                OA_ptr<MemRefExpr> loc_memref = 
                             temp_inv_loc->getMemRefExpr();
                if(inv_memref == loc_memref) {
                   return true;
                }
             }
     }
     return false;
}

/***********************************************************************/


//! incomplete output of info for debugging 
//! may also include non-persistent handle values
void AliasMap::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    // print locations for each set, ID : { locs }
    os << "============= AliasMap ============" << std::endl;
    OA_ptr<IdIterator> idIterPtr = getIdIterator();
    for ( ; idIterPtr->isValid(); ++(*idIterPtr) ) {
      int i = idIterPtr->current();
      os << "AliasMapSet[" << i << "] = { ";
      OA_ptr<LocIterator> locIterPtr = getLocIterator(i);
      for (; locIterPtr->isValid(); (*locIterPtr)++ ) {
        OA_ptr<Location> loc = (locIterPtr->current());
        os << ", ";
        os << "<";
        loc->dump(os,ir);
        os << "> ";
      }
      os << " }" << std::endl;
    }

    // print all memrefs and their mapping
    os << "MemRef mapping to AliasMap sets:" << std::endl;
    OA_ptr<MemRefIterator> memIterPtr = getMemRefIter();
    for (; memIterPtr->isValid(); (*memIterPtr)++) {
      MemRefHandle memref = memIterPtr->current();
      os << "(" << memref.hval() << ") " << ir->toString(memref);
      os << " --> [";

      OA_ptr<std::set<int> > ids = getMapSetIds(memref);
      std::set<int>::iterator it;
      for(it = ids->begin(); it != ids->end(); ++it) {
        os << *it << " ";
      }
      os << "]" << std::endl;

    }
 
}


  } // end of Alias namespace
} // end of OA namespace

/*! \file
  
  \brief Implementation of XAIF::AliasMapXAIF

  \author Michelle Strout
  \version $Id: AliasMapXAIF.cpp,v 1.3 2005/02/16 21:48:20 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

//! =============================================
//! Header files
//! ============================================
#include <OpenAnalysis/XAIF/AliasMapXAIF.hpp>

#include <OpenAnalysis/Utils/OutputBuilder.hpp>
namespace OA {
  namespace XAIF {
<<<<<<< .working
=======

//! ==================================================
//! turn on debug=true while debugging
//! ==================================================
static bool debug = false;
>>>>>>> .merge-right.r888

<<<<<<< .working
    static bool debug = false;
=======
//! =================================================
//! Static Members
//! =================================================
int AliasMapXAIF::sCurrentSetId = 1;
>>>>>>> .merge-right.r888

<<<<<<< .working
    AliasMapXAIF::AliasMapXAIF(ProcHandle p) : mProcHandle(p) {}
=======



//! ***************************************************
//!
//! Location Range Methods
//!
//! ***************************************************

LocRange:: LocRange(int start, int end) 
             : mStart(start), mEnd(end) {}


LocRange::~LocRange() {}

 
int LocRange::getStart() const { return mStart; }

int LocRange::getEnd() const { return mEnd; }

void LocRange::output(IRHandlesIRInterface& ir) const {

     sOutBuild->objStart("LocRange");
     sOutBuild->field("mStart", OA::int2string(mStart) );
     sOutBuild->field("mEnd", OA::int2string(mEnd) );
     sOutBuild->objEnd("LocRange");

}
>>>>>>> .merge-right.r888

<<<<<<< .working
    //! get iterator over all memory references that information is available for
    OA_ptr<MemRefHandleIterator> AliasMapXAIF::getMemRefIter() {
      // create an empty MemRefSet that will be given to iterator
      OA_ptr<MemRefSet> memrefSet; memrefSet = new MemRefSet;
=======



//! ****************************************************
//!
//! LocTuple Methods
//!
//! ****************************************************

LocTuple::LocTuple() : mRange(0,0), mFullOverlap(false) {}

LocTuple::LocTuple(int start, int end, bool fullOverlap)
        : mRange(start,end), mFullOverlap(fullOverlap) {}

LocTuple::~LocTuple() {}

LocRange LocTuple::getLocRange() const { 
         return mRange; 
}

bool LocTuple::isFull() const { 
         return mFullOverlap; 
}

bool LocTuple::operator==(const LocTuple& other) const {
   
    if (mRange.getStart() == other.getLocRange().getStart()
          && mRange.getEnd() == other.getLocRange().getEnd()
          && mFullOverlap == other.mFullOverlap)
      { return true;} else { return false; }

}

bool LocTuple::operator!=(const LocTuple& other) const { 
    return !(*this == other); 
}

bool LocTuple::operator<(const LocTuple& other) const
{ if (*this == other)  { return false; }
      else if (mRange.getStart() < other.mRange.getStart()) { return true; }
      else if (mRange.getStart() > other.mRange.getStart()) { return false; }
      else if (mRange.getEnd() < other.mRange.getEnd()) { return true; }
      else if (mRange.getEnd() > other.mRange.getEnd()) { return false; }
      else if (mFullOverlap == true) { return true; }
      else { return false; }
}

void LocTuple::output(IRHandlesIRInterface& ir) const  {
        sOutBuild->objStart("LocTuple");
        sOutBuild->fieldStart("mRange");
        mRange.output(ir);
        sOutBuild->fieldEnd("mRange");
        sOutBuild->field("mFullOverlap",OA::bool2string(mFullOverlap));
        sOutBuild->objEnd("LocTuple");
}




//! **************************************************
//!
//! IdIterator Methods
//!
//! **************************************************

IdIterator::IdIterator(std::map<int,OA_ptr<std::set<LocTuple> > >& aMap)
{
   //! iterate through ids and put them in a set
   std::map<int,OA_ptr<std::set<LocTuple> > >::iterator mapIter;
   for (mapIter=aMap.begin(); mapIter!=aMap.end(); mapIter++) {
        mSet.insert(mapIter->first);
   }
   reset();
}
>>>>>>> .merge-right.r888

<<<<<<< .working
      // put all memory references that we know about into the set
      std::map<MemRefHandle,int>::iterator mapIter;
      for (mapIter=mMemRefToIdMap.begin(); mapIter!=mMemRefToIdMap.end(); mapIter++) {
        memrefSet->insert(mapIter->first);
      }
=======
IdIterator::~IdIterator() {}
>>>>>>> .merge-right.r888

<<<<<<< .working
      OA_ptr<MemRefHandleIterator> retIter; 
      retIter = new Alias::AliasMapMemRefIter(memrefSet);
      return retIter;
    } // end AliasMapXAIF::getMemRefIter()
=======
void IdIterator::operator++() { if (isValid()) mIter++; }

bool IdIterator::isValid() const  { return mIter!=mSet.end(); }

int IdIterator::current() const  { return *mIter; }

void IdIterator::reset() { mIter = mSet.begin(); }






//! ***************************************************************
//!
//!              LocTuple Iterator Methods
//!
//! ***************************************************************

LocTupleIterator::LocTupleIterator(std::set<LocTuple>& pSet) {

   //! copy the set so we don't depend on aSet 
   //! hanging around in memory
   mSet = pSet;
   reset();
}
>>>>>>> .merge-right.r888

<<<<<<< .working
    int AliasMapXAIF::getMapSetId(MemRefHandle ref) {
      std::map<MemRefHandle,int>::iterator pos;
      pos = mMemRefToIdMap.find(ref);
      if (pos != mMemRefToIdMap.end()) {
	return pos->second;
      }
      else {
	return 0;
      }
    } // end AliasMapXAIF::getMapSetId(MemRefHandle ref)
=======

LocTupleIterator::~LocTupleIterator() {}

void LocTupleIterator::operator++() { if (isValid()) mIter++; }

bool LocTupleIterator::isValid() const  { return mIter!=mSet.end(); }

LocTuple LocTupleIterator::current() const  { return *mIter; }

void LocTupleIterator::reset() { mIter = mSet.begin(); }






//! *************************************************************
//!
//!                    AliasMapXAIF methods
//!
//! *************************************************************


AliasMapXAIF::AliasMapXAIF(ProcHandle p) : mProcHandle(p)
{
}

AliasMapXAIF::~AliasMapXAIF() 
{
}

//! ==============================================================
//! get unique id for the alias map set for this memory reference,
//! SET_ID_NONE indicates that this memory reference doesn't map 
//! to any of the existing AliasMap sets
//! ==============================================================
int AliasMapXAIF::getMapSetId(MemRefHandle ref) 
{
    std::map<MemRefHandle,int>::iterator pos;
    pos = mMemRefToIdMap.find(ref);
    if (pos != mMemRefToIdMap.end()) {
        return pos->second;
    } else {
        return AliasMapXAIF::SET_ID_NONE;
    }
}
>>>>>>> .merge-right.r888

    int AliasMapXAIF::findMapSet(MemRefHandle ref) {
      std::map<MemRefHandle,int>::iterator pos;
      pos = mMemRefToIdMap.find(ref);
      if (pos != mMemRefToIdMap.end()) {
	return pos->second;
      }
      else {
	return AliasMapXAIF::SET_ID_NONE;
      }
    } // end AliasMapXAIF::findMapSet(MemRefHandle ref)

<<<<<<< .working
    int AliasMapXAIF::findMapSet(OA_ptr<std::set<LocTuple> > pLocTupleSet) {
      int retval = SET_ID_NONE;
      // loop through location sets and compare them to given location set
      std::map<int,OA_ptr<std::set<LocTuple> > >::iterator mapIter;
      for (mapIter=mIdToLocTupleSetMap.begin(); mapIter!=mIdToLocTupleSetMap.end(); mapIter++) {
	OA_ptr<std::set<LocTuple> > mapLocTupleSet = mapIter->second;
	if (pLocTupleSet == mapLocTupleSet) {
	  return mapIter->first;
	}
      }
      return retval;
    } // end AliasMapXAIF::findMapSet(OA_ptr<std::set<LocTuple> > pLocTupleSet)
=======


//! ==================================================================
//! get id for an alias map set that contains
//! an equivalent set of locations, SET_ID_NONE indicates 
//! that no equivalent location set was found in any AliasMap sets 
//! ===================================================================
int AliasMapXAIF::getMapSetId(OA_ptr<std::set<LocTuple> > pLocTupleSet)
{
    int retval = SET_ID_NONE;
>>>>>>> .merge-right.r888

<<<<<<< .working
    OA_ptr<IdIterator> AliasMapXAIF::getIdIterator() {
      OA_ptr<IdIterator> retval;
      retval = new IdIterator(mIdToLocTupleSetMap);
      return retval;
    } // end AliasMapXAIF::getIdIterator()
=======
    //! ====================================================
    //! loop through location sets and compare them to given
    //! location set
    //! ====================================================
    std::map<int,OA_ptr<std::set<LocTuple> > >::iterator mapIter;
    for (mapIter=mIdToLocTupleSetMap.begin(); 
         mapIter!=mIdToLocTupleSetMap.end(); mapIter++)
    {
        OA_ptr<std::set<LocTuple> > mapLocTupleSet = mapIter->second;
        if (pLocTupleSet == mapLocTupleSet)
        {
            return mapIter->first;
        } 
    }
>>>>>>> .merge-right.r888

    //! get iterator over all locations in a particular set
    OA_ptr<LocTupleIterator> AliasMapXAIF::getLocIterator(int setId) {
      OA_ptr<LocTupleIterator> retval;
      retval = new LocTupleIterator(*mIdToLocTupleSetMap[setId]); 
      return retval;
    } // end AliasMapXAIF::getLocIterator()

<<<<<<< .working
    //! associate the given location set with the given mapSet
    void AliasMapXAIF::mapLocTupleSet(OA_ptr<std::set<LocTuple> > ltSet, int setId) {
      mIdToLocTupleSetMap[setId] = ltSet;
    } // end AliasMapXAIF::mapLocTupleSet()
=======
//! ===========================================================
//! get iterator over all locations in a particular set
//! ============================================================
OA_ptr<LocTupleIterator> AliasMapXAIF::getLocIterator(int setId) 
{ OA_ptr<LocTupleIterator> retval;
  retval = new LocTupleIterator(*mIdToLocTupleSetMap[setId]); 
  return retval;
}
>>>>>>> .merge-right.r888

    //! associate a MemRefHandle with the given mapset, means that the MemRefHandle can access
    //! all of the locations in the mapset, if only one full location then is a must access
    void AliasMapXAIF::mapMemRefToMapSet(MemRefHandle ref, int setId) {
      // if this memref is already pointed to one mapset then need
      // to take it out of mIdToMemRefSetMap for the current set id
      int currSetId = findMapSet(ref);
      if (currSetId != AliasMapXAIF::SET_ID_NONE) {
	mIdToMemRefSetMap[currSetId].erase(ref);
      } 
      mMemRefToIdMap[ref] = setId;
      mIdToMemRefSetMap[setId].insert(ref);
    } // end AliasMapXAIF::mapMemRefToMapSet()

<<<<<<< .working
    //! output for regression tests
    void AliasMapXAIF::output(IRHandlesIRInterface& ir) {
      sOutBuild->objStart("AliasMapXAIF");
      sOutBuild->outputIRHandle(mProcHandle, ir);
=======
//! ===========================================================
//! create an empty alias map set and return the id
//! ===========================================================
int AliasMapXAIF::makeEmptySet()
{
    int newsetId = sCurrentSetId;
    sCurrentSetId++;
    return newsetId;
}
     
//! =============================================================
//! associate the given location set with the given mapSet
//! =============================================================
void AliasMapXAIF::mapLocTupleSet(OA_ptr<std::set<LocTuple> > ltSet, int setId)
{
    mIdToLocTupleSetMap[setId] = ltSet;
}
>>>>>>> .merge-right.r888

<<<<<<< .working
      // print locations for each set, ID : { LocTuple }
      sOutBuild->mapStart("mIdToLocTupleSetMap", "int", "OA_ptr<std::set<LocTuple> >");
=======
//! =============================================================
//! associate a MemRefHandle with the given mapset,
//! means that the MemRefHandle can access all of the locations
//! in the mapset, if only one full location then is a must access
//! ==============================================================
void AliasMapXAIF::mapMemRefToMapSet(MemRefHandle ref, int setId)
{
    // if this memref is already pointed to one mapset then need
    // to take it out of mIdToMemRefSetMap for the current set id
    int currSetId = getMapSetId(ref);
    if (currSetId != AliasMapXAIF::SET_ID_NONE) {
        mIdToMemRefSetMap[currSetId].erase(ref);
    } 
    mMemRefToIdMap[ref] = setId;
    mIdToMemRefSetMap[setId].insert(ref);
}
>>>>>>> .merge-right.r888

<<<<<<< .working
      for (OA_ptr<IdIterator> idIterPtr = getIdIterator(); idIterPtr->isValid(); ++(*idIterPtr) ) {
	//os << "AliasMapSet[" << i << "] = { ";
	int i = idIterPtr->current();
	sOutBuild->mapEntryStart();
	sOutBuild->mapKey(OA::int2string(i));
	sOutBuild->mapValueStart();
	sOutBuild->listStart();
	for (OA_ptr<LocTupleIterator> locIterPtr = getLocIterator(i); locIterPtr->isValid(); ++(*locIterPtr) ) {
	  LocTuple loc = locIterPtr->current();
	  sOutBuild->listItemStart();
	  loc.output(ir);
	  sOutBuild->listItemEnd();
	}
	sOutBuild->listEnd();
	sOutBuild->mapValueEnd();
	sOutBuild->mapEntryEnd();
      }
      sOutBuild->mapEnd("mIdToLocTupleSetMap");
=======

//! ========================================================
//! output for regression tests
//! ========================================================
void AliasMapXAIF::output(IRHandlesIRInterface& ir) const
{

    sOutBuild->objStart("AliasMapXAIF");
>>>>>>> .merge-right.r888

      // print all memrefs and their mapping
      sOutBuild->mapStart("mMemRefToIdMap", "MemRefHandle", "int");
      for (OA_ptr<MemRefHandleIterator> memIterPtr = getMemRefIter(); memIterPtr->isValid(); (*memIterPtr)++) {
	MemRefHandle memref = memIterPtr->current();
	sOutBuild->mapEntryStart();
	sOutBuild->mapKeyStart();
	sOutBuild->outputIRHandle(memref, ir);
	sOutBuild->mapKeyEnd();
	sOutBuild->mapValue(OA::int2string(findMapSet(memref)));
	sOutBuild->mapEntryEnd();
      }
      sOutBuild->mapEnd("mMemRefToIdMap");
      sOutBuild->objEnd("AliasMap");
    } // end AliasMapXAIF::output()

<<<<<<< .working
    //! incomplete output of info for debugging 
    void AliasMapXAIF::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir) {
      // print locations for each set, ID : { LocTuple }
      os << "============= AliasMapXAIF ============" << std::endl;
      OA_ptr<IdIterator> idIterPtr = getIdIterator();
      for ( ; idIterPtr->isValid(); ++(*idIterPtr) ) {
	int i = idIterPtr->current();
	os << "AliasMapSet[" << i << "] = { ";
	for (OA_ptr<LocTupleIterator> locIterPtr = getLocIterator(i); locIterPtr->isValid(); ++(*locIterPtr) ) {
	  LocTuple loc = locIterPtr->current();
	  os << ", ";
	  os << "< " << loc.getLocRange().getStart() << "..";
	  os << loc.getLocRange().getEnd() << ", " << loc.isFull();
	  os << " > ";
	}
	os << " }" << std::endl;
      }
=======
    // print locations for each set, ID : { LocTuple }
    sOutBuild->mapStart("mIdToLocTupleSetMap", "int", 
                         "OA_ptr<std::set<LocTuple> >");

    std::map<int,OA_ptr<std::set<LocTuple> > >::const_iterator mapIter;
    mapIter = mIdToLocTupleSetMap.begin();

    for ( ; mapIter != mIdToLocTupleSetMap.end(); ++mapIter ) {
      //os << "AliasMapSet[" << i << "] = { ";
      int i = mapIter->first;

      sOutBuild->mapEntryStart();
         sOutBuild->mapKey(OA::int2string(i));
         sOutBuild->mapValueStart();
           sOutBuild->listStart();
           std::set<LocTuple>::const_iterator locIterPtr;
           locIterPtr = mapIter->second->begin();

           for (; locIterPtr != mapIter->second->end(); ++locIterPtr) {
                 LocTuple loc = *(locIterPtr);
                 sOutBuild->listItemStart();
                 loc.output(ir);
                 sOutBuild->listItemEnd();
           }

           sOutBuild->listEnd();
         sOutBuild->mapValueEnd();
      sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mIdToLocTupleSetMap");
>>>>>>> .merge-right.r888

<<<<<<< .working
      // print all memrefs and their mapping
      os << "MemRef mapping to AliasMap sets:" << std::endl;
      for (OA_ptr<MemRefHandleIterator> memIterPtr = getMemRefIter(); memIterPtr->isValid(); (*memIterPtr)++) {
	MemRefHandle memref = memIterPtr->current();
	os << "(" << memref << ") " << ir->toString(memref);
	os << " --> " << findMapSet(memref) << std::endl;
=======

    //! =========================================
    //! print all memrefs and their mapping
    //! =========================================
    sOutBuild->mapStart("mMemRefToIdMap", "MemRefHandle", "int");


    //! ========================================================
    //! create an empty MemRefSet that will be given to iterator
    //! =========================================================
    OA_ptr<MemRefSet> memrefSet; memrefSet = new MemRefSet;

    //! ============================================================
    //! put all memory references that we know about into the set
    //! ============================================================
    std::map<MemRefHandle,int>::const_iterator mIter;
    for (mIter=mMemRefToIdMap.begin(); mIter!=mMemRefToIdMap.end();
         mIter++) {
              MemRefHandle memref = mIter->first;
              sOutBuild->mapEntryStart();
              sOutBuild->mapKeyStart();
              sOutBuild->outputIRHandle(memref, ir);
              sOutBuild->mapKeyEnd();
              sOutBuild->mapValue(OA::int2string(mIter->second));
              sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mMemRefToIdMap");

    sOutBuild->objEnd("AliasMap");
}



//! ======================================================================
//! incomplete output of info for debugging 
//! ======================================================================
void AliasMapXAIF::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    /*
    // print locations for each set, ID : { LocTuple }
    os << "============= AliasMapXAIF ============" << std::endl;
    OA_ptr<IdIterator> idIterPtr
         = new IdIterator(mIdToLocTupleSetMap);
    for ( ; idIterPtr->isValid(); ++(*idIterPtr) ) {
      int i = idIterPtr->current();
      os << "AliasMapSet[" << i << "] = { ";
      OA_ptr<LocTupleIterator> locIterPtr = getLocIterator(i);
      for (; locIterPtr->isValid(); ++(*locIterPtr) ) {
        LocTuple loc = locIterPtr->current();
        os << ", ";
        os << "< " << loc.getLocRange().getStart() << "..";
        os << loc.getLocRange().getEnd() << ", " << loc.isFull();
        os << " > ";
>>>>>>> .merge-right.r888
      }
    } // end AliasMapXAIF::dump()

<<<<<<< .working
=======
    
    // print all memrefs and their mapping
    os << "MemRef mapping to AliasMap sets:" << std::endl;
    OA_ptr<MemRefHandleIterator> memIterPtr = getMemRefIter();
    for (; memIterPtr->isValid(); (*memIterPtr)++) {
      MemRefHandle memref = memIterPtr->current();
      os << "(" << memref << ") " << ir->toString(memref);
      os << " --> " << getMapSetId(memref) << std::endl;
    }
    */
}


>>>>>>> .merge-right.r888
  } // end of Alias namespace
} // end of OA namespace


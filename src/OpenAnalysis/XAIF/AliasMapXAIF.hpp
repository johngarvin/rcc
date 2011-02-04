/*! \file
  
  \brief Maps MemRefHandle's to a particular alias map set.

  \authors Michelle Strout
  \version $Id: AliasMapXAIF.hpp,v 1.4 2005/03/18 18:14:17 ntallent Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  Each map set has an id and a set of virtual storage locations 
  that memory reference in the set might reference.
  The virtual locations that XAIF expects are tuples where the first
  entry is a numeric range and the second entry is a bit indicating
  whether the range is fully or partially accessed.
*/

#ifndef AliasMapXAIF_H
#define AliasMapXAIF_H

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>

namespace OA {
  namespace XAIF {

typedef std::set<MemRefHandle> MemRefSet;

//! ============================================================
//! Numeric range to indicate a number of locations in an array
//! ============================================================

class LocRange : public virtual Annotation {
  public:
    
    //! =====================================================
    //! Constructor/Destructor
    //! =====================================================
    LocRange(int start, int end);
    ~LocRange();

    //! ====================================================
    //! get/set methods
    //! ====================================================
    int getStart() const;
    int getEnd() const;

    //! =====================================================
    //! Output Methods
    //! ======================================================
    void output(IRHandlesIRInterface& ir) const;

  private:
    int mStart, mEnd;
};


//! ===========================================================
//! A range and a bit to indicate whether the access is to the 
//! full range or just some unknown part of the range
//! ===========================================================

class LocTuple : public virtual Annotation {
  public:

    //! ============================================
    //! Constructor/Destructor
    //! ============================================ 
    LocTuple();

    LocTuple(int start, int end, bool fullOverlap);
    ~LocTuple();

    //! =============================================
    //! get/set methods
    //! =============================================
    LocRange getLocRange() const;

    //! ============================================
    //! Check
    //! ===========================================
    bool isFull() const;

    //! ================================================
    //! Comparision Methods
    //! ================================================
    bool operator==(const LocTuple& other) const;
    bool operator!=(const LocTuple& other) const;
    bool operator<(const LocTuple& other) const;

    //! ======================================================
    //! Output Methods
    //! ======================================================
    void output(IRHandlesIRInterface& ir) const;
       
  private:
    LocRange mRange;
    bool mFullOverlap;
};


//! =======================================================
//! iterator over ids
//! =======================================================

class IdIterator{
  public:

    //! ================================================
    //! Constructor/Destructor
    //! =================================================

    IdIterator(std::map<int,OA_ptr<std::set<LocTuple> > >& aMap);
    ~IdIterator();

    //! ===============================================
    //! Iterator Methods
    //! ===============================================

    void operator++();
    bool isValid() const;
    int current() const;
    void reset();

  private:
    std::set<int> mSet;
    std::set<int>::iterator mIter;
};


//! =====================================================
//! iterator over locations
//! =====================================================

class LocTupleIterator {
  public:

    //! ====================================================
    //! Constructor/Destructor
    //! ====================================================
    
    LocTupleIterator(std::set<LocTuple>& pSet);
    ~LocTupleIterator();

    //! ================================================
    //! Iterator Methods
    //! ================================================

    void operator++();
    bool isValid() const;
    LocTuple current() const;
    void reset();

  private:
    std::set<LocTuple> mSet;
    std::set<LocTuple>::iterator  mIter;
};


//! ====================================================
//! AliasMapXAIF stores set of virtual addresses 
//! virtual addreses are nothing but integer values.
//! ====================================================

class AliasMapXAIF : public virtual Annotation {
  public:

    //!============================================
    //! Constructor/Destructor
    //! ===========================================
    AliasMapXAIF(ProcHandle p);
    ~AliasMapXAIF();

    //! ===================================
    //! Static Member for setId
    //! ===================================
    static const int SET_ID_NONE = -1; 

    //*****************************************************************
    // Info methods unique to Alias::AliasMapXAIF
    //*****************************************************************

    //! =============================================================
    //! get unique id for the alias map set for this memory reference
<<<<<<< .working
    //! This method is NOT used for construction, and CANNOT return SET_ID_NONE
=======
    //! SET_ID_NONE indicates that this memory reference doesn't map 
    //! to any of the existing AliasMap sets
    //! ==============================================================
>>>>>>> .merge-right.r888
    int getMapSetId(MemRefHandle ref); 

    //! =============================================================
    int getMapSetId(std::set<int> aSet) {
        int Id=AliasMapXAIF::SET_ID_NONE;
        std::map<std::set<int>, int>::iterator pos;
        for(pos = mSetToSetId.begin();
            pos != mSetToSetId.end();
            ++pos) {
            std::set<int> bSet = pos->first;
            if(aSet == bSet) {
               Id = pos->second;
            }
        }
        return Id;
    }
 
    //! ==============================================================
<<<<<<< .working
    //! get unique id for the alias map set for this memory reference
    //! SET_ID_NONE indicates that this memory reference doesn't map to any of the existing AliasMap sets
    int findMapSet(MemRefHandle ref);

    //! get id for an alias map set that contains an equivalent set of locations.
    //! SET_ID_NONE indicates that no equivalent location set was found in any AliasMap sets
    int findMapSet(OA_ptr<std::set<LocTuple> > pLocTupleSet);

=======
    //! get id for an alias map set that contains
    //! an equivalent set of locations, SET_ID_NONE indicates 
    //! that no equivalent location set was found in any AliasMap sets 
    //! ==============================================================
    int getMapSetId(OA_ptr<std::set<LocTuple> > pLocTupleSet); 
 
>>>>>>> .merge-right.r888
    //! ==============================================================
    //! get iterator over ids for this alias map
    //! ==============================================================
    OA_ptr<IdIterator> getIdIterator();

    //! ==============================================================
    //! get iterator over all locations in a particular set
    //! ==============================================================
    OA_ptr<LocTupleIterator> getLocIterator(int setId); 

    //*****************************************************************
    // Construction methods 
    //*****************************************************************
    
<<<<<<< .working
=======
    //! ===========================================================
    //! create a new alias map set and return the id
    //! ===========================================================
    int makeEmptySet();
 
>>>>>>> .merge-right.r888
    //! ===========================================================
    //! associate the given location set with the given mapSet
    //! ===========================================================
    void mapLocTupleSet(OA_ptr<std::set<LocTuple> > ltSet, int setId);


    //! ============================================================
    //! associate a MemRefHandle with the given mapSet,
    //! means that the MemRefHandle can access all of the locations
    //! in the mapSet
    //! ============================================================
    void mapMemRefToMapSet(MemRefHandle ref, int setId);

    //! ============================================================
    void mapSetToSetId(std::set<int> aSet, int setId) {
          mSetToSetId[aSet]=setId; 
    }


    //*****************************************************************
    // Output
    //*****************************************************************

    void output(IRHandlesIRInterface& ir) const;

    //! ===========================================================
    //! incomplete output of info for debugging, just lists map 
    //! set Ids and associated set of locations and mapping of 
    //! memrefs to the map set Ids
    //! ============================================================
    void dump(std::ostream& os, OA_ptr<OA::IRHandlesIRInterface> ir);

  private:
    //*******************************************
    // data members
    //******************************************

    //! ==================================================
    //! Procedure, AliasMapXAIF sets are associated with
    //! ==================================================
    ProcHandle mProcHandle; 

    //! ===============================================================
    //! keep track of id mapping to location sets and status with a map
    //! ===============================================================
    std::map<int,OA_ptr<std::set<LocTuple> > > mIdToLocTupleSetMap; 

    //! ==============================================================
    //! what memory references map to this alias map set
    //! ==============================================================
    std::map<int,MemRefSet> mIdToMemRefSetMap;

    //! ==============================================================
    //! the location set a MemRefHandle maps to
    //! ==============================================================
    std::map<MemRefHandle,int> mMemRefToIdMap;  

    //! ==============================================================
    //! set of memrefexpr for a memref
    //! =============================================================
    std::map<MemRefHandle,std::set<OA_ptr<MemRefExpr> > > 
        mMemRefToMRESetMap;
<<<<<<< .working
=======

    //! ================================================
    std::map<std::set<int>, int> mSetToSetId;
>>>>>>> .merge-right.r888
};


  } // end of Alias namespace
} // end of OA namespace

#endif


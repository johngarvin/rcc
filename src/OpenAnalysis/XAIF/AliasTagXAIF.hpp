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

//! ======================= Include Files ===================

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
#include <OpenAnalysis/Utils/Iterator.hpp>

//! ========================================================

namespace OA {

  namespace XAIF {





class AliasTagXAIFIterator : public Iterator {
  public:

    AliasTagXAIFIterator(std::set<int> tags) {
        mTags = tags;
        mIter = mTags.begin();
    }

    ~AliasTagXAIFIterator() { }

    //************************************************************
    // From interface
    //************************************************************
    virtual int current() {
        return (*mIter);
    }

    virtual bool isValid() const {
        return mIter != mTags.end();
    }

    virtual void operator++ () {
        ++mIter;
    }

    void reset() { mIter = mTags.begin(); }

  private:
    std::set<int>::iterator mIter;
    std::set<int> mTags;

};







class AliasTagXAIF : public virtual Annotation {

  public:

     //! ============== Constructor ==================
     AliasTagXAIF(OA_ptr<Alias::Interface> alias);

     //! ============== Destructor ==================
    ~AliasTagXAIF() {}

     //! ================ Data ===================
     static const int SET_ID_NONE = -1;

     //! ================ Construction ===================================

     //! Create the map between MemRefhandle and setId
     void mapMemRef2setId(MemRefHandle mhandle, int setId);

     //! Create the map between setId and vTag 
     void mapsetId2vTag(int setId, OA::OA_ptr<OA::Alias::AliasTagSet> aTag);

     //! Create the map between vTag and partialFlag
     void mapsetId2PartialFlag(int setId, bool flag);

     //! ====================== get Methods ===========================

     int getTagSetId(OA::OA_ptr<OA::Alias::AliasTagSet> aTag);

     int getTagSetId(MemRefHandle mhandle);

     int makeEmptySet();

     //! SetId Iterator
     OA_ptr<AliasTagXAIFIterator> getIdIterator();

     //! get AliasTagsXAIF for the given setId
     OA_ptr<Alias::AliasTagSet> getAliasTagXAIFSet(int setId);

     //! Get the May/Must information
     bool isPartial(int setId);     


     int getStartRange(int setId);

     int getEndRange(int setId);

     //! ==================  Output ====================================

     void output(IRHandlesIRInterface& ir) const;

     void 
     output(OA::IRHandlesIRInterface& ir, OA::Alias::Interface& alias) const;


     //! incomplete output of info for debugging, just lists map 
     //! set Ids and associated set of locations and mapping of 
     //! memrefs to the map set Ids
     void dump(std::ostream& os, OA_ptr<OA::IRHandlesIRInterface> ir);

  private:

     //! ============= Map between MemRefHandle and setId
     std::map<OA::MemRefHandle, int> mMemRef2setId;


     //! ============= Map between setId to vTag =================
     std::map<int, OA::OA_ptr<OA::Alias::AliasTagSet> > msetId2vTag;


     //! ============= Map between vTag to partialFlag =============
     std::map<int, bool> mvsetId2PartialFlag;


     //! ===== keep track of id mapping to location sets and status with a map =====
     static int sCurrentSetId;


     //! ===== Store Alias Analysis Results =====
     OA_ptr<Alias::Interface> mAlias;

};


  } // end of AliasTagXAIF namespace

} // end of OA namespace

#endif


/*! \file
  
  \brief Maps memory references into non-overlapping equivalence classes.

  \authors Michelle Strout
  \version $Id: EquivSets.hpp,v 1.18.10.7 2006/01/05 20:09:34 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef AliasEquivSets_H
#define AliasEquivSets_H

#include <cassert>
#include <iostream>
#include <set>
#include <vector>
#include <queue>
#include <map>

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/Location/Locations.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>

namespace OA {
  namespace Alias {

typedef std::set<MemRefHandle> MemRefSet;
typedef std::set<OA_ptr<Location> > LocSet;


//! general iterator for MemRefHandles
class EquivSetsMemRefIter : public MemRefIterator {
public:
  EquivSetsMemRefIter(OA_ptr<MemRefSet> pSet) : mSet(pSet) 
    { mIter = mSet->begin(); }
  ~EquivSetsMemRefIter() { }

  MemRefHandle current() const { return *mIter; }
  bool isValid()  const { return (mIter != mSet->end()); }
        
  void operator++() { if (mIter!=mSet->end()) ++mIter; }

  void reset() { mIter = mSet->begin(); }

private:
  OA_ptr<MemRefSet>  mSet;
  MemRefSet::iterator mIter;
};


/*!
   Not associated with a particular procedure.
   Just keeping track of may alias.
   Redesigned to be used with FIAlias.  More general applicability in
   terms of storing results for other analysis algorithms is unknown.
 */
class EquivSets : public virtual Annotation, public Alias::Interface {
  public:
    EquivSets();
    ~EquivSets() {}

    static const int SET_ID_NONE = -1; 

    //! indicate the aliasing relationship between two memory references
    AliasResultType alias(MemRefHandle ref1, 
                          MemRefHandle ref2);

    //! iterator over locations that a memory reference may reference
    OA_ptr<LocIterator> getMayLocs(MemRefHandle ref);

    //! iterator over locations that a memory reference must reference
    //! these locations will all have full static overlap
    OA_ptr<LocIterator> getMustLocs(MemRefHandle ref);

    //! iterator over locations that a memory refer expression may reference
    OA_ptr<LocIterator> getMayLocs(MemRefExpr &ref, ProcHandle proc);

    //! iterator over locations that a memory refer expression may reference
    OA_ptr<LocIterator> getMustLocs(MemRefExpr &ref, ProcHandle proc);

    //! get iterator over all memory references that alias information is
    //! available for
    OA_ptr<MemRefIterator> getMemRefIter();

    //*****************************************************************
    // Info methods unique to Alias::EquivSets
    //*****************************************************************

    //*****************************************************************
    // Construction methods 
    //*****************************************************************

    //! associate the given location with the given equivSet
    //! and procedure
    void addLocation(OA_ptr<Location> pLoc, ProcHandle proc, int equivSet);
    
    //! get iterator over all locations in a particular set
    OA_ptr<LocIterator> getLocIterator(ProcHandle proc, int equivSet); 

    //! get the map from set it to location sets.
    OA_ptr<std::map<int,OA_ptr<LocSet> > > getIdToLocSetMap(ProcHandle proc);

    //! associate a MemRefHandle with the given equivSet,
    //! means that the MemRefHandle can access all of the locations
    //! in the equivSet, a MemRefHandle can be mapped to more than
    //! one equiv set because it can be mapped to more than one MRE
    void mapMemRefToEquivSet(MemRefHandle ref, int equivSetId);

    //! associate a MemRefExpr with the given mapSet,
    //! means that the MemRefExpr can access all of the locations
    //! in the mapSet, if only one full location then is a must access
    void mapMemRefToEquivSet(OA_ptr<MemRefExpr> ref, int setId);

    //! associate a MemRefHandle with a given procedure
    void mapMemRefToProc(MemRefHandle ref, ProcHandle proc);

    //! if any of the EquivSets end up mapping to the UnknownLoc
    //! then all EquivSets must be merged
    void mapAllToUnknown();

    //*****************************************************************
    // Output
    //*****************************************************************

    void output(OA::IRHandlesIRInterface& ir);

    //! incomplete output of info for debugging, just lists
    //! sets and text representation of mem refs
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

  private:
OUTPUT
    // data members
      // the procedure each mem ref appears within
GENOUT   std::map<MemRefHandle,ProcHandle> mMemRefToProc;

      // mapping of memory reference handles to equiv sets
      // can be mapped to multiple because can be mapped to
      // multiple MREs
GENOUT   std::map<MemRefHandle,std::set<int> > mMemRefToSets;

       // each procedure will have a set of locations associated
       // with each equivset
GENOUT    std::map<ProcHandle,std::map<int,std::set<OA_ptr<Location> > > > 
       mProcNSetToLocs;

    // the location set a MemRefExpr maps to
    GENOUT std::map<OA_ptr<MemRefExpr>,int> mMREToIdMap;  

};

  } // end of Alias namespace
} // end of OA namespace

#endif


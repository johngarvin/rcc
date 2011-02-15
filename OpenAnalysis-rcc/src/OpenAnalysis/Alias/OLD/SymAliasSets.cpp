/*! \file
  
NOTE: deprecated!!  No longer being compiled.
6/2/06, Removed ManagerInsNoPtrInterAliasMap because it is no longer
valid.  It used the CallGraph data-flow analysis framework
and dependended on the isRefParam() call, which has been deprecated.

  \brief Implementation of SymAliasSets and helper class

  \author Michelle Strout
  \version $Id: SymAliasSets.cpp,v 1.2 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "SymAliasSets.hpp"
#include <Utils/Util.hpp>

namespace OA {
  namespace Alias {

static bool debug = false;

//! copy constructor
SymAliasSets::SymAliasSets(const SymAliasSets& other)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_SymAliasSets:ALL", debug);

    mIdToSymSetMap = other.mIdToSymSetMap;
    mSymToIdMap = other.mSymToIdMap;
    mNextSetId = other.mNextSetId;
}

//! Get unique set id for set that the given symbol maps to 
int SymAliasSets::getAliasSetId(SymHandle sym)
{
    // if the symbol isn't known
    if (mSymToIdMap.find(sym)==mSymToIdMap.end()) {
        return SET_ID_NONE;
    } else {
        return mSymToIdMap[sym];
    }
}

//! Returns iterator over all formal ref params and globals that 
//! are in the same set as the given SymHandle
OA_ptr<SymHandleIterator> SymAliasSets::getSymIterator(SymHandle sym)
{
    OA_ptr<SymHandleIterator> retval;

    // if the symbol isn't known about then return an iterator over
    // an empty set
    if (mSymToIdMap.find(sym)==mSymToIdMap.end()) {
        OA_ptr<SymSet> emptySet;
        emptySet = new SymSet;
        retval = new SymSetIterator( emptySet );
        return retval;

    // otherwise return iterator over set for Id
    } else {
        OA_ptr<SymSet> tempSet;
        tempSet = new SymSet(*(mIdToSymSetMap[mSymToIdMap[sym]]));
        retval = new SymSetIterator( tempSet );
        return retval;
    }
}

//*****************************************************************
// DataFlowSet interface
//*****************************************************************
bool SymAliasSets::operator ==(DataFlow::DataFlowSet &other) const
{
    SymAliasSets& otherRemap = dynamic_cast<SymAliasSets&>(other);

    if (mSymToIdMap == otherRemap.mSymToIdMap) {
        return true;
    } else {
        return false;
    }
}


bool SymAliasSets::operator !=(DataFlow::DataFlowSet &other) const
{
    return ! (*this == other);
}

OA_ptr<DataFlow::DataFlowSet> SymAliasSets::clone()
{
    OA_ptr<SymAliasSets> retval;
    retval = new SymAliasSets(*this);
    return retval;
}

void SymAliasSets::dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> ir)
{
    os << "==== SymAliasSets" << std::endl;
    std::map<int,OA_ptr<SymSet> >::iterator mapIter;
    for (mapIter = mIdToSymSetMap.begin();
         mapIter != mIdToSymSetMap.end(); mapIter++ )
    {
        OA_ptr<SymSet> theSet = mapIter->second;
        theSet->dump(os,ir);
    }

}
void SymAliasSets::dump(std::ostream &os)
{
    os << "==== SymAliasSets" << std::endl;
    std::map<SymHandle,int>::iterator mapIter1;
    for (mapIter1 = mSymToIdMap.begin();
         mapIter1 != mSymToIdMap.end(); mapIter1++ )
    {
        os << "SymHandle(" << mapIter1->first.hval() << ") --> " 
           << mapIter1->second << std::endl;
    }
    std::map<int,OA_ptr<SymSet> >::iterator mapIter;
    for (mapIter = mIdToSymSetMap.begin();
         mapIter != mIdToSymSetMap.end(); mapIter++ )
    {
        os << mapIter->first << " --> ";
        OA_ptr<SymSet> theSet = mapIter->second;
        theSet->dump(os);
    }
}

//*****************************************************************
// Meet routine
//*****************************************************************

/*! If there are any SymSets that overlap between the two SymAliasSets
    then merge those SymSets in the new SymAliasSets data structure.
*/
OA_ptr<SymAliasSets> 
SymAliasSets::meet(SymAliasSets& other) 
{
    if (debug) {
        std::cout << "SymAliasSets::meet" << std::endl;
    }

    // create empty return data structure
    OA_ptr<SymAliasSets> retval;
    retval = new SymAliasSets;

    // loop through other's SymSets and insert them into retval
    std::map<int,OA_ptr<SymSet> >::iterator mapIter;
    for (mapIter = other.mIdToSymSetMap.begin();
         mapIter != other.mIdToSymSetMap.end(); mapIter++ )
    {
        OA_ptr<SymSet> theSet = mapIter->second;
        retval->insertSet(theSet);
    }
    

    // loop through our SymSets
    for (mapIter = mIdToSymSetMap.begin();
         mapIter != mIdToSymSetMap.end(); mapIter++ )
    {
        OA_ptr<SymSet> theSet = mapIter->second;
        bool theSetWasMerged = false;

        // see if intersects any of the SymSets in other
        std::map<int,OA_ptr<SymSet> >::iterator mapIterOther;
        for (mapIterOther = retval->mIdToSymSetMap.begin();
             mapIterOther != retval->mIdToSymSetMap.end(); mapIterOther++ )
        {
            OA_ptr<SymSet> otherSet = mapIterOther->second;

            SymSet temp = *theSet;
            temp.setIntersect(*otherSet);

            // if they intersect then merge them
            if (! temp.empty() ) {
                theSetWasMerged = true;
                OA_ptr<SymSet> unionSet;
                unionSet = theSet;
                unionSet->setUnion(*otherSet);
                retval->replaceSet(mapIterOther->first, unionSet);
            }
        }

        // if theSet wasn't merged then put it in SymAliasSets
        if (theSetWasMerged==false) {
            retval->insertSet(theSet);
        }

    }
        
    return retval;
 }

//*****************************************************************
// Construction methods 
//*****************************************************************
    
//! insert set into datastructure and map all symbols it contains 
//! to the set, only used in this code so assuming that pSet
//! doesn't overlap with any existing sets
void SymAliasSets::insertSet(OA_ptr<SymSet> pSet)
{
    // map to next set id
    mIdToSymSetMap[mNextSetId] = pSet;

    // map all Symbols in the set to this set id
    OA_ptr<SymSetIterator> symIter;
    symIter = new SymSetIterator( mIdToSymSetMap[mNextSetId] );
    for ( ; symIter->isValid(); (*symIter)++ ) {
        mSymToIdMap[symIter->current()] = mNextSetId;
    }
    
    mNextSetId++;
}

//! replace the set with the given set id with the given set
void SymAliasSets::replaceSet(int oldSetId, OA_ptr<SymSet> pSet)
{
    // map to oldSetId
    mIdToSymSetMap[oldSetId] = pSet;

    // map all Symbols in the set to this set id
    OA_ptr<SymSetIterator> symIter;
    symIter = new SymSetIterator( mIdToSymSetMap[oldSetId] );
    for ( ; symIter->isValid(); (*symIter)++ ) {
        mSymToIdMap[symIter->current()] = oldSetId;
    }
}


//! indicate the two formal params and/or globals may alias each
//! other at a particular call therefore should be in same map
void SymAliasSets::mergeSyms(SymHandle sym1, SymHandle sym2)
{

    // look at what both symbols are mapped to
    int symSet1 = getAliasSetId(sym1);
    int symSet2 = getAliasSetId(sym2);

    // if neither of them are in a set yet then make a set, put them
    // in it, and insert set
    if (symSet1==SET_ID_NONE && symSet2==SET_ID_NONE) {
        OA_ptr<SymSet> newSet; newSet = new SymSet;
        newSet->insert(sym1);
        newSet->insert(sym2);
        insertSet(newSet);
    }

    // if they are in different sets
    // then create a new set with them both in it and insert the new set
    // also remove old sets
    if (symSet1!=symSet2) {
        // just insert sym1 into symSet2 if sym1 is not in a set already
        if (symSet1==SET_ID_NONE) {
            mIdToSymSetMap[symSet2]->insert(sym1);
            mSymToIdMap[sym1] = symSet2;
            
        // same goes for symSet2
        } else if (symSet2==SET_ID_NONE) {
            mIdToSymSetMap[symSet1]->insert(sym2);
            mSymToIdMap[sym2] = symSet1;

        // otherwise merge the two sets
        } else {
            // union them into the first set
            mIdToSymSetMap[symSet1]->setUnion(*mIdToSymSetMap[symSet2]);

            // map everything in second set to the first set
            OA_ptr<SymSetIterator> symIter;
            symIter = new SymSetIterator( mIdToSymSetMap[symSet2] );
            for ( ; symIter->isValid(); (*symIter)++ ) {
                mSymToIdMap[symIter->current()] = symSet1;
            } 

            // replace second set with an empty set
            OA_ptr<SymSet> emptySet; emptySet = new SymSet;
            mIdToSymSetMap[symSet2] = emptySet;
        }
    }
}

  } // end of Alias namespace
} // end of OA namespace

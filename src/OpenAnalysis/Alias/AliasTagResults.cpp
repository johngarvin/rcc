/*! \file
  
  \brief Implementation of Alias::AliasTagResults.

  \authors Michelle Strout

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "AliasTagResults.hpp"

using namespace std;


namespace OA {
  namespace Alias {

static bool debug = false;

AliasTagResults::AliasTagResults() {
    OA_DEBUG_CTRL_MACRO("DEBUG_AliasTagResults:ALL", debug);
}

AliasResultType 
AliasTagResults::alias(MemRefHandle ref1, MemRefHandle ref2)
{
    AliasResultType retval = MAYALIAS;
    // FIXME: NOT implemented yet.
    // If AliasTagSet for ref1 has a non-empty intersection with
    // the AliasTagSet for ref2 then they alias.
    // If either of their sets are may, then the alias is a may alias. 
    
    return retval;
}
       
OA_ptr<AliasTagSet> 
AliasTagResults::getAliasTags( OA_ptr<MemRefExpr> mre )
{
    OA_ptr<AliasTagSet> tagSet, retval;

    // look up the alias tag set for the mre in a map
    tagSet = mMREToTags[mre];
    if(tagSet.ptrEqual(NULL)) {
        retval = new AliasTagSet(false);
    } else {
        retval = tagSet->clone();
    }
    return retval;
}

OA_ptr<AliasTagSet> 
AliasTagResults::getAliasTags( MemRefHandle memref )
{
    OA_ptr<AliasTagSet> tagSet, retval;

    // look up the alias tag set for the mre in a map
    tagSet = mMemRefToTags[memref];
    if(tagSet.ptrEqual(NULL)) {
        retval = new AliasTagSet(false);
    } else {
        retval = tagSet->clone();
    }
    return retval;
}

//! Return the maximum value for all of the alias tags this class knows
//! about.  Range of alias tags will be from 0 through this value.
AliasTag 
AliasTagResults::getMaxAliasTag() 
{
    std::map<AliasTag, OA_ptr<std::set<MemRefHandle> > >::iterator iter;
    AliasTag max(0);
    for (iter=mTagToMemRefs.begin(); iter != mTagToMemRefs.end(); iter++) {
        if (max < iter->first ) {
            max = iter->first;
        }
    }
    return max;
}

//! Given an AliasTag, returns an iterator over MemRefExprs that 
//! may access the given tag.
OA_ptr<MemRefExprIterator> 
AliasTagResults::getMemRefExprIterator(AliasTag tag) const
{
    OA_ptr<MemRefExprIterator> retval;

    retval = new MemRefExprIterator(mTagToMREs.find(tag)->second);

    return retval;
}



    
//*****************************************************************
// Construction methods 
//*****************************************************************

void 
AliasTagResults::mapMemRefToAliasTagSet(
        OA_ptr<MemRefExpr> mre, OA_ptr<AliasTagSet> alias_tag_set )
{
    // map MRE to set of tags
    mMREToTags[mre] = alias_tag_set;

    // for each tag in the set, map tag to MRE
    OA_ptr<AliasTagIterator> tagIter = alias_tag_set->getIterator();
    AliasTag alias_tag;
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
        alias_tag = tagIter->current();
        if(mTagToMREs[alias_tag].ptrEqual(NULL)) {
            mTagToMREs[alias_tag] = new set<OA_ptr<MemRefExpr> >();
        }

        mTagToMREs[alias_tag]->insert(mre);
    }
    // set may/must flag
    mMREToMustFlag[mre] = alias_tag_set->isMust();
     
}

//! Associate a MemRefHandle with given alias tag
void 
AliasTagResults::mapMemRefToAliasTagSet( 
        MemRefHandle ref, OA_ptr<AliasTagSet> alias_tag_set )
{
   
    // map memref handle to set of tags
    mMemRefToTags[ref] = alias_tag_set;

    // for each tag in the set, map tag to MemRefHandle
    OA_ptr<AliasTagIterator> tagIter = alias_tag_set->getIterator();
    AliasTag alias_tag;
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
        alias_tag = tagIter->current();
        if(mTagToMemRefs[alias_tag].ptrEqual(NULL)) {
            mTagToMemRefs[alias_tag] = new std::set<MemRefHandle >();
        }

        mTagToMemRefs[alias_tag]->insert(ref);
    }

    // set may/must flag
    mMemRefToMustFlag[ref] = alias_tag_set->isMust();
    
}

  } // end of Alias namespace
} // end of OA namespace



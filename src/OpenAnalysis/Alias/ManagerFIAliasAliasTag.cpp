/*! \file
  
  \brief The AnnotationManager that generates AliasTagResults
         with FIAlias.

  \authors Michelle Strout

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerFIAliasAliasTag.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace Alias {

static bool debug = false;

/*!
*/
ManagerFIAliasAliasTag::ManagerFIAliasAliasTag(OA_ptr<AliasIRInterface> _ir) 
   : ManagerFIAlias(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerFIAliasAliasTag:ALL", debug);
}

OA_ptr<Alias::Interface> 
ManagerFIAliasAliasTag::performAnalysis( OA_ptr<IRProcIterator> procIter,
                                         FIAliasImplement implement )
{

    // create an empty set of results
    OA_ptr<AliasTagResults> retval;
    retval = new AliasTagResults();

    // Invoke the ManagerFIAlias routine to create the alias sets
    // within the union-find universe.
    OA_ptr<UnionFindUniverse> ufset = performFIAlias(procIter, implement);

    // Depending on the implementation of FIAlias, only a 
    // subset of the defined procedures may have been analyzed.
    OA_ptr<IRProcIterator> analyzedProcIter = getAnalyzedProcIter();

    //--------------------------------------------------------------
    // Convert the union-find universe to mappings to alias tag sets

    // map each ufsetID to a tag id by just using a counter
    std::map<int,AliasTag> ufsetIDToTag;
    // need a map for strictly local named refs as well
    std::map<OA_ptr<MemRefExpr>,AliasTag> strictlyLocalToTag;

    // for each ufsetID keep track of alias tags for sets
    // and strictly locals that it subsumes
    std::map<int,std::set<AliasTag> > ufsetToSubsumes;

    // keeps track of which ufsets have subset ufsets
    // that it subsumes
    std::map<int,bool> ufsetSubsumesSubsets;

    int count = 1;
    // visit all of the MREs in the program being analyzed
    // to get the set of valid ufsetIDs.
    if(debug) {
        std::cout << "Assign alias tag numbers to each UF set" << std::endl;
    }

    std::set<int> ufsetIDs;
    std::map<OA_ptr<MemRefExpr>,int>::iterator mreIter;
    for (mreIter=mMREToID.begin(); mreIter!=mMREToID.end(); mreIter++ ) {
        OA_ptr<MemRefExpr> mre = mreIter->first;

        // only map those MREs that do not involve an addressOf operation
        if(mre->isaRefOp()) {
           OA_ptr<RefOp> refop = mre.convert<RefOp>();
           if(refop->isaAddressOf()) { continue; }
        }

        // get ufset for MRE
        int ufsetID = ufset->Find(mreIter->second);
        ufsetIDs.insert(ufsetID);

        if(debug) {
            std::cout << "\t" << mre->comment(*mIR) << " in UFset "
                 << ufsetID << std::endl;
        }
        // initially assume that ufset does not subsume any ufset
        // due to subsetRef
        ufsetSubsumesSubsets[ufsetID] = false;

        // only give this set an alias tag if it is the first
        // time we have seen it
        if (ufsetIDToTag.find(ufsetID)==ufsetIDToTag.end()) {
            if(debug) {
                std::cout << "\t\tUFSet " << ufsetID << " mapped to tag " << count
                          << std::endl;
            }

            ufsetIDToTag[ufsetID] = AliasTag(count++);
        }
        
        // give strictly locals their own alias tag and have
        // the ufset they are in subsume that alias tag
        if (mre->isaNamed()) {
          OA_ptr<NamedRef> namedRef = mre.convert<NamedRef>();
          if (namedRef->isStrictlyLocal()) {
            if (strictlyLocalToTag.find(mre)==strictlyLocalToTag.end()) {
                strictlyLocalToTag[mre] = AliasTag(count++);
            }
            ufsetToSubsumes[ufsetID].insert(strictlyLocalToTag[mre]);
          }
        }

    }

    // Map each ufset to a set of ufset alias tags that it subsumes 
    // including itself.
    // A ufset subsumes all ufsets that include subsetrefs to the MREs
    // it contains.  Also a ufset subsumes all alias tags given to 
    // strictly local NamedRefs that it contains.
    if(debug) {
        std::cout << "Mapping each UFSet to set of Alias Tags it subsumes due to SubSetRef:" << std::endl;
    }
    // loop through the ufsetIDs 
    std::set<int>::iterator setIter;
    for (setIter=ufsetIDs.begin(); setIter!=ufsetIDs.end(); setIter++ ) {
        int ufsetID = *setIter;
        
        if(debug) {
            std::cout << "\tConsidering ufset " << ufsetID << std::endl;
        }

        // we need to subsume self so that all the MemRefHandles
        // and MREs get mapped appropriately
        ufsetToSubsumes[ufsetID].insert(ufsetIDToTag[ufsetID]);
        if(debug) {
            std::cout << "\tMap self as a subsumed set" << std::endl;
        }

        // loop through the justRefop keys
        // for this ufset
        std::map<OA_ptr<MemRefExpr>,int> outerRefToUFSetID = mMap[ufsetID];
        std::map<OA_ptr<MemRefExpr>,int>::iterator outerRefIter;
        
        for (outerRefIter=outerRefToUFSetID.begin();
             outerRefIter!=outerRefToUFSetID.end();
             outerRefIter++) 
        {
            OA_ptr<MemRefExpr> outerRef = outerRefIter->first;

            // if the refOp is a SubSetRef of some kind
            if (outerRef->isaSubSetRef()) {
                if(debug) {
                    std::cout << "\t\tSubsumes ufset containing "
                         << outerRef->comment(*mIR) << std::endl;
                    std::cout << "\t\t\tWhich is "
                         << ufset->Find( outerRefIter->second ) << std::endl;
                }
                ufsetToSubsumes[ufsetID].insert( ufsetIDToTag[ufset->Find(
                    outerRefIter->second )]);
                ufsetSubsumesSubsets[ufsetID] = true;
            }
        }
    }

    // map each mre to set of alias tags using the above two maps
    // for each mre
    for (mreIter=mMREToID.begin(); mreIter!=mMREToID.end(); mreIter++ ) {
        OA_ptr<MemRefExpr> mre = mreIter->first;

        // only map those MREs that do not involve an addressOf operation
        if(mre->isaRefOp()) {
           OA_ptr<RefOp> refop = mre.convert<RefOp>();
           if(refop->isaAddressOf()) { continue; }
        }

        if(debug) {
          std::cout << "Map alias tags for: " << mre->comment(*mIR) << std::endl;
        }
        
        // find the ufset for the mre
        int ufsetID = ufset->Find(mreIter->second);

        if(debug) {
            std::cout << "\tWhich is in UFSet " << ufsetID << std::endl;
        }

        // Create AliasTagSet for MRE.
        // All alias tags set should be may EXCEPT for strictly
        // local named refs when the ufset that the strictly local
        // is in only subsumes itself.
        // The strictly local must also be in a ufset that does NOT
        // subsume any other ufsets due to SubsetRefs.
        // Has to be this way because there is no guarantee that a
        // single strictly local in a ufset subsumes all of the sets
        // involving subset refs for the ufset.
        bool isMust=false;
        OA_ptr<AliasTagSet> tag_set;
        if (strictlyLocalToTag.find(mre)!=strictlyLocalToTag.end()
            && !ufsetSubsumesSubsets[ufsetID]) 
        {
            isMust = true;
            tag_set = new AliasTagSet(isMust);
            tag_set->insert(strictlyLocalToTag[mre]); 

        // May alias
        } else {
            isMust = false;
            tag_set = new AliasTagSet(isMust);
            tag_set->insert(ufsetIDToTag[ ufsetID ]);

            // for each subsumed alias tag for MREs ufset 
            std::set<AliasTag>::iterator setIter;
            for (setIter =ufsetToSubsumes[ufsetID].begin();
                setIter!=ufsetToSubsumes[ufsetID].end(); setIter++)
            {
                tag_set->insert(*setIter);
            }

        }

        // map MRE and all MemRefHandles for that MRE to 
        // created alias tag set
        retval->mapMemRefToAliasTagSet(mre, tag_set);

        // for each MemRefHandle for mre
        std::set<MemRefHandle>::iterator memrefIter;
        for(memrefIter=mMREToMemRefHandles[mre].begin();
            memrefIter!=mMREToMemRefHandles[mre].end();
            memrefIter++)
        {
            retval->mapMemRefToAliasTagSet(*memrefIter, tag_set); 
        }
    }

    return retval;
}


  } // end of namespace Alias
} // end of namespace OA

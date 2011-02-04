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

//! =============== Header =============
#include "AliasTagXAIF.hpp"
//! ====================================

namespace OA {
  namespace XAIF {

  static bool debug = false;

  //! sCurrentSetId should start with 2 because 0 and 1 are default.
  //! PLM: I still dont understand clearly what exactly 0 and 1 represent
  //!      but I am considering to make current algorithm and XAIF output
  //!      consistent with the OpenAD output so that I can check MITgcm
  //!      Activity Analysis.

  int AliasTagXAIF::sCurrentSetId = 2;


  AliasTagXAIF::AliasTagXAIF(OA_ptr<Alias::Interface> alias)
    : mAlias(alias) {
  }


  void AliasTagXAIF::mapMemRef2setId(MemRefHandle mhandle, int setId)
  {
       if(mMemRef2setId.find(mhandle) == mMemRef2setId.end()) {

          mMemRef2setId[mhandle] = setId;

       }
  }


  void 
  AliasTagXAIF::mapsetId2vTag(int setId, OA::OA_ptr<OA::Alias::AliasTagSet> aTag) {

        if (msetId2vTag[setId].ptrEqual(0)) {
            msetId2vTag[setId] = new OA::Alias::AliasTagSet(aTag->isMust());
        }

        msetId2vTag[setId] = aTag;
  }


  void 
  AliasTagXAIF::mapsetId2PartialFlag( int setId, bool flag) {

        if(mvsetId2PartialFlag.find(setId) == mvsetId2PartialFlag.end()) {

           mvsetId2PartialFlag[setId] = flag;

        }

  }

  
  int AliasTagXAIF::getTagSetId(OA::OA_ptr<OA::Alias::AliasTagSet> aTag) {

        int retval = AliasTagXAIF::SET_ID_NONE;

        OA_ptr<DataFlow::DataFlowSet> dset1 = aTag->getDataFlowSet();  
        OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > dImpl1;
        dImpl1 = dset1.convert<DataFlow::DataFlowSetImpl<Alias::AliasTag> >();

        std::map<int, OA::OA_ptr<OA::Alias::AliasTagSet> >::iterator aIt;
        for(aIt = msetId2vTag.begin(); aIt != msetId2vTag.end(); ++aIt)
        {
                OA_ptr<Alias::AliasTagSet> bTag = aIt->second;
                OA_ptr<DataFlow::DataFlowSet> dset2 = bTag->getDataFlowSet();
                OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > dImpl2;
                dImpl2 = dset2.convert<DataFlow::DataFlowSetImpl<Alias::AliasTag> >();

                if(dImpl1 == dImpl2) {
                   retval = aIt->first;
                }
        }

        return retval;
  }


  

  int AliasTagXAIF::getTagSetId(MemRefHandle mhandle) {

        return mMemRef2setId[mhandle];

  }



  OA_ptr<AliasTagXAIFIterator> AliasTagXAIF::getIdIterator()
  {
         OA_ptr<AliasTagXAIFIterator> aIter; 
         std::set<int> aSet;

         std::map<int, OA::OA_ptr<OA::Alias::AliasTagSet> >::iterator msetId2vTag_iter;

         for(msetId2vTag_iter = msetId2vTag.begin(); msetId2vTag_iter != msetId2vTag.end();
             ++msetId2vTag_iter) {
             aSet.insert(msetId2vTag_iter->first);
         }

         aIter = new AliasTagXAIFIterator(aSet);
         return aIter;
  }


  OA_ptr<Alias::AliasTagSet> AliasTagXAIF::getAliasTagXAIFSet(int setId)
  {
  
         //! Create dummy AliasTagSet

         OA_ptr<Alias::AliasTagSet> aSet;

         aSet = new Alias::AliasTagSet(false);
 
         //! ===== Locate setId in the AliasTagXAIF =====

         std::map<int, OA::OA_ptr<OA::Alias::AliasTagSet> >::const_iterator pos;

         pos = msetId2vTag.find(setId);


         //! ===== Found setId =====

         if (pos!=msetId2vTag.end()) {

             //! ===== Get the AliasTagSet =====

             aSet = pos->second;

         } //! ===== end if =====          

         return aSet;

  }



  bool AliasTagXAIF::isPartial(int setId) {

         //! ===== Default, set partial flag =====

         bool partial = false;

         //! ===== Locate setId =====

         std::map<int, bool>::const_iterator pos;

         pos = mvsetId2PartialFlag.find(setId);

         //! ===== Found setId =====

         if (pos!=mvsetId2PartialFlag.end()) {

             //! ===== Get the May/Must status =====

             partial = pos->second;

         } //! ===== end if =====

         //! Return partialFlag

         return partial;

  }



  int AliasTagXAIF::getStartRange(int setId)
  {
         int Srange = -1;

         //! Create dummy AliasTagSet

         OA_ptr<Alias::AliasTagSet> aSet;

         aSet = new Alias::AliasTagSet(false);

         //! ===== Locate setId in the AliasTagXAIF =====

         std::map<int, OA::OA_ptr<OA::Alias::AliasTagSet> >::const_iterator pos;

         pos = msetId2vTag.find(setId);


         //! ===== Found setId =====

         if (pos!=msetId2vTag.end()) {

             //! ===== Get the AliasTagSet =====

             aSet = pos->second;

             OA::OA_ptr<OA::Alias::AliasTagIterator> aIter;

             aIter = aSet->getIterator();

             Srange = aIter->current().val(); 

         } //! ===== end if =====

         return Srange;
      
  }

  int AliasTagXAIF::getEndRange(int setId)
  {

         int Erange = -1;

         //! Create dummy AliasTagSet

         OA_ptr<Alias::AliasTagSet> aSet;

         aSet = new Alias::AliasTagSet(false);

         //! ===== Locate setId in the AliasTagXAIF =====

         std::map<int, OA::OA_ptr<OA::Alias::AliasTagSet> >::const_iterator pos;

         pos = msetId2vTag.find(setId);


         //! ===== Found setId =====

         if (pos!=msetId2vTag.end()) {

             //! ===== Get the AliasTagSet =====

             aSet = pos->second;

             OA::OA_ptr<OA::Alias::AliasTagIterator> aIter;

             aIter = aSet->getIterator();

             for( ; aIter->isValid(); ++(*aIter) ) { 

                   OA::Alias::AliasTag atag = aIter->current();

                   Erange = atag.val(); 
             }

         } //! ===== end if =====

         return Erange;

  }


  //! create an empty alias map set and return the id
  int AliasTagXAIF::makeEmptySet()
  {
      int newsetId = sCurrentSetId;
      sCurrentSetId++;
      return newsetId;
  }


  void 
  AliasTagXAIF::dump(std::ostream& os, OA_ptr<OA::IRHandlesIRInterface> ir) {
  }


  void AliasTagXAIF::output(IRHandlesIRInterface& ir) const {

       output(ir,*mAlias);
       /*
       std::cout << "Please use the AliasMapXAIF output with Alias::Results" 
                 << std::endl;
       assert(0);
       */
  }

  void AliasTagXAIF::output(OA::IRHandlesIRInterface& ir, 
                            OA::Alias::Interface& alias) const
  {

       sOutBuild->mapStart("setId2vTag", "int", "OA::OA_ptr<OA::Alias::AliasTagSet> ");


    std::map<int, OA::OA_ptr<OA::Alias::AliasTagSet> >::const_iterator
                   setId2vTag_iterator;

    for(setId2vTag_iterator = msetId2vTag.begin(); 
        setId2vTag_iterator != msetId2vTag.end();
        setId2vTag_iterator++)
    {
        const int &first = setId2vTag_iterator->first;
        const OA::OA_ptr<OA::Alias::AliasTagSet>  second 
               = setId2vTag_iterator->second;

        if ( second.ptrEqual(0) ) { continue; }

        sOutBuild->mapEntryStart();
          sOutBuild->mapKey(OA::int2string(first));
            sOutBuild->mapValueStart();
             sOutBuild->listStart();

              OA_ptr<DataFlow::DataFlowSet> dset = second->getDataFlowSet(); 
              OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > remapSet1;
              remapSet1 = dset.convert<DataFlow::DataFlowSetImpl<Alias::AliasTag> >();

              OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > aIter;
              aIter = new DataFlow::DataFlowSetImplIterator<Alias::AliasTag>(remapSet1);

              for (; aIter->isValid(); ++(*aIter)) {
                   OA::Alias::AliasTag aTag = aIter->current();
                   sOutBuild->listItemStart();
                      aTag.output(ir, alias);
                   sOutBuild->listItemEnd();
              }
             sOutBuild->listEnd();
           sOutBuild->mapValueEnd();
         sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("setId2vTag");




    sOutBuild->mapStart("setId2PartialFlag", "int", "bool");

    std::map<int, bool>::const_iterator mvsetId2PartialFlag_iterator;
    for( mvsetId2PartialFlag_iterator = mvsetId2PartialFlag.begin();
         mvsetId2PartialFlag_iterator != mvsetId2PartialFlag.end();
         ++mvsetId2PartialFlag_iterator )
    {
         const int &first = mvsetId2PartialFlag_iterator->first;        
         const bool &second = mvsetId2PartialFlag_iterator->second; 

         sOutBuild->mapEntryStart();
           sOutBuild->mapKey(OA::int2string(first));
           sOutBuild->mapValueStart();
              sOutBuild->mapValue(OA::bool2string(second));
           sOutBuild->mapValueStart(); 
         sOutBuild->mapEntryEnd();  
    }
    sOutBuild->mapEnd("setId2PartialFlag");

 

 
    sOutBuild->mapStart("mMemRef2setId", "MemRefHandle", "int");

    std::map<MemRefHandle, int>::const_iterator mMemRef2setId_iterator;
    for( mMemRef2setId_iterator = mMemRef2setId.begin();
         mMemRef2setId_iterator != mMemRef2setId.end();
         ++mMemRef2setId_iterator )
    {
         const MemRefHandle first = mMemRef2setId_iterator->first;
         const int second = mMemRef2setId_iterator->second;

         sOutBuild->mapEntryStart();
           sOutBuild->mapKeyStart();
             sOutBuild->outputIRHandle(first, ir);
           sOutBuild->mapKeyEnd();
           sOutBuild->mapValueStart();
             sOutBuild->mapValue(OA::int2string(second));
           sOutBuild->mapValueStart();
         sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mMemRef2setId");
 
}



  } // end of Alias namespace
} // end of OA namespace

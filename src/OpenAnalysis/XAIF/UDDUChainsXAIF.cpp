/*! \file
  
  \brief Implementation of XAIF::UDDUChainsXAIF

  \author Michelle Strout
  \version $Id: UDDUChainsXAIF.cpp,v 1.12 2005/01/17 18:46:34 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <OpenAnalysis/XAIF/UDDUChainsXAIF.hpp>

#include <OpenAnalysis/Utils/OutputBuilder.hpp>
namespace OA {
  namespace XAIF {

    UDDUChainsXAIF::UDDUChainsXAIF() {
      // initialize the extra map
      mMemRefToIdMap = new std::map<MemRefHandle,int>;
    }
 
    OA_ptr<UDDUChainsXAIF::ChainIterator> 
    UDDUChainsXAIF::getUDChainIterator(MemRefHandle h) {
      OA_ptr<ChainIterator> retval;
      // if haven't seen this MemRef before then just create an empty set
      // and return an iterator over that
      std::map<MemRefHandle,int>::iterator pos = mMemRefToIdMap->find(h);
      if (pos == mMemRefToIdMap->end()) {
	OA_ptr<StmtSet> emptySet;  emptySet = new StmtSet;
	retval = new ChainIterator(emptySet);
 	// otherwise return an iterator over the set with which
	// this mem ref is associated
      } else {
	retval = new ChainIterator((*mIdToStmtSetMap)[pos->second]);
      } 
      return retval; 
    }

    OA_ptr<UDDUChainsXAIF::ChainIterator> 
    UDDUChainsXAIF::getDUChainIterator(MemRefHandle h) {
      // DU and UD chains are stored in the same fashion
      return getUDChainIterator(h);
    }

    int UDDUChainsXAIF::getUDDUChainId(MemRefHandle ref) {
      int retval;
      // if have seen this MemRef before then return setId
      std::map<MemRefHandle,int>::iterator pos = mMemRefToIdMap->find(ref);
      if (pos != mMemRefToIdMap->end()) {
	retval = pos->second;
      } else {
	// we don't have a chaing for this memory reference so indicate that
	// it has an empty chain by returning the zeroth chain
	retval = 0;
      }
      return retval;
    }

    OA_ptr<UDDUChainsXAIF::MemRefsWithChainIterator> 
    UDDUChainsXAIF::getMemRefsWithChainIterator() {
      OA_ptr<MemRefsWithChainIterator> retval;
      retval = new MemRefsWithChainIterator(mMemRefToIdMap);
      return retval;
    }
    
    void UDDUChainsXAIF::insertInto(MemRefHandle ref, int setId) {
      (*mMemRefToIdMap)[ref] = setId;
    }

    void UDDUChainsXAIF::output(OA::IRHandlesIRInterface& ir) const { 

         sOutBuild->objStart("UDDUChainsXAIF");

         ChainsXAIF::output(ir);

         sOutBuild->mapStart("mMemRefToIdMap", "MemRefHandle", "int");

         std::map<MemRefHandle,int>::const_iterator mIter;
         for(mIter = mMemRefToIdMap->begin(); mIter != mMemRefToIdMap->end(); ++mIter)
         {
             const MemRefHandle mhandle = mIter->first;
             const int Id = mIter->second;

             sOutBuild->mapEntryStart();
               sOutBuild->mapKeyStart();
                  sOutBuild->outputIRHandle(mhandle, ir);
               sOutBuild->mapKeyEnd();
                  sOutBuild->mapValue(OA::int2string(Id));
             sOutBuild->mapEntryEnd();
         }
         sOutBuild->mapEnd("mMemRefToIdMap");

         sOutBuild->objEnd("UDDUChainsXAIF");
    }

    void UDDUChainsXAIF::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir) {
      ChainsXAIF::dump(os,ir);
      
      // print all memrefs and their mapping
      os << "MemRef mapping to UDDUChains sets:" << std::endl;
      OA_ptr<MemRefsWithChainIterator> memRefIterPtr
        = getMemRefsWithChainIterator();
      for ( ; memRefIterPtr->isValid(); (*memRefIterPtr)++ ) {
	MemRefHandle memref = memRefIterPtr->current();
	int setId = getUDDUChainId(memref);
	os << "(" << memref.hval() << ") " << ir->toString(memref);
	os << " --> " << setId << std::endl;
      }
    }

  } 
}

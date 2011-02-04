/*! \file
  
  \brief Implementation of XAIF::ChainsXAIF

  \author Michelle Strout
  \version $Id: ChainsXAIF.cpp,v 1.12 2005/01/17 18:46:34 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <OpenAnalysis/XAIF/ChainsXAIF.hpp>

#include <OpenAnalysis/Utils/OutputBuilder.hpp>
namespace OA {
  namespace XAIF {

    ChainsXAIF::ChainsXAIF() {
      // initialize the maps
      mIdToStmtSetMap = new std::map<int,OA_ptr<StmtSet> >;
      // have the first set be empty
      StmtSet emptySet;
      addStmtSet(emptySet,1);
      // start out with StmtHandle(0) in the second set
      addStmt(StmtHandle(0),2);
    }

    OA_ptr<ChainsXAIF::ChainIterator> 
    ChainsXAIF::getChainIterator(int id) {
      OA_ptr<ChainIterator> retval;
      
      // if don't have a chain for this id then just create an empty one
      // and return an iterator over that
      std::map<int,OA_ptr<StmtSet> >::iterator pos = mIdToStmtSetMap->find(id);
      if (pos == mIdToStmtSetMap->end()) {
	OA_ptr<StmtSet> emptySet;  emptySet = new StmtSet;
	retval = new ChainIterator(emptySet);
	// else create an iterator for requested set
      } else {
	retval = new ChainIterator(pos->second);
      }
      return retval;
    }
 
    OA_ptr<ChainsXAIF::ChainsIterator> ChainsXAIF::getChainsIterator() {
      OA_ptr<ChainsIterator> retval;
      retval = new ChainsIterator(mIdToStmtSetMap);
      return retval;
    }
    
    void ChainsXAIF::addStmt(StmtHandle stmt, int id) {
      if ((*mIdToStmtSetMap)[id].ptrEqual(0)) {
	(*mIdToStmtSetMap)[id] = new StmtSet;
      }
      (*mIdToStmtSetMap)[id]->insert(stmt);
    }

    void ChainsXAIF::addStmtSet(StmtSet& stmtSet, int id) {
      if ((*mIdToStmtSetMap)[id].ptrEqual(0)) {
	(*mIdToStmtSetMap)[id] = new StmtSet;
      }
      // just iterate over stmtSet and insert each entry
      StmtSet::iterator setIter;
      for (setIter=stmtSet.begin(); setIter!=stmtSet.end(); setIter++) {
	(*mIdToStmtSetMap)[id]->insert(*setIter);
      }
    }

    int ChainsXAIF::findChain(StmtSet& stmtSet) {
      int retval = CHAIN_ID_NONE;
      OA_ptr<ChainsIterator> chainIterPtr = getChainsIterator();
      for ( ; chainIterPtr->isValid(); ++(*chainIterPtr)) {
	if (*((*mIdToStmtSetMap)[chainIterPtr->currentId()]) == stmtSet) {
	  retval = chainIterPtr->currentId();
	}
      }
      return retval;
    }




    void ChainsXAIF::output(OA::IRHandlesIRInterface& ir) const {

         sOutBuild->objStart("ChainsXAIF");

         sOutBuild->mapStart("mIdToStmtSetMap", "int", "OA_ptr<StmtSet>");

         std::map<int,OA_ptr<StmtSet> >::const_iterator mIter;
         for(mIter = mIdToStmtSetMap->begin(); mIter != mIdToStmtSetMap->end(); ++mIter)
         {
             const int Id = mIter->first;

             sOutBuild->mapEntryStart();
             sOutBuild->mapKeyStart();
                 sOutBuild->mapValue(OA::int2string(Id));
             sOutBuild->mapKeyEnd();

             const OA_ptr<StmtSet> sSet = mIter->second;

             StmtSet::const_iterator sIter;
             for(sIter = sSet->begin(); sIter != sSet->end(); ++sIter) {
                 StmtHandle stmt = *(sIter);
                 sOutBuild->mapValueStart();
                     sOutBuild->outputIRHandle(stmt, ir);
                 sOutBuild->mapValueEnd();
             }
             sOutBuild->mapEntryEnd();
         }

         sOutBuild->mapEnd("ChainsXAIF");

    }




    void ChainsXAIF::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
    {
      // print statemetns for each set, ID : { stmts }
      os << "============= ChainsXAIF ============" << std::endl;
      OA_ptr<ChainsIterator> chainIterPtr = getChainsIterator();
      for ( ; chainIterPtr->isValid(); ++(*chainIterPtr)) {
	os << "ChainsSet[" << chainIterPtr->currentId() << "] = { ";
	OA_ptr<ChainIterator> siterPtr = chainIterPtr->currentChainIterator();
	for ( ; siterPtr->isValid(); (*siterPtr)++ ) {
	  StmtHandle stmt = siterPtr->current();
	  os << ", ";
	  os << "<" << ir->toString(stmt) << "> ";
	}
	os << " }" << std::endl;
      }
    }

  } 
} 

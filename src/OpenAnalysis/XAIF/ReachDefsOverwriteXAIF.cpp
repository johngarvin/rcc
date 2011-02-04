/*! \file
  
  \brief Implementation of XAIF::ReachDefsOverwriteXAIF

  \author Jean Utke

  See ../../../Copyright.txt for details. <br>

*/

#include <OpenAnalysis/XAIF/ReachDefsOverwriteXAIF.hpp>

namespace OA {
  namespace XAIF {

    ReachDefsOverwriteXAIF::ReachDefsOverwriteXAIF() {
      // initialize the extra map
      mStmtToIdMap = new std::map<StmtHandle,int>;
    }

    ReachDefsOverwriteXAIF::~ReachDefsOverwriteXAIF() {}

    int ReachDefsOverwriteXAIF::getChainId(StmtHandle stmt) {
      int retval;
      // if have seen this stmt before then return setId
      std::map<StmtHandle,int>::iterator pos = mStmtToIdMap->find(stmt);
      if (pos != mStmtToIdMap->end()) {
	retval = pos->second;
      } else {
	// we don't have a chaing for this memory reference so indicate that
	// it has an empty chain by returning the zeroth chain
	retval = 0;
      }
      return retval;
    }

    void ReachDefsOverwriteXAIF::insert(StmtHandle ref, int chainId) {
      (*mStmtToIdMap)[ref] = chainId;
    }

  } 
}

/*! \file
  
\brief Implementation of ReachDefsOverwrite::ReachDefsOverwriteStandard

Copyright (c) 2002-2005, Rice University <br>
Copyright (c) 2004-2005, University of Chicago <br>
Copyright (c) 2006, Contributors <br>
All rights reserved. <br>
See ../../../Copyright.txt for details. <br>
*/

#include "ReachDefsOverwriteStandard.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace ReachDefsOverwrite {

    ReachDefsOverwriteStandard::ReachDefsOverwriteStandard(ProcHandle p) : ReachDefs::ReachDefsStandard(p) {};

    void ReachDefsOverwriteStandard::output(IRHandlesIRInterface& ir)
    {    
      ReachDefs::ReachDefsStandard::output(ir);
      sOutBuild->objStart("ReachDefsOverwriteStandard");
      sOutBuild->mapStart("mOverwrittenBy", "StmtHandle", "OA_ptr<std::set<StmtHandle> >");
      std::map<StmtHandle,OA_ptr<std::set<StmtHandle> > >::iterator mapIter;
      for (mapIter = mOverwrittenBy.begin(); mapIter != mOverwrittenBy.end(); mapIter++) {
	StmtHandle s = mapIter->first;
	OA_ptr<std::set<StmtHandle> > rdset = mapIter->second;
	if ( rdset.ptrEqual(0) ) continue;
	sOutBuild->mapEntryStart();
	sOutBuild->mapKeyStart();
	sOutBuild->outputIRHandle(s, ir);
	sOutBuild->mapKeyEnd();
	sOutBuild->mapValueStart();
	sOutBuild->listStart();
	std::set<StmtHandle>::iterator setIter;
	for (setIter=rdset->begin(); setIter!=rdset->end(); setIter++) {
	  sOutBuild->listItemStart();
	  sOutBuild->outputIRHandle(*setIter, ir);
	  sOutBuild->listItemEnd();
	}
	sOutBuild->listEnd();
	sOutBuild->mapValueEnd();
	sOutBuild->mapEntryEnd();
      }
      sOutBuild->mapEnd("mOverwrittenBy");
      sOutBuild->objEnd("ReachDefsOverwriteStandard");
    }

    OA_ptr<ReachDefs::Interface::ReachDefsIterator> ReachDefsOverwriteStandard::getOverwritingStmts(StmtHandle s){ 
      OA_ptr<ReachDefs::Interface::ReachDefsIterator> retval;
      if (mOverwrittenBy[s].ptrEqual(0)) {
	mOverwrittenBy[s] = new std::set<StmtHandle>;
      }
      retval = new ReachDefs::ReachDefsStandard::ReachDefsStandardReachDefsIterator(mOverwrittenBy[s]);
      return retval;
    }

    void ReachDefsOverwriteStandard::insertOverwrittenBy(StmtHandle def, 
							 StmtHandle ovw) { 
      if (mOverwrittenBy[def].ptrEqual(NULL)) {
	mOverwrittenBy[def] = new std::set<StmtHandle>;
      }
      mOverwrittenBy[def]->insert(ovw);
    }

  } 
} 

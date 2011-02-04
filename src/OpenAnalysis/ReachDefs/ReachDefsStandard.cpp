/*! \file
  
  \brief Implementation of ReachDefs::ReachDefsStandard

  \author Michelle Strout
  \version $Id: ReachDefsStandard.cpp,v 1.8 2004/12/21 21:15:19 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ReachDefsStandard.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace ReachDefs {

//! Return an iterator that will iterate over the set of reaching
//! defs for the stmt
OA_ptr<Interface::ReachDefsIterator> 
ReachDefsStandard::getReachDefsIterator(StmtHandle s) 
{ 
    OA_ptr<ReachDefsStandardReachDefsIterator> retval;

    if (mReachDefs[s].ptrEqual(0)) {
        mReachDefs[s] = new std::set<StmtHandle>;
    }
    retval = new ReachDefsStandardReachDefsIterator(mReachDefs[s]);
    return retval;
}

//! Return an iterator over reaching definitions that reach the exit node
//! in the procedure
OA_ptr<Interface::ReachDefsIterator> 
ReachDefsStandard::getExitReachDefsIterator() 
{ 
    OA_ptr<ReachDefsStandardReachDefsIterator> retval;
    retval = new ReachDefsStandardReachDefsIterator(mExitReachDefs);
    return retval;
}

void ReachDefsStandard::output(IRHandlesIRInterface& ir) const
{    
    sOutBuild->objStart("ReachDefsStandard");

    sOutBuild->mapStart("mReachDefs", "StmtHandle", "OA_ptr<std::set<StmtHandle> >");
    std::map<StmtHandle,OA_ptr<std::set<StmtHandle> > >::const_iterator mapIter;
    for (mapIter = mReachDefs.begin(); mapIter != mReachDefs.end(); mapIter++) {
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
    sOutBuild->mapEnd("mReachDefs");

    sOutBuild->fieldStart("mExitReachDefs");
    sOutBuild->listStart();
    std::set<StmtHandle>::iterator setIter;
    for (setIter=mExitReachDefs->begin(); 
         setIter!=mExitReachDefs->end(); setIter++) 
    {
        sOutBuild->listItemStart();
        sOutBuild->outputIRHandle(*setIter, ir);
        sOutBuild->listItemEnd();
    }
    sOutBuild->listEnd();
    sOutBuild->fieldEnd("mExitReachDefs");
    sOutBuild->objEnd("ReachDefsStandard");
}



//! incomplete output of info for debugging, just lists stmts
//! and associated set of stmts that are reaching defs for the given
void ReachDefsStandard::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    std::map<StmtHandle,OA_ptr<std::set<StmtHandle> > >::iterator mapIter;
    for (mapIter = mReachDefs.begin(); mapIter != mReachDefs.end(); mapIter++) {
        StmtHandle s = mapIter->first;

        os << "StmtHandle(" << s.hval() << ") " << ir->toString(s) 
           << std::endl << "\treachdefs: " << std::endl;
        // iterate over reaching defs for statement s
        OA_ptr<Interface::ReachDefsIterator> setIterPtr
            = getReachDefsIterator(s);
        for (; setIterPtr->isValid(); (*setIterPtr)++) {
            os << "\t" << ir->toString(setIterPtr->current()) << std::endl;
        }

    }
}



  } // end of ReachDefs namespace
} // end of OA namespace

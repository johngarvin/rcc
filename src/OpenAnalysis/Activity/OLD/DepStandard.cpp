/*! \file
  
  \brief Implementation of DepStandard

  \author Michelle Strout
  \version $Id: DepStandard.cpp,v 1.8 2005/06/10 02:32:02 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "DepStandard.hpp"
#include <Utils/Util.hpp>

namespace OA {
  namespace Activity {

static bool debug = false;


//*****************************************************************
// Interface Implementation
//*****************************************************************
    
/*!
   Return an iterator over all locations whose definition may 
   depend on the given use location.
*/
OA_ptr<LocIterator> 
DepStandard::getMayDefIterator(StmtHandle stmt, const OA_ptr<Location> use)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_DepStandard:ALL", debug);
    if (mDepDFSet[stmt].ptrEqual(0)) {
        mDepDFSet[stmt] = new DepDFSet;
    }
    return mDepDFSet[stmt]->getDefsIterator(use);
}

/*!
   Return an iterator over all locations that are differentiable
   locations used in the possible definition of the given location

   For now assuming that all defs depend on all uses.
*/
OA_ptr<LocIterator> 
DepStandard::getDiffUseIterator(StmtHandle stmt, OA_ptr<Location> def)
{
    if (mDepDFSet[stmt].ptrEqual(0)) {
        mDepDFSet[stmt] = new DepDFSet;
    }
    if (debug) {
        mDepDFSet[stmt]->dump(std::cout);
    }
    return mDepDFSet[stmt]->getUsesIterator(def);
}
    
//! Return an iterator over all locations that are definitely
//! defined in the given stmt
OA_ptr<LocIterator> DepStandard::getMustDefIterator(StmtHandle stmt)
{
  OA_ptr<LocSetIterator> retval;
  if (mMustDefMap[stmt].ptrEqual(0)) {
    OA_ptr<LocSet> emptySet; emptySet = new LocSet;
    retval = new LocSetIterator(emptySet);  
  } else {
    retval = new LocSetIterator(mMustDefMap[stmt]); 
  }
  return retval;
}


//*****************************************************************
// Construction methods
//*****************************************************************

//! Insert use,def dependence pair
void DepStandard::insertDepForStmt(StmtHandle stmt, 
                                   OA_ptr<Location> use,
                                   OA_ptr<Location> def)
{
    assert(!use.ptrEqual(0));
    assert(!def.ptrEqual(0));

    // first make sure there is a DepDFSet for the given stmt
    if (mDepDFSet[stmt].ptrEqual(0)) {
        mDepDFSet[stmt] = new DepDFSet;
    }

    // then insert the dependence
    mDepDFSet[stmt]->insertDep(use,def);
}

//! Insert must def location 
void DepStandard::insertMustDefForStmt(StmtHandle stmt, 
                                       OA_ptr<Location> def)
{
    if (mMustDefMap[stmt].ptrEqual(0)) {
        mMustDefMap[stmt] = new LocSet;
    }
    mMustDefMap[stmt]->insert(def);
 }
 
//*****************************************************************
// Output
//*****************************************************************
void DepStandard::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    std::cout << "====================== Dep" << std::endl;

    OA_ptr<LocIterator> locIterPtr;
    std::map<StmtHandle,OA_ptr<LocSet> >::iterator mapIter;

    std::cout << "MustDefMap = " << std::endl;
    for (mapIter=mMustDefMap.begin(); mapIter!=mMustDefMap.end(); mapIter++) {
        std::cout << "\tstmt = " << ir->toString(mapIter->first) << std::endl;
        locIterPtr = getMustDefIterator(mapIter->first);
        for ( ; locIterPtr->isValid(); (*locIterPtr)++ ) {
            std::cout << "\t\t";
            locIterPtr->current()->dump(std::cout,ir);
            std::cout << std::endl;
        }
    }
    std::cout << "DepDFSets = " << std::endl;
    std::map<StmtHandle,OA_ptr<DepDFSet> >::iterator mapIter2;
    for (mapIter2=mDepDFSet.begin(); mapIter2!=mDepDFSet.end(); mapIter2++) {
        std::cout << "\tstmt = " << ir->toString(mapIter2->first) << std::endl;
        if (!mDepDFSet[mapIter2->first].ptrEqual(NULL)) {
            OA_ptr<DepDFSet> depDFSet = mapIter2->second;
            depDFSet->dump(os,ir);
        }
    }

    std::cout << std::endl;
}

  } // end of Activity namespace
} // end of OA namespace

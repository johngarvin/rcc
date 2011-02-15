/*! \file
  
  \brief Implementation of ICFGDep

  \author Michelle Strout
  \version $Id: ICFGDep.cpp,v 1.2 2005/06/10 02:32:02 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ICFGDep.hpp"
#include <Utils/Util.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

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
OA_ptr<DataFlow::DataFlowSetImplIterator<int> >
ICFGDep::getMayDefIterator(StmtHandle stmt, int use)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ICFGDep:ALL", debug);
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
OA_ptr<DataFlow::DataFlowSetImplIterator<int> >
ICFGDep::getDiffUseIterator(StmtHandle stmt, int def)
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

OA_ptr<DataFlow::DataFlowSetImplIterator<int> > 
ICFGDep::getMustDefIterator(StmtHandle stmt)
{
  //OA_ptr<LocSetIterator> retval;
  OA_ptr<DataFlow::DataFlowSetImplIterator<int> > retval;
  if (mMustDefMap[stmt].empty()) {
    //OA_ptr<LocSet> emptySet; emptySet = new LocSet;
    //retval = new LocSetIterator(emptySet);  
    OA_ptr<DataFlow::DataFlowSetImpl<int> > dSet;
    dSet = new DataFlow::DataFlowSetImpl<int>();
    retval = new DataFlow::DataFlowSetImplIterator<int>(dSet);
  } else {
    //retval = new LocSetIterator(mMustDefMap[stmt]); 
    OA_ptr<DataFlow::DataFlowSetImpl<int> > dSet;
    dSet = new DataFlow::DataFlowSetImpl<int>(mMustDefMap[stmt]);
    retval = new DataFlow::DataFlowSetImplIterator<int>(dSet);
  }
  return retval;
}


//*****************************************************************
// Construction methods
//*****************************************************************

//! Insert use,def dependence pair
void ICFGDep::insertDepForStmt(StmtHandle stmt, int use, int def)
{
    //assert(!use.ptrEqual(0));
    //assert(!def.ptrEqual(0));

    // first make sure there is a DepDFSet for the given stmt
    if (mDepDFSet[stmt].ptrEqual(0)) {
        mDepDFSet[stmt] = new DepDFSet;
    }

    // then insert the dependence
    mDepDFSet[stmt]->insertDep(use,def);
}

//! Insert must def location 
void ICFGDep::insertMustDefForStmt(StmtHandle stmt, int def)
{
    //if (mMustDefMap[stmt].empty()) {
    //    mMustDefMap[stmt] = new std::set<int>;
    //}
    mMustDefMap[stmt].insert(def);
 }
 
//*****************************************************************
// Output
//*****************************************************************
void ICFGDep::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    /*
    std::cout << "====================== Dep" << std::endl;

    OA_ptr<AliasTagIteratorImpl> locIterPtr;
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
    */
    
}

void ICFGDep::output(OA::IRHandlesIRInterface& ir)
{
  // output MustDefMap
  sOutBuild->mapStart("MustDefMap", "StmtHandle", "LocSet");

  std::map<StmtHandle,std::set<int> >::iterator mapIter;

  OA_ptr<DataFlow::DataFlowSetImplIterator<int> > dsetIterPtr;

  for (mapIter=mMustDefMap.begin(); mapIter!=mMustDefMap.end(); mapIter++) {

    StmtHandle stmt = mapIter->first;
    if (stmt == StmtHandle(0)) { continue; }

    sOutBuild->mapEntryStart();
    sOutBuild->mapKey(ir.toString(stmt));
    sOutBuild->mapValueStart();

    dsetIterPtr = getMustDefIterator(mapIter->first);
    if (dsetIterPtr->isValid()) {
 
      sOutBuild->listStart();
      for ( ; dsetIterPtr->isValid(); (*dsetIterPtr)++ ) {
        sOutBuild->listItemStart();
        
        int tag;
        tag = dsetIterPtr->current();
        sOutBuild->mapKey(OA::int2string(tag));

        sOutBuild->listItemEnd();

      }
      sOutBuild->listEnd();

    }
    // removing a level of indent manually to accommodate a map Value
    // that has a list of objects
    ostringstream correct;
    correct << popIndt;
    sOutBuild->outputString( correct.str() );

    sOutBuild->mapValueEnd();

  }

  sOutBuild->mapEnd("MustDefMap");


  // output DepDFSets
  sOutBuild->mapStart("mDepDFSet", "StmtHandle", "DepDFSet");
  
  std::map<StmtHandle,OA_ptr<DepDFSet> >::iterator mapIter2;

  for (mapIter2=mDepDFSet.begin(); mapIter2!=mDepDFSet.end(); mapIter2++) {
    StmtHandle stmt = mapIter2->first;
    if (stmt == StmtHandle(0)) { continue; }

    sOutBuild->mapEntryStart();
    sOutBuild->mapKeyStart();

    ostringstream stmtlabel;
    stmtlabel << "stmt: " << ir.toString(stmt);
    sOutBuild->outputString( stmtlabel.str() );
    
    sOutBuild->mapKeyEnd();
    sOutBuild->mapValueStart();

    ostringstream indentMapValue;
    indentMapValue << pushIndt;
    sOutBuild->outputString( indentMapValue.str() );

    OA_ptr<DepDFSet> depDFSet = mapIter2->second;
    if (!depDFSet.ptrEqual(NULL)) {
      depDFSet->output(ir); 
    }
    // removing two levels of indent manually to accommodate a map Value
    // that has a list of objects manually indented
    ostringstream correct;
    correct << popIndt << popIndt;
    sOutBuild->outputString( correct.str() );

    sOutBuild->mapValueEnd();

  }
  sOutBuild->mapEnd("DepDFSets");
}

  } // end of Activity namespace
} // end of OA namespace

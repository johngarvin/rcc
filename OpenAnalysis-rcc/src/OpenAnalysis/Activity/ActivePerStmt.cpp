/*! \file
  
  \brief Implementation of Activity::ActivePerStmt

  \author Michelle Strout
  \version $Id: InActivePerStmt.cpp,v 1.1 2005/07/01 02:49:56 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <OpenAnalysis/Activity/ActivePerStmt.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace Activity {

ActivePerStmt::ActivePerStmt() 
{ 
  mNumIter = 0;
}


OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > 
ActivePerStmt::getInActiveSet(StmtHandle stmt)
{
    if (mInActive.find(stmt)!=mInActive.end()) {
        return mInActive[stmt];
    } else {
        OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > emptySet; 
        emptySet = new DataFlow::DFSetBitImpl<Alias::AliasTag>(1);
        return emptySet;
    }
}

OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > 
ActivePerStmt::getOutActiveSet(StmtHandle stmt)
{
    if (mOutActive.find(stmt)!=mOutActive.end()) {
        return mOutActive[stmt];
    } else {
        OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > emptySet; 
        emptySet = new DataFlow::DFSetBitImpl<Alias::AliasTag>(1);
        return emptySet;
    }
}


OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> >
ActivePerStmt::getInActiveSet(CallHandle call)
{
    if (mCallInActive.find(call)!=mCallInActive.end()) {
        return mCallInActive[call];
    } else {
        OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > emptySet;
        emptySet = new DataFlow::DFSetBitImpl<Alias::AliasTag>(1);
        return emptySet;
    }
}

OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> >
ActivePerStmt::getOutActiveSet(CallHandle call)
{
    if (mCallOutActive.find(call)!=mCallOutActive.end()) {
        return mCallOutActive[call];
    } else {
        OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > emptySet;
        emptySet = new DataFlow::DFSetBitImpl<Alias::AliasTag>(1);
        return emptySet;
    }
}

//*****************************************************************
// Construction methods 
//*****************************************************************

void ActivePerStmt::copyIntoInActive(StmtHandle s, 
                                 OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > dfset)
{
    OA_ptr<DataFlow::DataFlowSet> cloneDFset = dfset->clone();
    mInActive[s] = cloneDFset.convert<DataFlow::DFSetBitImpl<Alias::AliasTag> >();
}

void ActivePerStmt::copyIntoOutActive(StmtHandle s, 
                                  OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > dfset)
{
    OA_ptr<DataFlow::DataFlowSet> cloneDFset = dfset->clone();
    mOutActive[s] = cloneDFset.convert<DataFlow::DFSetBitImpl<Alias::AliasTag> >();
}

void ActivePerStmt::copyIntoInActive(CallHandle c,
                                 OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > dfset)
{
    OA_ptr<DataFlow::DataFlowSet> cloneDFset = dfset->clone();
    mCallInActive[c] = cloneDFset.convert<DataFlow::DFSetBitImpl<Alias::AliasTag> >();
}

void ActivePerStmt::copyIntoOutActive(CallHandle c,
                                  OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > dfset)
{
    OA_ptr<DataFlow::DataFlowSet> cloneDFset = dfset->clone();
    mCallOutActive[c] = cloneDFset.convert<DataFlow::DFSetBitImpl<Alias::AliasTag> >();
}


//*****************************************************************
// Annotation Interface
//*****************************************************************
void ActivePerStmt::output(IRHandlesIRInterface &ir) const
{
  sOutBuild->mapStart("ActiveSets","StmtHandle","ActiveLocDFSet"); {
  std::map<StmtHandle, OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > >::const_iterator mapIter;
    for (mapIter = mInActive.begin(); mapIter != mInActive.end(); mapIter++) {
      StmtHandle stmt = mapIter->first;
      OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > inActiveSet = mapIter->second;
      const OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > outActiveSet =
        mOutActive.find(stmt)->second;
      sOutBuild->mapEntryStart(); {
        sOutBuild->mapKeyStart(); {
          sOutBuild->outputIRHandle(stmt,ir);
        } sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart(); {
          sOutBuild->objStart("InActiveSet"); {
            inActiveSet->output(ir);
          } sOutBuild->objEnd("InActiveSet");
          sOutBuild->objStart("OutActiveSet"); {
            if(!outActiveSet.ptrEqual(0)) {
                outActiveSet->output(ir);
            }
          } sOutBuild->objEnd("OutActiveSet");
        } sOutBuild->mapValueEnd();
      } sOutBuild->mapEntryEnd();
    }
  } sOutBuild->mapEnd("ActiveSets");
}

//*****************************************************************
// Output
//*****************************************************************

//! incomplete output of info for debugging
void ActivePerStmt::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    os << "============================ ActivePerStmt" << std::endl;

    // iterate over all stmts we know about
    std::map<StmtHandle, 
             OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > >::iterator mapIter;
    for (mapIter = mInActive.begin(); mapIter != mInActive.end(); mapIter++) {
        StmtHandle s = mapIter->first;
        OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > inActSet = mapIter->second;
        OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > outActSet = mOutActive[s];
        if (outActSet.ptrEqual(0)) {
          outActSet = new DataFlow::DFSetBitImpl<Alias::AliasTag>(1);
        }

        os << "StmtHandle(" << ir->toString(s) << ")" 
           << std::endl << "\tInActive: " << std::endl;
        inActSet->dump(os,ir);
        os << "\tOutActive: " << std::endl;
        outActSet->dump(os,ir);
    }
}



  } // end of Activity namespace
} // end of OA namespace

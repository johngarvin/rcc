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

#include "ActivePerStmt.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace Activity {

ActivePerStmt::ActivePerStmt() 
{ 
  mNumIter = 0;
}


OA_ptr<DataFlow::DataFlowSetImpl<int> > 
ActivePerStmt::getInActiveSet(StmtHandle stmt)
{
    if (mInActive.find(stmt)!=mInActive.end()) {
        return mInActive[stmt];
    } else {
        OA_ptr<DataFlow::DataFlowSetImpl<int> > emptySet; 
        emptySet = new DataFlow::DataFlowSetImpl<int> ;
        return emptySet;
    }
}

OA_ptr<DataFlow::DataFlowSetImpl<int> > 
ActivePerStmt::getOutActiveSet(StmtHandle stmt)
{
    if (mOutActive.find(stmt)!=mOutActive.end()) {
        return mOutActive[stmt];
    } else {
        OA_ptr<DataFlow::DataFlowSetImpl<int> > emptySet; 
        emptySet = new DataFlow::DataFlowSetImpl<int> ;
        return emptySet;
    }
}

//*****************************************************************
// Construction methods 
//*****************************************************************

void ActivePerStmt::copyIntoInActive(StmtHandle s, 
                                 OA_ptr<DataFlow::DataFlowSetImpl<int> > dfset)
{
    OA_ptr<DataFlow::DataFlowSet> cloneDFset = dfset->clone();
    mInActive[s] = cloneDFset.convert<DataFlow::DataFlowSetImpl<int> >();
}

void ActivePerStmt::copyIntoOutActive(StmtHandle s, 
                                  OA_ptr<DataFlow::DataFlowSetImpl<int> > dfset)
{
    OA_ptr<DataFlow::DataFlowSet> cloneDFset = dfset->clone();
    mOutActive[s] = cloneDFset.convert<DataFlow::DataFlowSetImpl<int> >();
}


//*****************************************************************
// Annotation Interface
//*****************************************************************
void ActivePerStmt::output(IRHandlesIRInterface &ir)
{
  sOutBuild->mapStart("ActiveSets","StmtHandle","ActiveLocDFSet"); {
  std::map<StmtHandle, OA_ptr<DataFlow::DataFlowSetImpl<int> > >::iterator mapIter;
    for (mapIter = mInActive.begin(); mapIter != mInActive.end(); mapIter++) {
      StmtHandle stmt = mapIter->first;
      OA_ptr<DataFlow::DataFlowSetImpl<int> > inActiveSet = mapIter->second;
      OA_ptr<DataFlow::DataFlowSetImpl<int> > outActiveSet = mOutActive[stmt];
      if (outActiveSet.ptrEqual(0)) { 
        outActiveSet = new DataFlow::DataFlowSetImpl<int>();
      }
      sOutBuild->mapEntryStart(); {
        sOutBuild->mapKeyStart(); {
          sOutBuild->outputIRHandle(stmt,ir);
        } sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart(); {
          sOutBuild->objStart("InActiveSet"); {
            inActiveSet->output(ir);
          } sOutBuild->objEnd("InActiveSet");
          sOutBuild->objStart("OutActiveSet"); {
            outActiveSet->output(ir);
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
             OA_ptr<DataFlow::DataFlowSetImpl<int> > >::iterator mapIter;
    for (mapIter = mInActive.begin(); mapIter != mInActive.end(); mapIter++) {
        StmtHandle s = mapIter->first;
        OA_ptr<DataFlow::DataFlowSetImpl<int> > inActSet = mapIter->second;
        OA_ptr<DataFlow::DataFlowSetImpl<int> > outActSet = mOutActive[s];
        if (outActSet.ptrEqual(0)) {
          outActSet = new DataFlow::DataFlowSetImpl<int>();
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

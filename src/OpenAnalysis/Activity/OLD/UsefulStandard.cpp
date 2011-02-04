/*! \file
  
  \brief Implementation of Activity::UsefulStandard

  \author Michelle Strout
  \version $Id: UsefulStandard.cpp,v 1.6 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <OpenAnalysis/Activity/UsefulStandard.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace Activity {

UsefulStandard::UsefulStandard(ProcHandle p)
{ 
    mDepTagSet = new DataFlow::DataFlowSetImpl<int>(); 
    //mFinalInUseful = new DataFlow::LocDFSet;
    mNumIter = 0;
}

OA_ptr<DataFlow::DataFlowSetImplIterator<int> >
 UsefulStandard::getDepSetIterator() 
{ 
    OA_ptr<DataFlow::DataFlowSetImplIterator<int> > retval;
    retval = new DataFlow::DataFlowSetImplIterator<int>(mDepTagSet);
    return retval;
}

OA_ptr<DataFlow::DataFlowSetImplIterator<int> >
 UsefulStandard::getInUsefulIterator(StmtHandle s)
{
    OA_ptr<DataFlow::DataFlowSetImplIterator<int> > retval;
    if (mInUseful[s].ptrEqual(0)) {
        mInUseful[s] = new DataFlow::DataFlowSetImpl<int>();
    }
    retval = new DataFlow::DataFlowSetImplIterator<int>(mInUseful[s]);
    return retval;
}

OA_ptr<DataFlow::DataFlowSetImplIterator<int> >
 UsefulStandard::getOutUsefulIterator(StmtHandle s)
{
    OA_ptr<DataFlow::DataFlowSetImplIterator<int> > retval;
    if (mOutUseful[s].ptrEqual(0)) {
        mOutUseful[s] = new DataFlow::DataFlowSetImpl<int>();
    }
    retval = new DataFlow::DataFlowSetImplIterator<int>(mOutUseful[s]);
    return retval;
}

//OA_ptr<DataFlow::LocDFSet> UsefulStandard::getFinalUseful()
//{
//    return mFinalInUseful;
//}

  //void UsefulStandard::mapFinalUseful(OA_ptr<DataFlow::LocDFSet> final)
  //{
  //    mFinalInUseful = final;
  //}

//*****************************************************************
// Output 
//*****************************************************************

//! incomplete output of info for debugging
void UsefulStandard::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    os << "=================================== UsefulStandard" << std::endl;
    os << "DepSet = " << std::endl;
    mDepTagSet->dump(os,ir);

    // iterate over all stmts we know about
    std::map<StmtHandle, 
             OA_ptr<DataFlow::DataFlowSetImpl<int> > >::iterator mapIter;
    for (mapIter = mInUseful.begin(); mapIter != mInUseful.end(); mapIter++) {
        StmtHandle s = mapIter->first;

        os << "StmtHandle(" << s.hval() << ") " << ir->toString(s) 
           << std::endl << "\tInUseful: " << std::endl;
        mapIter->second->dump(os,ir);
    }
}

//*****************************************************************
// Annotation Interface
//*****************************************************************
void UsefulStandard::output(IRHandlesIRInterface &ir)
{
  sOutBuild->objStart("mNumIter"); {
    ostringstream oss;
    oss << mNumIter;
    sOutBuild->outputString( oss.str() );
  } sOutBuild->objEnd("mNumIter");

  sOutBuild->objStart("mDepTagSet"); {
    mDepTagSet->output(ir);
  } sOutBuild->objEnd("mTagLocSet");

  sOutBuild->mapStart("mStmtToUsefulSetsMap","StmtHandle","UsefulSets"); {
    std::map<StmtHandle,
      OA_ptr<DataFlow::DataFlowSetImpl<int> > >::iterator mapIter;
    for (mapIter = mInUseful.begin(); mapIter != mInUseful.end(); mapIter++) {
      StmtHandle stmt = mapIter->first;
      OA_ptr<DataFlow::DataFlowSetImpl<int> > inUsefulSet = mapIter->second;
      OA_ptr<DataFlow::DataFlowSetImpl<int> > outUsefulSet = mOutUseful[stmt];
      // should have a outUseful for every inUseful, but just in case ...
      if (outUsefulSet.ptrEqual(0)) { 
        outUsefulSet = new DataFlow::DataFlowSetImpl<int>(); 
      }

      sOutBuild->mapEntryStart(); {
        sOutBuild->mapKeyStart(); {
          sOutBuild->outputIRHandle(stmt,ir);
        } sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart(); {
          sOutBuild->fieldStart("InUsefulSet"); {
            inUsefulSet->output(ir);
          } sOutBuild->fieldEnd("InUsefulSet");
          sOutBuild->fieldStart("OutUsefulSet"); {
            outUsefulSet->output(ir);
          } sOutBuild->fieldEnd("OutUsefulSet");
        } sOutBuild->mapValueEnd();
      } sOutBuild->mapEntryEnd();
    }
  } sOutBuild->mapEnd("mStmtToInUsefulSetMap");
}


  } // end of Activity namespace
} // end of OA namespace

/*! \file
  
  \brief Definition for InterReachConsts

  \authors Michelle Strout, Barbara Kreaseck
  \version $Id: InterReachConsts.cpp,v 1.2 2005/06/10 02:32:04 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "InterReachConsts.hpp"

namespace OA {
  namespace ReachConsts {

//! Returns ReachConsts results for the given procedure
OA_ptr<ReachConsts::Interface> InterReachConsts::getReachConsts(ProcHandle proc)
      { if (mProcToReachConstsMap[proc].ptrEqual(0)) {
          OA_ptr<ReachConsts::Interface> empty;
          empty = new ReachConsts::ReachConstsStandard(proc);
          mProcToReachConstsMap[proc] = empty;
        }
        return mProcToReachConstsMap[proc];
      }


//*****************************************************************
// Annotation Interface
//*****************************************************************
void InterReachConsts::output(OA::IRHandlesIRInterface& ir) {

  sOutBuild->mapStart("mProcToReachConstsMap","ProcHandle","ReachConsts"); {  
    std::map<ProcHandle,OA_ptr<ReachConsts::Interface> >::iterator mapIter;
    for (mapIter=mProcToReachConstsMap.begin();
         mapIter!=mProcToReachConstsMap.end();
         mapIter++) 
      {
        ProcHandle proc = mapIter->first;
        OA_ptr<ReachConsts::Interface> rc = mapIter->second;

        sOutBuild->mapEntryStart(); {
          sOutBuild->mapKeyStart(); {
            sOutBuild->outputIRHandle(proc,ir);
          } sOutBuild->mapKeyEnd();
          sOutBuild->mapValueStart(); {
            sOutBuild->fieldStart("ReachConstStandard"); {
            rc->output(ir);
            } sOutBuild->fieldEnd("ReachConstStandard");
          } sOutBuild->mapValueEnd();
        } sOutBuild->mapEntryEnd();
      }
  } sOutBuild->mapEnd("mProcToReachConstsMap");

}


//*****************************************************************
// Output
//*****************************************************************
//! debugging output
void InterReachConsts::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
        os << "====================== InterDep" << std::endl;
        std::map<ProcHandle,OA_ptr<ReachConsts::Interface> >::iterator mapIter;
        for (mapIter=mProcToReachConstsMap.begin();
             mapIter!=mProcToReachConstsMap.end();
             mapIter++) 
        {
            os << "----------------------- proc results ----" << std::endl;
            OA_ptr<ReachConsts::Interface> results = mapIter->second;
            results->dump(os,ir);
            os << "-------------------------------" << std::endl;
        }


}

  } // end of ReachConsts namespace
} // end of OA namespace



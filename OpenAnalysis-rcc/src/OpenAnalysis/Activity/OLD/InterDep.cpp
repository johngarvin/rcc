/*! \file
  
  \brief Definition for InterDep

  \authors Michelle Strout
  \version $Id: InterDep.cpp,v 1.2 2005/06/10 02:32:02 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "InterDep.hpp"

namespace OA {
  namespace Activity {

//! Returns dep results for the given procedure
OA_ptr<Activity::DepStandard> InterDep::getDepResults(ProcHandle proc)
      { if (mProcToDepMap[proc].ptrEqual(0)) {
          OA_ptr<DepStandard> empty;
          empty = new DepStandard();
          mProcToDepMap[proc] = empty;
        }
        return mProcToDepMap[proc];
      }

//! Returns a dep set for given proc call
//! FIXME?: only used in ManagerDep, general client
//! doesn't need this because will have Dep results per stmt in proc
//! ManagerDep needs it by call because results for each procedure
//! have been mapped to locations in caller
OA_ptr<DepDFSet> InterDep::getDepForCall(ExprHandle call)
      {
        if (mCallToDepDFSet[call].ptrEqual(0)) {
          mCallToDepDFSet[call] = new DepDFSet;
          OA_ptr<Location> unknown; unknown = new UnknownLoc;
          mCallToDepDFSet[call]->insertDep(unknown,unknown);
        }
        return mCallToDepDFSet[call];
      }

//*****************************************************************
// Output
//*****************************************************************
//! debugging output
void InterDep::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
      {
        os << "====================== InterDep" << std::endl;
        std::map<ProcHandle,OA_ptr<DepStandard> >::iterator mapIter;
        for (mapIter=mProcToDepMap.begin();
             mapIter!=mProcToDepMap.end();
             mapIter++) 
        {
            os << "----------------------- proc results ----" << std::endl;
            OA_ptr<DepStandard> results = mapIter->second;
            results->dump(os,ir);
            os << "-------------------------------" << std::endl;
        }

        std::map<ExprHandle,OA_ptr<DepDFSet> >::iterator callIter;
        for (callIter=mCallToDepDFSet.begin();
             callIter!=mCallToDepDFSet.end();
             callIter++) 
        {
            os << "----------------------- call results ----" << std::endl;
            OA_ptr<DepDFSet> results = callIter->second;
            results->dump(os,ir);
            os << "-------------------------------" << std::endl;
        }

      }

  } // end of Activity namespace
} // end of OA namespace



/*! \file
  
  \brief Declarations for the VaryStandard implementation of interprocedural 
         Vary results.

  \authors Michelle Strout
  \version $Id: InterVary.hpp,v 1.3 2005/07/01 02:49:56 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef InterVary_H
#define InterVary_H

#include <OpenAnalysis/Activity/VaryStandard.hpp>
#include <map>

namespace OA {
  namespace Activity {

class InterVary {
  public:
    InterVary() { mNumIter = 0;}
    virtual ~InterVary() {}

    //! Returns vary results for the given procedure
    OA_ptr<Activity::VaryStandard> getVaryResults(ProcHandle proc)
      { if (mProcToVaryMap[proc].ptrEqual(0)) {
          OA_ptr<VaryStandard> empty;
          empty = new VaryStandard(proc);
          mProcToVaryMap[proc] = empty;
        }
        return mProcToVaryMap[proc];
      }

    //! Returns number of iterations over all nodes
    int getNumIter() { return mNumIter; }

    //*****************************************************************
    // Output
    //*****************************************************************
    //! debugging output
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
      {
        os << "====================== InterVary" << std::endl;
        std::map<ProcHandle,OA_ptr<VaryStandard> >::iterator mapIter;
        for (mapIter=mProcToVaryMap.begin();
             mapIter!=mProcToVaryMap.end();
             mapIter++) 
        {
            OA_ptr<VaryStandard> results = mapIter->second;
            results->dump(os,ir);
            os << "-------------------------------" << std::endl;
        }
        os << "\nInterVary Number of Iterations = " << mNumIter
           << "\n\n====================== End InterVary\n\n";
      }

    
    //*****************************************************************
    // Construction methods 
    //*****************************************************************

    //! attach VaryStandard to procedure
    void mapProcToVary(ProcHandle proc, OA_ptr<VaryStandard> vary)
      { mProcToVaryMap[proc] = vary; }

    //! set number of iterations over all nodes
    void setNumIter(int n) { mNumIter = n; }

  private:
    std::map<ProcHandle,OA_ptr<VaryStandard> > mProcToVaryMap;
    int mNumIter;

};

  } // end of Activity namespace
} // end of OA namespace

#endif


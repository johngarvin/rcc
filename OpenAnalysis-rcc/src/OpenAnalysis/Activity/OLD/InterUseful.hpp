/*! \file
  
  \brief Declarations for the UsefulStandard implementation of interprocedural 
         Vary results.

  \authors Michelle Strout
  \version $Id: InterUseful.hpp,v 1.2 2005/06/10 02:32:02 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef InterUseful_H
#define InterUseful_H

#include <OpenAnalysis/Activity/UsefulStandard.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <map>

namespace OA {
  namespace Activity {

    class InterUseful : public virtual Annotation {
  public:
    InterUseful() { mNumIter = 0; }
    virtual ~InterUseful() {}

    //! Returns useful results for the given procedure
    OA_ptr<Activity::UsefulStandard> getUsefulResults(ProcHandle proc)
      { if (mProcToUsefulMap[proc].ptrEqual(0)) {
          OA_ptr<UsefulStandard> empty;
          empty = new UsefulStandard(proc);
          mProcToUsefulMap[proc] = empty;
        }
        return mProcToUsefulMap[proc];
      }
  
    //! Returns number of iterations over all node
    int getNumIter() { return mNumIter; }

    //*****************************************************************
    // Output
    //*****************************************************************
    //! debugging output
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
      {
        os << "====================== InterUseful" << std::endl;
        std::map<ProcHandle,OA_ptr<UsefulStandard> >::iterator mapIter;
        for (mapIter=mProcToUsefulMap.begin();
             mapIter!=mProcToUsefulMap.end();
             mapIter++) 
        {
            OA_ptr<UsefulStandard> results = mapIter->second;
            results->dump(os,ir);
            os << "-------------------------------" << std::endl;
        }
        os << "\nInterUseful Number of Iterations = " << mNumIter
           << "\n\n====================== End InterUseful\n\n";
      }

    

    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface &ir) {

      sOutBuild->objStart("InterUseful"); {
        sOutBuild->mapStart("mProcToUsefulMap","ProcHandle","UsefulStandard");
        std::map<ProcHandle,OA_ptr<UsefulStandard> >::iterator mapIter;
        for (mapIter=mProcToUsefulMap.begin();
             mapIter!=mProcToUsefulMap.end();
             mapIter++)
          {
            ProcHandle proc = mapIter->first;
            OA_ptr<UsefulStandard> results = mapIter->second;

            sOutBuild->mapEntryStart(); {
              sOutBuild->mapKeyStart(); {
                sOutBuild->outputIRHandle(proc,ir);
              } sOutBuild->mapKeyEnd();
              sOutBuild->mapValueStart(); {
                results->output(ir);
              } sOutBuild->mapValueEnd();
            } sOutBuild->mapEntryEnd();
          }
        sOutBuild->mapEnd("mProcToUsefulMap");
      } sOutBuild->objEnd("InterUseful");

    }

    //*****************************************************************
    // Construction methods 
    //*****************************************************************

    //! attach UsefulStandard to procedure
    void mapProcToUseful(ProcHandle proc, OA_ptr<UsefulStandard> useful)
      { mProcToUsefulMap[proc] = useful; }

    //! set number of iterations over all nodes
    void setNumIter(int n) { mNumIter = n; }

  private:
    std::map<ProcHandle,OA_ptr<UsefulStandard> > mProcToUsefulMap;
  int mNumIter;


};

  } // end of Activity namespace
} // end of OA namespace

#endif


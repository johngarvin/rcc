/*! \file
  
  \brief Declarations for the DepStandard implementation of interprocedural 
         Dep results.

  \authors Michelle Strout
  \version $Id: InterDep.hpp,v 1.2 2005/06/10 02:32:02 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef InterDep_H
#define InterDep_H

#include <OpenAnalysis/Activity/DepStandard.hpp>
#include <map>

namespace OA {
  namespace Activity {

class InterDep {
  public:
    InterDep() {}
    virtual ~InterDep() {}

    //! Returns dep results for the given procedure
    OA_ptr<Activity::DepStandard> getDepResults(ProcHandle proc);
        
    //! Returns a dep set for given proc call
    //! FIXME?: only used in ManagerDep, general client
    //! doesn't need this because will have Dep results per stmt in proc
    //! ManagerDep needs it by call because results for each procedure
    //! have been mapped to locations in caller
    OA_ptr<DepDFSet> getDepForCall(ExprHandle call);
        
    //*****************************************************************
    // Output
    //*****************************************************************
    //! debugging output
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
   
    //*****************************************************************
    // Construction methods 
    //*****************************************************************

    //! attach DepStandard to procedure
    void mapProcToDep(ProcHandle proc, OA_ptr<DepStandard> dep)
      { mProcToDepMap[proc] = dep; }

    //! attach Deps to a function call
    void mapCallToDep(ExprHandle call, OA_ptr<DepDFSet> dep)
      { mCallToDepDFSet[call] = dep; }

  private:
    std::map<ProcHandle,OA_ptr<DepStandard> > mProcToDepMap;
    std::map<ExprHandle,OA_ptr<DepDFSet> > mCallToDepDFSet;


};

  } // end of Activity namespace
} // end of OA namespace

#endif


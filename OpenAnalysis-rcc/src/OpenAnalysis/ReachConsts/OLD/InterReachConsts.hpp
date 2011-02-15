/*! \file
  
  \brief Declarations for the ReachConsts::Interface implementation of 
         interprocedural ReachConsts results.

  \authors Michelle Strout, Barbara Kreaseck
  \version $Id: InterReachConsts.hpp,v 1.2 2005/06/10 02:32:04 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef InterReachConsts_H
#define InterReachConsts_H

#include <OpenAnalysis/ReachConsts/Interface.hpp>
#include <OpenAnalysis/ReachConsts/ReachConstsStandard.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <map>

namespace OA {
  namespace ReachConsts {

    class InterReachConsts : public virtual Annotation {
  public:
    InterReachConsts() {}
    virtual ~InterReachConsts() {}

    //! Returns ReachConsts results for the given procedure
    OA_ptr<ReachConsts::Interface> getReachConsts(ProcHandle proc);
        
    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(OA::IRHandlesIRInterface& ir);

    //*****************************************************************
    // Output
    //*****************************************************************
    //! debugging output
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
   
    //*****************************************************************
    // Construction methods 
    //*****************************************************************

    //! attach DepStandard to procedure
    void mapProcToReachConsts(ProcHandle proc, 
                              OA_ptr<ReachConsts::Interface> rcs)
      { mProcToReachConstsMap[proc] = rcs; }

    //! attach Deps to a function call
    //void mapCallToDep(ExprHandle call, OA_ptr<DepDFSet> dep)
    //  { mCallToDepDFSet[call] = dep; }

  private:
  std::map<ProcHandle,OA_ptr<ReachConsts::Interface> > 
      mProcToReachConstsMap;
  //std::map<ExprHandle,OA_ptr<DepDFSet> > mCallToDepDFSet;


};

  } // end of Activity namespace
} // end of OA namespace

#endif


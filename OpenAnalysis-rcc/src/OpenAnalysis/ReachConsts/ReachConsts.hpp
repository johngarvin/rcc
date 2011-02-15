/*! \file
  
  \brief Maps StmtHandles to set of RCPairs that reach
         each StmtHandle 

  \authors Michelle Strout, Barbara Kreaseck
  \version $Id: ReachConstsStandard.hpp,v 1.9 2005/06/10 02:32:05 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ReachConsts_hpp
#define ReachConsts_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>
#include <OpenAnalysis/ReachConsts/Interface.hpp>
#include <OpenAnalysis/IRInterface/ReachConstsIRInterface.hpp>
#include <OpenAnalysis/IRInterface/ConstValBasicInterface.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>

#include "RCPairSet.hpp"

namespace OA {
  namespace ReachConsts {


//! PLM :Questions 
//! - Do we need ReachConsts for MemRefHandles ?


class ReachConstsStandard : public virtual Interface, 
                            public virtual Annotation
{

  public:

    //! ============== Constructor/Destructor ================
    ReachConstsStandard(ProcHandle p) ;
    ~ReachConstsStandard() {}

    //! ========= ReachConsts::Interface routines ===============
    OA_ptr<ConstValBasicInterface> 
    getReachConst(StmtHandle s, OA::OA_ptr<Alias::AliasTagSet> tset);

    OA_ptr<ConstValBasicInterface> 
    getReachConst(CallHandle c, OA::OA_ptr<Alias::AliasTagSet> tset);

    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

    void output(OA::IRHandlesIRInterface& ir,
                Alias::Interface &alias) const;




    //! ======== Construction Routines ==========

  //! this RCPairSet replaces any previously stored set for this statement
  void storeRCPairSet(StmtHandle s, OA_ptr<RCPairSet> rcSet);
  OA_ptr<RCPairSetIterator> getRCPairIterator(StmtHandle s);

  //! ============ Access for CallStmt ==============
  void storeRCPairSet(CallHandle s, OA_ptr<RCPairSet> rcSet);
  OA_ptr<RCPairSetIterator> getRCPairIterator(CallHandle c);

  //! =============== Extra Output/Debugging =====================
  void output(IRHandlesIRInterface& ir) const;




  private:
    
    //! ===== data members =====
    std::map<StmtHandle, OA_ptr<RCPairSet> > mReachConsts;
    std::map<CallHandle, OA_ptr<RCPairSet> > mReachConstsForCallStmt;
  //std::map<MemRefHandle,OA_ptr<ConstValBasicInterface> > mMemRef2ReachConst;
};


  } // end of ReachConsts namespace
} // end of OA namespace

#endif


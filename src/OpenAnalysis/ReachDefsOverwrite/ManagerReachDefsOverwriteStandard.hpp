/*! \file
  
  \brief Declarations of the AnnotationManager that generates a ReachDefsOverwriteStandard

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ReachDefsOverwriteManagerStandard_h
#define ReachDefsOverwriteManagerStandard_h

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectInterface.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/IRInterface/ReachDefsIRInterface.hpp>
#include <OpenAnalysis/ReachDefs/ManagerReachDefsStandard.hpp>
#include <OpenAnalysis/ReachDefsOverwrite/ReachDefsOverwriteStandard.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>

namespace OA {
  namespace ReachDefsOverwrite {


typedef DataFlow::DataFlowSetImpl<Alias::AliasTag> TagsDFSet;


    /*! 
      The AnnotationManager for ReachDefsOverwriteStandard.
      This class can build an ReachDefsOverwriteStandard 
    */
    class ManagerReachDefsOverwriteStandard : public virtual ReachDefs::ManagerReachDefsStandard { 

    public:
      ManagerReachDefsOverwriteStandard(OA_ptr<ReachDefs::ReachDefsIRInterface> _ir);
      ~ManagerReachDefsOverwriteStandard () {}

      OA_ptr<ReachDefsOverwriteStandard> performAnalysis(ProcHandle, 
				 OA_ptr<CFG::CFGInterface> cfg, 
                                 OA_ptr<Alias::Interface> alias, 
                             OA_ptr<SideEffect::InterSideEffectInterface> interSE,
			     DataFlow::DFPImplement algorithm);
    private:
      
      OA_ptr<DataFlow::DataFlowSet> transfer(OA_ptr<DataFlow::DataFlowSet> in, OA::StmtHandle stmt); 

      std::map<StmtHandle, TagsDFSet> mStmtAllDefMap;

      OA_ptr<ReachDefsOverwriteStandard> mReachDefOverwriteMap;
    
    };

  } 
} 

#endif

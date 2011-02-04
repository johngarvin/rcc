/*! \file
  
  \brief Declarations of the AnnotationManager that generates a ReachDefsOverwriteXAIF

  \authors Jean Utke

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ManagerReachDefsXAIF_h
#define ManagerReachDefsXAIF_h

//--------------------------------------------------------------------

// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/ReachDefsOverwrite/Interface.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/XAIF/ReachDefsOverwriteXAIF.hpp>
#include <OpenAnalysis/IRInterface/XAIFIRInterface.hpp>

namespace OA {
  namespace XAIF {

    /*! 
      The AnnotationManager for ReachDefsXAIF.
      This class can build an ReachDefsXAIF, 
      (eventually) read one in from a file, and write one out to a file.
    */
    class ManagerReachDefsOverwriteXAIF  { 

    public:

      ManagerReachDefsOverwriteXAIF(OA_ptr<XAIFIRInterface>);
      ~ManagerReachDefsOverwriteXAIF () {}

      OA_ptr<ReachDefsOverwriteXAIF> performAnalysis(OA_ptr<CFG::CFGInterface> cfg,
						     OA_ptr<ReachDefsOverwrite::Interface> reachDefs);

    private:

      OA_ptr<XAIFIRInterface> mIR;    

      std::map<MemRefHandle,StmtHandle> mMemRefToStmt;

      static int ourCurrentStartId;

      static int getNextChainId(); 

      
    };

  } 
} 

#endif

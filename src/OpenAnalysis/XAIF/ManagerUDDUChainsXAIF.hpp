/*! \file
  
  \brief Declarations of the AnnotationManager that generates a 
         UDDUChainsXAIF.   These don't satisfy the UDDUChains::Interface
         because they are a filtered version of UDDUChains specifically
         for XAIF.

  \authors Michelle Strout
  \version $Id: ManagerUDDUChainsXAIF.hpp,v 1.9 2004/11/24 03:10:14 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  Special things about UDDUChainsXAIF:
  - If a chain element (StmtHandle) crosses basic block boundaries
    then the special StmtHandle(0) is used instead.  This includes crossing
    a BB for a loop.  For example, if a possible def for a use comes after
    the use in the BB, then that def will show up as StmtHandle(0) in
    the ud-chain.
  - The first UDDUChain includes the StmtHandle(0) by itself.
  - The zeroth UDDUChain is an empty chain

*/

#ifndef ManagerUDDUChainsXAIF_h
#define ManagerUDDUChainsXAIF_h

//--------------------------------------------------------------------
// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/UDDUChains/Interface.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/XAIF/UDDUChainsXAIF.hpp>
#include <OpenAnalysis/IRInterface/XAIFIRInterface.hpp>

namespace OA {
  namespace XAIF {


    /*! 
      The AnnotationManager for UDDUChainsXAIF.
      This class can build an UDDUChainsXAIF, 
      (eventually) read one in from a file, and write one out to a file.
    */
    class ManagerUDDUChainsXAIF { //??? eventually public OA::AnnotationManager

    public:
      ManagerUDDUChainsXAIF(OA_ptr<XAIFIRInterface>);
      ~ManagerUDDUChainsXAIF () {}

      OA_ptr<UDDUChainsXAIF> performAnalysis(OA_ptr<CFG::CFGInterface> cfg,
					     OA_ptr<UDDUChains::Interface> udChains,
					     bool donotfilterBB=false);
      
    private:
      OA_ptr<XAIFIRInterface> mIR;    
      std::map<StmtHandle,OA_ptr<CFG::NodeInterface> > mStmtToBBMap;
      std::map<MemRefHandle,StmtHandle> mMemRefToStmt;

      static int ourCurrentStartId;

      static int getNextChainId(); 

    };

  } // end of XAIF namespace
} // end of OA namespace

#endif

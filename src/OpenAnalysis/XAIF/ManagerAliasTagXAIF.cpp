/*! \file
  
  \brief The AnnotationManager that filters UDDUChains for XAIF.

  \authors Michelle Strout
  \version $Id: ManagerAliasMapXAIF.cpp,v 1.18 2005/03/17 21:47:47 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <OpenAnalysis/XAIF/ManagerAliasTagXAIF.hpp>
#include <OpenAnalysis/Utils/Util.hpp>

namespace OA {
  namespace XAIF {

bool debug = false;

static int setId = 1;

/*!
*/
ManagerAliasTagXAIF::ManagerAliasTagXAIF(OA_ptr<XAIFIRInterface> _ir) : mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerAliasMapXAIF:ALL", debug);
}


/*!
*/
OA_ptr<XAIF::AliasTagXAIF> ManagerAliasTagXAIF::performAnalysis(
          ProcHandle proc,  OA_ptr<Alias::Interface> alias)
{

  //! ====== Debug the ProcedureHandle ======

  if(debug) {
     std::cout << "The Procedure is" << mIR->toString(proc) << std::endl;
  }

  //! ======= Create initial AliasTagXAIF =======

  OA_ptr<AliasTagXAIF> retAliasMap; retAliasMap = new AliasTagXAIF(alias);

  //! ======= Iterate over StatementHandles in the Procedure ========

  OA_ptr<OA::IRStmtIterator> sItPtr = mIR->getStmtIterator(proc);

  for ( ; sItPtr->isValid(); (*sItPtr)++) {

       OA::StmtHandle stmt = sItPtr->current();

       //! =========== get all memory references =============

       OA_ptr<MemRefHandleIterator> mrItPtr = mIR->getAllMemRefs(stmt);

       for ( ; mrItPtr->isValid(); (*mrItPtr)++) {

            MemRefHandle memref = mrItPtr->current();

            //! =========== Debug Memory References =============

            if (debug) {
                std::cout << "memref = " 
                          << mIR->toString(memref) 
                          << std::endl;
            }

            //! =========== Get the AliasTags =============

            OA::OA_ptr<OA::Alias::AliasTagSet> deftagSet;

            deftagSet = alias->getAliasTags(memref);

            //! ===== Filter out AddressOf MemRefHandles =====

            OA_ptr<Alias::AliasTagIterator> dIt = deftagSet->getIterator();

            if(dIt->isValid()) {

               int setId = retAliasMap->getTagSetId(deftagSet);

               //! ===== if can't then create a new one and map it to a new one =====
 
               if (setId==AliasTagXAIF::SET_ID_NONE) {

                   setId = retAliasMap->makeEmptySet();

                   //! ====== Create the map between setId and vTag =====

                   retAliasMap->mapsetId2vTag(setId, deftagSet);

                   //! ===== Create the map between vTag and partialFlag =====
 
                  retAliasMap->mapsetId2PartialFlag(setId, deftagSet->isMust());

               }

               //! ===== Create the map between MemRefHandle and setId =====

               retAliasMap->mapMemRef2setId(memref, setId);

            } // AddressOf otherwise

       } //! === End For, Iterating over MemRefHandles ===

  } //! === End For, Iterating over MemRefHandles ===   

  return retAliasMap;

}

  } //! === end of namespace XAIF ===

} //! === end of namespace OA ===


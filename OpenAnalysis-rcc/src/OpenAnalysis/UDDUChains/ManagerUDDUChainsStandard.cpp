/*! \file
  
  \brief The AnnotationManager that generates UDDUChains.

  \authors Michelle Strout
  \version $Id: ManagerUDDUChainsStandard.cpp,v 1.21 2005/03/17 21:47:47 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerUDDUChainsStandard.hpp"

namespace OA {
  namespace UDDUChains {

bool debug = false;

//! ===========================================================
//!
//! Get all statements in the procedure
//!    Get All Uses in the Statement
//!       Get All Defs in Reaching Definitions of Statement
//!           if( Use == Def )
//!               Create UDDUStmtChain(stmt, ReachStmt)
//!               Create UDMemRefChain(Use, ReachStmt)
//!               Create DUMemRefChain(Def, Stmt)
//! ============================================================



/*!
*/

ManagerUDDUChainsStandard::ManagerUDDUChainsStandard(OA_ptr<UDDUChainsIRInterface> _ir) : mIR(_ir) 
{
 OA_DEBUG_CTRL_MACRO("DEBUG_ManagerUDDUChainsStandard:ALL", debug);
}


/*!
*/

OA_ptr<UDDUChainsStandard> ManagerUDDUChainsStandard::performAnalysis(ProcHandle proc, 
        OA_ptr<Alias::Interface> alias, OA_ptr<ReachDefs::Interface> reachDefs,
        OA_ptr<SideEffect::InterSideEffectInterface> interSE)
{

  //! ==================================
  //! Create new UDDUChains
  //! ==================================
  OA_ptr<UDDUChainsStandard> aUDDUChains;
  aUDDUChains = new UDDUChainsStandard(proc);

  //! ======================================================================
  //! for each reaching definition for a statement, need to determine 
  //! if that reaching definition has a def (due to memref or function call)
  //! that may alias a use (due to memref or function call) in the statement
  //! ======================================================================

  if (debug) {
    std::cout << "ManagerUDDUChainsStandard::performAnalysis: ";
    std::cout << std::endl;
  }


  //! ===========================================
  //! Get All Statements in the procedure
  //! ===========================================

  OA_ptr<OA::IRStmtIterator> stmtIterPtr;
  stmtIterPtr = mIR->getStmtIterator(proc);

  //! ==================================================================
  //! for each stmt keep track of number of reaching defs that are must
  //! reaching defs, if there are no must reaching defs and there
  //! were uses, we should add
  //! StmtHandle(0) into the udchain indicating that the define might be
  //! happening out of scope
  //! ==================================================================

  int mustCountForStmt = 0;
  int mayUseCountForStmt = 0;


  for ( ; stmtIterPtr->isValid(); (*stmtIterPtr)++) {
       OA::StmtHandle stmt = stmtIterPtr->current();


       //! =============================================
       //! loop over defs in this statement to make sure
       //! their du-chain is initialized to empty
       //! =============================================

       OA_ptr<MemRefHandleIterator> defIterPtr;
       defIterPtr = mIR->getDefMemRefs(stmt);
       for (; defIterPtr->isValid(); (*defIterPtr)++) {
            MemRefHandle def = defIterPtr->current();
            aUDDUChains->insertMemRefDef(def);
       } 


       //! ================================
       //! ForAll Uses in a Statement 
       //! ================================

       OA_ptr<MemRefHandleIterator> useIterPtr = mIR->getUseMemRefs(stmt);
       for (; useIterPtr->isValid(); (*useIterPtr)++) {

            MemRefHandle use = useIterPtr->current();
            aUDDUChains->insertMemRefUse(use);

            //! =============================================
            //! keep track of must reaching defs for this use
            //! =============================================
            mayUseCountForStmt++;
            int mustCountForUse = 0;
 
            //! ===========================
            //! Get AliasTags for the Use
            //! ===========================

            OA_ptr<Alias::AliasTagSet> UsetagSet;
            UsetagSet = alias->getAliasTags(use);

            //! =========================
            //! AliasTagIterator for Use
            //! =========================

            OA_ptr<Alias::AliasTagIterator> UseTagsIterPtr;
            UseTagsIterPtr = UsetagSet->getIterator();

            for ( ; UseTagsIterPtr->isValid(); ++(*UseTagsIterPtr) ) {
                Alias::AliasTag useTag = UseTagsIterPtr->current();

                //! ***************************************************
                //! ForAll, Reaching Definitions of the Statement
                //! ***************************************************

                //! ==================================================
                //! Type 1: Reaching Definition is Non-Call Statement
                //! ==================================================
                OA_ptr<ReachDefs::Interface::ReachDefsIterator> rdIterPtr; 
                rdIterPtr = reachDefs->getReachDefsIterator(stmt);
                for (; rdIterPtr->isValid(); ++(*rdIterPtr)) {
                     StmtHandle reachStmt = rdIterPtr->current();

                     //! ==========================
                     //! ForAll, Defs of ReachDefs
                     //! ==========================
                     OA_ptr<MemRefHandleIterator> defIterPtr;
                     defIterPtr = mIR->getDefMemRefs(reachStmt);
                     for (; defIterPtr->isValid(); (*defIterPtr)++) {
                          MemRefHandle def = defIterPtr->current();
                          aUDDUChains->insertMemRefDef(def);

                          //! ===========================
                          //! Get AliasTags for the Use
                          //! ===========================

                          OA_ptr<Alias::AliasTagSet> DeftagSet;
                          DeftagSet = alias->getAliasTags(def);
                          OA_ptr<Alias::AliasTagIterator> DefTagsIterPtr;
                          DefTagsIterPtr = DeftagSet->getIterator();
                          for ( ; DefTagsIterPtr->isValid();++(*DefTagsIterPtr)){                          
                            Alias::AliasTag defTag;
                            defTag = DefTagsIterPtr->current();

                 	    //! =================
                            //! use = def match 
                            //! =================
                            if(useTag == defTag) {
                              aUDDUChains->insertDefUse(reachStmt,stmt);
                              aUDDUChains->insertMemRefDefStmtUse(def,stmt);
                              aUDDUChains->insertStmtDefMemRefUse(reachStmt,use);

                              if(UsetagSet->isMust() && DeftagSet->isMust()) {
                                 mustCountForStmt++;
                                 mustCountForUse++;
                              }

                            } //! end if

                        } //! end for defAliasTagIterator

                  } //! end for defMemRefHandleIterator
         

       
                  //! ===================================================
                  //! ForAll, Reaching Definitions of a Statement through 
                  //! CallSites 
                  //! ================================================

                  OA_ptr<IRCallsiteIterator> callsiteIterPtr;
                  callsiteIterPtr = mIR->getCallsites(reachStmt);
                  for ( ; callsiteIterPtr->isValid(); ++(*callsiteIterPtr)) {
                       CallHandle call = callsiteIterPtr->current();

                       //! Iterate over the MODs/Defs due to function calls
                       OA_ptr<Alias::AliasTagIterator> aIter;
                       aIter = interSE->getMODIterator(call);
                       for( ; aIter->isValid(); ++(*aIter) ) {
                           OA::Alias::AliasTag defTag = aIter->current();
         
                           //! ================
                           //! use = def match
                           //! ================
                           if(useTag == defTag) { 

                              aUDDUChains->insertDefUse(reachStmt,stmt);
                              aUDDUChains->insertStmtDefMemRefUse(reachStmt,use);

                              //! ===============================
                              //! FIXME
                              //! PLM: Suspecious if this works
                              //! ===============================
                              
                              if(UsetagSet->isMust()) {
                                 mustCountForStmt++;
                                 mustCountForUse++;
                              }
                              

                           } //! end if

                       } //! end for aIter

                  } //! end for callsite Iterator

             } //! end for, end of reaching definitions for the given statement.


             //! ==============================================
             //! if current use had no must reaching defs then
             //! add StmtHandle(0)
             //! ==============================================

             if(mustCountForUse==0) {
                aUDDUChains->insertDefUse(StmtHandle(0),stmt);
                aUDDUChains->insertStmtDefMemRefUse(StmtHandle(0),use);
             }


        } //! end for, end of UseAliasTagsIterator

    } //! loop over uses in current stmt
   


    
    OA_ptr<IRCallsiteIterator> callsiteIterPtr = mIR->getCallsites(stmt);
    for ( ; callsiteIterPtr->isValid(); ++(*callsiteIterPtr)) {
         CallHandle call = callsiteIterPtr->current();

         //! ====================================
         //! Get the Uses at the callsite
         //! ====================================
         OA_ptr<Alias::AliasTagIterator> aIter;
         aIter = interSE->getUSEIterator(call);
        
         //! ====================================
         //! AliasTagsIterator for Uses
         //! ====================================
         for( ; aIter->isValid(); ++(*aIter) ) {
              OA::Alias::AliasTag atag = aIter->current();

              //! =============================================
              //! keep track of must reaching defs for this use
              //! =============================================
              mayUseCountForStmt++;
              int mustCountForUse = 0;

	      //! ==============================================
              //! Get the Reaching Definitions at the callsite
              //! ==============================================
              OA_ptr<ReachDefs::Interface::ReachDefsIterator> rdIterPtr;
              rdIterPtr = reachDefs->getReachDefsIterator(stmt);

              for (; rdIterPtr->isValid(); ++(*rdIterPtr)) {
                   StmtHandle reachStmt = rdIterPtr->current();

                   //! ==========================
                   //! Get the Defs of ReachDefs
                   //! ==========================
                   OA_ptr<MemRefHandleIterator> defIterPtr;
                   defIterPtr = mIR->getDefMemRefs(reachStmt);

                   for( ; defIterPtr->isValid(); ++(*defIterPtr) ) {
                       MemRefHandle def = defIterPtr->current();
                      
                       //! ===========================
                       //! Get AliasTags for the Defs 
                       //! ===========================
                       OA_ptr<Alias::AliasTagSet> DeftagSet;
                       DeftagSet = alias->getAliasTags(def);

                       //! ===========================
                       //! AliasTagsIterator for Defs
                       //! ===========================

                       OA_ptr<Alias::AliasTagIterator> DefTagsIterPtr;
                       DefTagsIterPtr = DeftagSet->getIterator();
                   
                       for (; DefTagsIterPtr->isValid(); ++(*DefTagsIterPtr) ) {
                            OA::Alias::AliasTag btag = DefTagsIterPtr->current();

                            //! ============================================
                            //! Does AliasTag for Def and Use are the same ?
                            //! ============================================
                            if( atag == btag ) {

                               //! =================
                               //! Found UDDUChains
                               //! =================
                               aUDDUChains->insertDefUse(reachStmt,stmt);
                               aUDDUChains->insertMemRefDefStmtUse(def,stmt);

                               if(DeftagSet->isMust()) {
                                 mustCountForStmt++;
                                 mustCountForUse++;
                               }
                           }

                       } //! end for, end defAliasTagsIter

                   } //! end for, end defIterPtr
              

                   //! ================================================
                   //! Reaching Definitions can be Call Statements
                   //! ================================================

                   OA_ptr<IRCallsiteIterator> rdcallsiteIterPtr;
                   rdcallsiteIterPtr = mIR->getCallsites(reachStmt);

                   for ( ;rdcallsiteIterPtr->isValid();++(*rdcallsiteIterPtr)) {
                        CallHandle call1 = rdcallsiteIterPtr->current();

                        //! =====================
                        //! Get Defs at callsite
                        //! =====================
                        OA_ptr<Alias::AliasTagIterator> aIter;
                        aIter = interSE->getMODIterator(call1);

                        //! ==========================
                        //! AliasTagsIter for the Defs 
                        //! ==========================
                        for( ; aIter->isValid(); ++(*aIter) ) {
                            OA::Alias::AliasTag btag = aIter->current();

                            //! ===============================================
                            //! Does AliasTag for the Def and Use are the same ?
                            //! ===============================================

                            if(atag == btag) {
                               
                               //! ================
                               //! Found UDDUChain
                               //! ================
                               aUDDUChains->insertDefUse(reachStmt,stmt);

                               //! FIXME
                               //! PLM April 8th 2008
                               //! Not sure how to find Use & Def Must Overlap

                            } // end if
                        } // end for, Iterate over the MOD set in the statement
                   } // end Iterator over callsite

              } // end the iterator over reaching definitions

              //! ===========================================
              //! if current use had no must reaching defs
              //! then add StmtHandle(0)
              //! ===========================================
              if (mustCountForUse==0) {
                  aUDDUChains->insertDefUse(StmtHandle(0),stmt);
              }

         } // end iterator over UseTags

    } // end iterator over callsites


    //! ==============================================================
    //! if there were no must reaching definitions for this statement
    //! then add StmtHandle(0) into the ud chain
    //! ==============================================================
    if (mustCountForStmt==0 && mayUseCountForStmt>0) {
        //! =======================================
        //! loop over uses in this statement
        //! =======================================
        OA_ptr<MemRefHandleIterator> useIterPtr = mIR->getUseMemRefs(stmt);
        for (; useIterPtr->isValid(); (*useIterPtr)++) {
            MemRefHandle use = useIterPtr->current();
            aUDDUChains->insertDefUse(StmtHandle(0),stmt);
            aUDDUChains->insertStmtDefMemRefUse(StmtHandle(0),use);
        }
    }

  } // loop over stmts


  //! =======================
  //! Get the Exit ReachDefs
  //! =======================
  OA_ptr<ReachDefs::Interface::ReachDefsIterator> rdIterPtr;
  rdIterPtr=reachDefs->getExitReachDefsIterator();

  for (; rdIterPtr->isValid(); ++(*rdIterPtr)) {
       StmtHandle reachStmt = rdIterPtr->current();

       //! ==========================
       //! Get the Defs of ReachDefs
       //! ==========================
       OA_ptr<MemRefHandleIterator> defIterPtr;
       defIterPtr = mIR->getDefMemRefs(reachStmt);

       for (; defIterPtr->isValid(); (*defIterPtr)++) {
              MemRefHandle def = defIterPtr->current();

              OA::OA_ptr<MemRefExprIterator> mreIterPtr;
              mreIterPtr = mIR->getMemRefExprIterator(def);

              for( ; mreIterPtr->isValid(); ++(*mreIterPtr) ) {
                 OA_ptr<MemRefExpr> mre = mreIterPtr->current();

                 if(mre->isaNamed()) {
                    OA_ptr<NamedRef> named = mre.convert<NamedRef>();
                    if(!(named->isStrictlyLocal())) {
 
                       //! ===================================
                       //! Found Exit Reaching Definitions
                       //! ===================================
                       aUDDUChains->insertMemRefDefStmtUse(def,StmtHandle(0));
                       aUDDUChains->insertDefUse(reachStmt,StmtHandle(0));

                    } 

                 } else {
                     //! ===================================
                     //! Found Exit Reaching Definitions
                     //! ===================================
                     aUDDUChains->insertMemRefDefStmtUse(def,StmtHandle(0));
                     aUDDUChains->insertDefUse(reachStmt,StmtHandle(0));
                 }
              }
       }


       //! =============================================================
       //! Go through Actual MREs and find out if they are nonlocal and
       //! Modified at the callsite.
       //! Do not consider all MODs at the callsite because
       //! AliasTags are passed across globally and you will
       //! get all the Deref of formal MREs also that you dont want.
       //! =============================================================


       OA_ptr<IRCallsiteIterator> callsiteIterPtr;
       callsiteIterPtr = mIR->getCallsites(reachStmt);
       for ( ; callsiteIterPtr->isValid(); ++(*callsiteIterPtr)) {
            CallHandle call = callsiteIterPtr->current();


            //! =============================================
            //! Get the Actual MRE
            //! =============================================
            OA::OA_ptr<OA::IRCallsiteParamIterator> aIter;
            aIter = mIR->getCallsiteParams(call);

            for( ; aIter->isValid(); ++(*aIter) ) {
                OA::ExprHandle expr = aIter->current();
            
                OA::OA_ptr<OA::ExprTree> eTree = mIR->getExprTree(expr);
  
                CollectMREVisitor evalVisitor;
                eTree->acceptVisitor(evalVisitor);

                OA::set<OA_ptr<MemRefExpr> > mSet;
                mSet = evalVisitor.getMemRef();

                OA::OA_ptr<std::set<OA_ptr<MemRefExpr> > > mSetPtr;
                mSetPtr = new std::set<OA_ptr<MemRefExpr> >(mSet);

                OA_ptr<MemRefExprIterator> mreIter;
                mreIter = new OA::MemRefExprIterator(mSetPtr);

                for( ; mreIter->isValid(); ++(*mreIter) ) {

                   OA::OA_ptr<OA::MemRefExpr> mre;
                   mre = mreIter->current();

                   //! ================================================
                   //! Check if actual mre is not strictlyLocal
                   //! ================================================
                    
                   //! composedWith Deref
                   int numDerefs = 1;
                   OA::OA_ptr<OA::Deref> deref_mre;
                   OA::OA_ptr<OA::MemRefExpr> nullMRE;
                   deref_mre = new OA::Deref(
                                             OA::MemRefExpr::USE,
                                             nullMRE,
                                             numDerefs);
                   mre = deref_mre->composeWith(mre->clone());


                   //! ===========================================
                   //! If actual mre is strictlyLocal then dont
                   //! want in the exit definitions, hence ignore
                   //! ===========================================

                   if(mre->isaNamed()) {
                    OA_ptr<NamedRef> named = mre.convert<NamedRef>();
                    if(named->isStrictlyLocal()) {
                       continue;
                    }
                   }  


                   //! ===========================================
                   //! Check if non-strictlyLocal actual mre is
                   //! modified at the call-site.
                   //! ===========================================  

                   OA::OA_ptr<OA::Alias::AliasTagSet> mretagSet;
                   mretagSet = alias->getAliasTags(mre);

                   OA_ptr<Alias::AliasTagIterator> aTagIterPtr;
                   aTagIterPtr = interSE->getMODIterator(call);

                   for ( ; aTagIterPtr->isValid(); ++(*aTagIterPtr)) {

                        Alias::AliasTag aTag = aTagIterPtr->current();

                        if(mretagSet->contains(aTag)) {
                           aUDDUChains->insertDefUse(reachStmt,StmtHandle(0));
                        } // end if
                   } // end for

                } // end for

            } // end for
       } // end for
  }

  return aUDDUChains;
  
}


  } // end of namespace UDDUChains
} // end of namespace OA

/*! \file
  
  \brief The AnnotationManager that generates InterReachConsts using ICFGDFSolver.

  \authors Michelle Strout, Barbara Kreaseck
  \version $Id: ManagerICFGReachConsts.cpp,v 1.1.2.1 2006/02/10 07:57:31 mstrout Exp $

  Copyright (c) 2002-2004, Rice University <br>
  Copyright (c) 2004, University of Chicago <br>  
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>

*/

#include <OpenAnalysis/ReachConsts/ManagerICFGReachConsts.hpp>


namespace OA {
  namespace ReachConsts {

static bool debug = true;

/*!
*/
ManagerICFGReachConsts::ManagerICFGReachConsts( 
    OA_ptr<ReachConstsIRInterface> _ir) : mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerICFGReachConsts:ALL", debug);
    mSolver = new DataFlow::ICFGDFSolver(DataFlow::ICFGDFSolver::Forward,*this);
}





OA_ptr<InterReachConsts> 
ManagerICFGReachConsts::performAnalysis(OA_ptr<ICFG::ICFGInterface> icfg,
                                        OA_ptr<Alias::Interface> Alias,
                                        DataFlow::DFPImplement algorithm)
{

   //! ===== store results that will be needed in callbacks =====
   mICFG = icfg;
   mAlias = Alias;

   //! ===== create an empty InterReachConsts =====
   mInterReachConsts = new InterReachConsts();

   //! ===== call iterative data-flow solver for ICFG =====
   mSolver->solve(icfg,algorithm);
    
   //! ===== assign results for each procedure into InterReachConsts =====
   std::map<ProcHandle,OA_ptr<ReachConstsStandard> >::iterator mapIter;
   for (mapIter=mReachConstsMap.begin(); mapIter!=mReachConstsMap.end(); 
        mapIter++) 
   {
        mInterReachConsts->mapProcToReachConsts(mapIter->first,mapIter->second);
   }

   return mInterReachConsts;
}


//! Helper function to transfer across an AssignPair, updates inSet, in-situ
OA_ptr<RCPairSet>
ManagerICFGReachConsts::transferAssignPair(StmtHandle stmt, CallHandle call, 
                                           bool haveStmt, 
                                           MemRefHandle mref, ExprHandle expr,
                                           ProcHandle proc,
                                           OA_ptr<RCPairSet> inSet)
{
  OA_ptr<RCPairSet> outSet = inSet->clone().convert<RCPairSet>();

  if (debug) {
    std::cout << "transAssignPair(" << mIR->toString(mref)
              << "," << mIR->toString(expr) << ")\n";
  }

  OA_ptr<ExprTree> eTreePtr;
  OA_ptr<ConstValBasicInterface> cvbiPtr;

  //! ===== get the expression tree for RHS =====    
  eTreePtr = mIR->getExprTree(expr);

  OA_ptr<ReachConsts::Interface> rcInterface;
  rcInterface = mReachConstsMap[proc]; 
  
  //! ===== Evaluate RHS expression =====
  OA_ptr<EvalToConstVisitor> evalVisitorPtr;
  if (haveStmt) {
    evalVisitorPtr = new EvalToConstVisitor(mIR, rcInterface, mAlias, stmt);
  } else {
    evalVisitorPtr = new EvalToConstVisitor(mIR, rcInterface, mAlias, call);
  }
  eTreePtr->acceptVisitor(*evalVisitorPtr);
  cvbiPtr = 0;
  cvbiPtr = evalVisitorPtr->getConstVal();
  
  //! Kill LHS from clone'd outSet
  OA_ptr<Alias::AliasTagSet> defSet = mAlias->getAliasTags(mref);
  RCPairSetIterator rcIter(*inSet);
  for (; rcIter.isValid(); ++(rcIter)) {
    RCPair rc = rcIter.current();
    if (defSet->contains(rc.getTag())) {
      if (debug) {
        std::cout << "\tkilling RCPair" << rc.toString(*mIR) << std::endl;
      }
      outSet->remove(rc);
    }
  }
  
  //! Gen RHS if we have a constVal
  // do we care whether defSet->isMust() ??
  
  if (!cvbiPtr.ptrEqual(NULL)) {
    if (debug) {
      std::cout << "\texpr == (" << cvbiPtr->toString() << ")\n";
    }
    if (defSet->isMust()) {
      if (debug) {
        std::cout << "\tdefSet is MUST\n";
      }
      OA_ptr<Alias::AliasTagIterator> aIter = defSet->getIterator();
      for ( ; aIter->isValid() ; ++(*aIter)) {
        Alias::AliasTag tag = aIter->current();
        if (debug) {
          std::cout << "\t\tgen'ing RCPair<" << tag << ",VALUE="
                    << cvbiPtr->toString() << ">\n";
        }
        outSet->insert(RCPair(tag,cvbiPtr)); 
      }
    } else {
      if (debug) {
        std::cout << "\tdefSet isNOT MUST\n";
      }
    }
  } else {
    if (debug) {
      std::cout << "\texpr != const val\n";
    }
  }

  return outSet;
}



//! *******************************************************************
//! implementation of ICFGDFProblem interface below
//! *******************************************************************



//! ===== TOP indicates Universal Set =====
OA_ptr<DataFlow::DataFlowSet> ManagerICFGReachConsts::initializeTop()
{
  //! Initialize to TOP
  OA_ptr<RCPairSet> retval;
  retval = new RCPairSet();
  retval->setUniversal(); // Universal is TOP
  return retval;
}






OA_ptr<DataFlow::DataFlowSet>
ManagerICFGReachConsts::initializeNodeIN(OA_ptr<ICFG::NodeInterface> n)
{

   OA_ptr<RCPairSet> retval;

   if (n->isAnEntry() && (n->size() == 0)) {
     retval = new RCPairSet(); // default is NOT universal (i.e., BOTTOM)
   } else {
     retval = new RCPairSet();
     retval->setUniversal(); // universal is TOP
   }

   return retval;
}






OA_ptr<DataFlow::DataFlowSet>
ManagerICFGReachConsts::initializeNodeOUT(OA_ptr<ICFG::NodeInterface> n)
{
    return initializeTop();
}






  
//! ====================================================================
//! ReachConsts is a forward MUST analysis, so meet is intersection
//! -- since RCPair::operator== does a full compare, intersection will
//!    only contain those pairs with the same AliasTag and ConstValBasic value
//! ====================================================================


OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGReachConsts::meet(OA_ptr<DataFlow::DataFlowSet> set1, 
                             OA_ptr<DataFlow::DataFlowSet> set2)
{

  //! ======================================================
  //! Get RCPair DataFlow Set
  //! ======================================================

  OA_ptr<DataFlow::DataFlowSet> set1clone = set1->clone();
  OA_ptr<DataFlow::DataFlowSet> set2clone = set2->clone();
  OA_ptr<RCPairSet> rcSet1 = set1clone.convert<RCPairSet>();
  OA_ptr<RCPairSet> rcSet2 = set2clone.convert<RCPairSet>();


  if(debug) {
     std::cout << "\tmeet rcSet1 = ";
     rcSet1->dump(std::cout, mIR);
     std::cout << std::endl;
     std::cout << "\tmeet rcSet2 = ";
     rcSet2->dump(std::cout, mIR);
     std::cout << std::endl;
  }


  //! =================================
  //! intersect rcSet1 into rcSet2 
  //! =================================
  
  // no problem changing rcSet2 since it is a clone.
  rcSet2->intersectEqu(*rcSet1);




  if (debug) {
    std::cout << "\tmeet retval = ";
    rcSet2->dump(std::cout, mIR);
    std::cout << std::endl;
  }

  return rcSet2;
}


//! ===============================================================
//! 1. Get the Assignment Pairs(LHS, RHS) per statement
//! 2. evaluate RHS and calculate gen constants
//! 3. kill LHS old constant and replace them with new constant
//!    dont store non-constant LHS
//! ==============================================================

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGReachConsts::transfer(ProcHandle proc,
                                 OA_ptr<DataFlow::DataFlowSet> in, 
                                 OA::StmtHandle stmt)
{

  OA_ptr<RCPairSet> inRecast = in->clone().convert<RCPairSet>();
  OA_ptr<RCPairSet> outRecast = in->clone().convert<RCPairSet>();

  if (debug) {
    std::cout << "stmtTransfer(" << mIR->toString(stmt) << ")\n";
  }

  if (inRecast->isUniversalSet()) {
    // then we probably hit a RETURN_NODE whose CALL_NODE has been visited
    // but whose EXIT_NODE has not been visited prior to visiting this node.
    // Just return UniversalSet() for now, and during the next iteration,
    // the EXIT_NODE will have already been visited ... =)
    return outRecast;
  }

  //! ===== if there isn't a ReachConsts for this proc then make one =====
  if (mReachConstsMap.find(proc)==mReachConstsMap.end()) {
      mReachConstsMap[proc] = new ReachConstsStandard(proc);
  }


  //! ===== set the reaching constant definitions for the statement =====
  mReachConstsMap[proc]->storeRCPairSet(stmt,inRecast);


  //! ===== get the assign pairs for the statement =====
  OA_ptr<AssignPairIterator> espIterPtr;
  espIterPtr = mIR->getAssignPairIterator(stmt);

  //! ===== count number of assign pairs for the statement =====
  int count = 0;
  for (espIterPtr->reset(); espIterPtr->isValid(); (*espIterPtr)++) {
    count++;
  }

  if (debug) {
    std::cout << "\tthere are " << count << " assignPairs\n";
  }

  MemRefHandle mref;
  ExprHandle expr;
  OA_ptr<ExprTree> eTreePtr;
  OA_ptr<ConstValBasicInterface> cvbiPtr;

  //! handle one assignPair differently from more-than-one assignPair
  if (count <= 1) {
    espIterPtr->reset();
    if (espIterPtr->isValid()) { // then there is one assignPair
      mref = espIterPtr->currentTarget();
      expr = espIterPtr->currentSource();
      
      outRecast = transferAssignPair(stmt, CallHandle(0), true,
                                     mref,expr,proc,inRecast);

    }

  } else { // assignPair count > 1

    /*    // proposal #1: iterator order (allows indiscriminant use and def)
    //-----------------------------------------------------------------
    for (espIterPtr->reset(); espIterPtr->isValid(); (*espIterPtr)++) {
      outRecast = transferAssignPair(espIterPtr->currentTarget(),
                                     espIterPtr->currentSource(),
                                     outRecast);
    }
    */

    OA_ptr<ReachConsts::Interface> rcInterface;
    rcInterface = mReachConstsMap[proc];

    // proposal #5: uses Partial-Universal sets
    //-----------------------------------------------------------------
    vector<OA_ptr<RCPairSet> > apOut(count);
    vector<OA_ptr<RCPairSet> > apKill(count);
    vector<OA_ptr<RCPairSet> > apGenTop(count); // for speedup?
    vector<bool> isMustDef(count); // for speedup?
    OA_ptr<RCPairSet> apDefTags; // needed?
    OA_ptr<RCPairSet> genTOP;
    OA_ptr<RCPairSet> mayDefKills;
    OA_ptr<RCPairSet> apInAll;
    OA_ptr<RCPairSet> oldApInAll;
    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > aIter;

    // 1. loop thru APs and union all of the def tags (must or may)
    // and 2. create Partial with all def tags
    genTOP = new RCPairSet();
    mayDefKills = new RCPairSet();
    int apCnt; 
    for (espIterPtr->reset(), apCnt=0; 
         espIterPtr->isValid(); 
         (*espIterPtr)++, apCnt++) {
      MemRefHandle mref = espIterPtr->currentTarget();
      OA_ptr<Alias::AliasTagSet> defSet = mAlias->getAliasTags(mref);
      isMustDef[apCnt] = defSet->isMust();
      apKill[apCnt] = new RCPairSet(defSet); // makes partial with defSet tags
      genTOP->unionEqu(*(apKill[apCnt]));
      if (! (defSet->isMust()) ) {
        mayDefKills->unionEqu(*(apKill[apCnt]));
      }
    }

    // 3. set-up for convergence iterations
    for (espIterPtr->reset(), apCnt=0; 
         espIterPtr->isValid(); 
         (*espIterPtr)++, apCnt++) {
      apGenTop[apCnt] = (genTOP->clone()).convert<RCPairSet>();
      apGenTop[apCnt]->minusEqu(*(apKill[apCnt]));
    }

    apInAll = inRecast->clone().convert<RCPairSet>();

    // subtract out the mayDefs to reduce iterations
    apInAll->minusEqu(*mayDefKills);

    bool changed = true;
    
    // 4. iterate: compute apOut sets and apInAll until convervence
    while (changed) {
      changed = false;

      oldApInAll = (apInAll->clone()).convert<RCPairSet>();

      for (espIterPtr->reset(), apCnt=0; 
           espIterPtr->isValid(); 
           (*espIterPtr)++, apCnt++) {
        // kills
        apOut[apCnt] = (apInAll->clone()).convert<RCPairSet>();;
        apOut[apCnt]->minusEqu(*(apKill[apCnt]));
        apOut[apCnt]->unionEqu(*(apGenTop[apCnt]));

        // gens
        if (isMustDef[apCnt]) {

          if (debug) {
            std::cout << "\tdefSet is MUST\n";
          }
 
          ExprHandle expr = espIterPtr->currentSource();

          OA_ptr<ExprTree> eTreePtr;
          OA_ptr<ConstValBasicInterface> cvbiPtr;
          
          //! ===== get the expression tree for RHS =====    
          eTreePtr = mIR->getExprTree(expr);
          
          //! ===== Evaluate RHS expression =====
          //EvalToConstVisitor evalVisitor(mIR, rcInterface, mAlias, stmt);
          EvalToConstVisitor evalVisitor(mIR, mReachConstsMap[proc],
                                         mAlias, stmt);
          eTreePtr->acceptVisitor(evalVisitor);
          cvbiPtr = 0;
          cvbiPtr = evalVisitor.getConstVal();

          if (!cvbiPtr.ptrEqual(NULL)) {
            if (debug) {
              std::cout << "\texpr == (" << cvbiPtr->toString() << ")\n";
            }
            
            //! ===== Generate RCPairs directly into out set =====
            aIter = apKill[apCnt]->getTagSetIterator();
            for ( ; aIter->isValid() ; ++(*aIter)) {
              Alias::AliasTag tag = aIter->current();
              if (debug) {
                std::cout << "\t\tgen'ing RCPair<" << tag << ",VALUE="
                          << cvbiPtr->toString() << ">\n";
              }
              // none of these tags should be in apOut[apCnt] because
              // we just minusEqu'd them above
              apOut[apCnt]->insert(RCPair(tag,cvbiPtr)); 
            }

          } else {
            if (debug) {
              std::cout << "\texpr != const val\n";
            }
          }
        } else {
          if (debug) {
            std::cout << "\tdefSet isNOT MUST\n";
          }
        }

        //! update inset for next iteration
        apInAll->intersectEqu(*(apOut[apCnt]));

        //! store it in the mReachConstsMap for next eval
        mReachConstsMap[proc]->storeRCPairSet(stmt,apInAll);

      } // end of assign pairs loop

      // check for convergence
      if (oldApInAll != apInAll) {
        changed = true;
      }

    } // end of while(changed) convergence loop

    // 5. calculate out set for stmt

    outRecast = (apOut[0]->clone()).convert<RCPairSet>();
    for (apCnt=1; apCnt<count; apCnt++) {
      outRecast->intersectEqu(*(apOut[apCnt]));
    }

  } // end count > 1

  if (debug) {
    std::cout << "\ttransfer outRecast = ";
    outRecast->dump(std::cout, mIR);
    std::cout << std::endl;
  }
  
  return outRecast;

}





OA_ptr<DataFlow::DataFlowSet>
ManagerICFGReachConsts::transfer(ProcHandle proc,
                                 OA_ptr<DataFlow::DataFlowSet> in,
                                 OA::CallHandle call)
{

  OA_ptr<RCPairSet> inRecast = in->clone().convert<RCPairSet>();
  OA_ptr<RCPairSet> outRecast = in->clone().convert<RCPairSet>();
  
  if (debug) {
    std::cout << "callTransfer(" << mIR->toString(call) << ")\n";
  }

  if (inRecast->isUniversalSet()) {
    // then we are probably a RETURN_NODE whose CALL_NODE has been visited
    // but whose EXIT_NODE has not been visited.  Just return UniversalSet()
    // for now, and during the next iteration, the EXIT_NODE will have already
    // been visited ... =)
    return outRecast;
  }

  //! ===== if there isn't a ReachConsts for this proc then make one =====
  if (mReachConstsMap.find(proc)==mReachConstsMap.end()) {
      mReachConstsMap[proc] = new ReachConstsStandard(proc);
  }
  
  //! ===== set the reaching constant definitions for the statement =====
  mReachConstsMap[proc]->storeRCPairSet(call,inRecast);

  //! ===== get the assign pairs for the statement =====
  OA_ptr<AssignPairIterator> espIterPtr;
  espIterPtr = mIR->getAssignPairIterator(call);

  //! ===== count number of assign pairs for the statement =====
  int count = 0;
  for (espIterPtr->reset(); espIterPtr->isValid(); (*espIterPtr)++) {
    count++;
  }

  if (debug) {
    std::cout << "\tthere are " << count << " assignPairs\n";
  }

  MemRefHandle mref;
  ExprHandle expr;
  OA_ptr<ExprTree> eTreePtr;
  OA_ptr<ConstValBasicInterface> cvbiPtr;

  //! handle one assignPair differently from more-than-one assignPair
  if (count <= 1) {
    espIterPtr->reset();
    if (espIterPtr->isValid()) { // then there is one assignPair
      mref = espIterPtr->currentTarget();
      expr = espIterPtr->currentSource();
      
      outRecast = transferAssignPair(StmtHandle(0), call, false,
                                     mref, expr, proc, inRecast);

    }

  } else { // assignPair count > 1

    OA_ptr<ReachConsts::Interface> rcInterface;
    rcInterface = mReachConstsMap[proc];

    // proposal #5: uses Partial-Universal sets
    //-----------------------------------------------------------------
    vector<OA_ptr<RCPairSet> > apOut(count);
    vector<OA_ptr<RCPairSet> > apKill(count);
    vector<OA_ptr<RCPairSet> > apGenTop(count); // for speedup?
    vector<bool> isMustDef(count); // for speedup?
    OA_ptr<RCPairSet> apDefTags; // needed?
    OA_ptr<RCPairSet> genTOP;
    OA_ptr<RCPairSet> mayDefKills;
    OA_ptr<RCPairSet> apInAll;
    OA_ptr<RCPairSet> oldApInAll;
    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > aIter;

    // 1. loop thru APs and union all of the def tags (must or may)
    // and 2. create Partial with all def tags
    genTOP = new RCPairSet();
    mayDefKills = new RCPairSet();
    int apCnt; 
    for (espIterPtr->reset(), apCnt=0; 
         espIterPtr->isValid(); 
         (*espIterPtr)++, apCnt++) {
      MemRefHandle mref = espIterPtr->currentTarget();
      OA_ptr<Alias::AliasTagSet> defSet = mAlias->getAliasTags(mref);
      isMustDef[apCnt] = defSet->isMust();
      apKill[apCnt] = new RCPairSet(defSet); // makes partial with defSet tags
      genTOP->unionEqu(*(apKill[apCnt]));
      if (! (defSet->isMust()) ) {
        mayDefKills->unionEqu(*(apKill[apCnt]));
      }
    }

    // 3. set-up for convergence iterations
    for (espIterPtr->reset(), apCnt=0; 
         espIterPtr->isValid(); 
         (*espIterPtr)++, apCnt++) {
      apGenTop[apCnt] = (genTOP->clone()).convert<RCPairSet>();
      apGenTop[apCnt]->minusEqu(*(apKill[apCnt]));
    }

    apInAll = inRecast->clone().convert<RCPairSet>();

    // subtract out the mayDefs to reduce iterations
    apInAll->minusEqu(*mayDefKills);

    bool changed = true;
    
    // 4. iterate: compute apOut sets and apInAll until convervence
    while (changed) {
      changed = false;

      oldApInAll = (apInAll->clone()).convert<RCPairSet>();

      for (espIterPtr->reset(), apCnt=0; 
           espIterPtr->isValid(); 
           (*espIterPtr)++, apCnt++) {
        // kills
        apOut[apCnt] = (apInAll->clone()).convert<RCPairSet>();;
        apOut[apCnt]->minusEqu(*(apKill[apCnt]));
        apOut[apCnt]->unionEqu(*(apGenTop[apCnt]));

        // gens
        if (isMustDef[apCnt]) {

          if (debug) {
            std::cout << "\tdefSet is MUST\n";
          }
 
          ExprHandle expr = espIterPtr->currentSource();

          OA_ptr<ExprTree> eTreePtr;
          OA_ptr<ConstValBasicInterface> cvbiPtr;
          
          //! ===== get the expression tree for RHS =====    
          eTreePtr = mIR->getExprTree(expr);
          
          //! ===== Evaluate RHS expression =====
          
          EvalToConstVisitor evalVisitor(mIR, rcInterface, mAlias, call);
          eTreePtr->acceptVisitor(evalVisitor);
          cvbiPtr = 0;
          cvbiPtr = evalVisitor.getConstVal();

          if (!cvbiPtr.ptrEqual(NULL)) {
            if (debug) {
              std::cout << "\texpr == (" << cvbiPtr->toString() << ")\n";
            }
            
            //! ===== Generate RCPairs directly into out set =====
            aIter = apKill[apCnt]->getTagSetIterator();
            for ( ; aIter->isValid() ; ++(*aIter)) {
              Alias::AliasTag tag = aIter->current();
              if (debug) {
                std::cout << "\t\tgen'ing RCPair<" << tag << ",VALUE="
                          << cvbiPtr->toString() << ">\n";
              }
              // none of these tags should be in apOut[apCnt] because
              // we just minusEqu'd them above
              apOut[apCnt]->insert(RCPair(tag,cvbiPtr)); 
            }

          } else {
            if (debug) {
              std::cout << "\texpr != const val\n";
            }
          }
        } else {
          if (debug) {
            std::cout << "\tdefSet isNOT MUST\n";
          }
        }

        //! update inset for next iteration
        apInAll->intersectEqu(*(apOut[apCnt]));

        //! store it in the mReachConstsMap for next eval
        mReachConstsMap[proc]->storeRCPairSet(call,apInAll);
        
      } // end of assign pairs loop

      // check for convergence
      if (oldApInAll != apInAll) {
        changed = true;
      }

    } // end of while(changed) convergence loop

    // 5. calculate out set for stmt

    outRecast = (apOut[0]->clone()).convert<RCPairSet>();
    for (apCnt=1; apCnt<count; apCnt++) {
      outRecast->intersectEqu(*(apOut[apCnt]));
    }

  } // end count > 1

  if (debug) {
    std::cout << "\ttransfer outRecast = ";
    outRecast->dump(std::cout, mIR);
    std::cout << std::endl;
  }
  
  return outRecast;

}







//! ===================================================================
//! This function evaluate assignment paires at the call-site
//! and propogate resulting constant values from caller to callee.
//! Assignment pairs at the call-site can be found by using 
//! expressions as actual.
//! ==================================================================

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGReachConsts::callerToCallee(ProcHandle caller,
                                       OA_ptr<DataFlow::DataFlowSet> dfset, 
                                       CallHandle call, ProcHandle callee)
{
    return dfset->clone();
}









//! ==================================================================
//! Propogate Constant Value AliasTag Set from Callee procedure 
//! To Caller procedure. No extra work required. 
//! ==================================================================
  
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGReachConsts::calleeToCaller(ProcHandle callee,
                                       OA_ptr<DataFlow::DataFlowSet> dfset, 
                                       CallHandle call, ProcHandle caller)
{
    return dfset->clone();
}








//! =================================================================
//! Propagate a data-flow set from call node to return node
//! No extra work required
//! =================================================================

OA_ptr<DataFlow::DataFlowSet>
ManagerICFGReachConsts::callToReturn(ProcHandle caller,
                                     OA_ptr<DataFlow::DataFlowSet> dfset,
                                     CallHandle call, ProcHandle callee)
{
  OA_ptr<RCPairSet> outSet;
  outSet = new RCPairSet();
  outSet->setUniversal();
  return outSet;

}






//! ===================================================================
//! Nothing special for exit nodes in a procedure
//! This should never be called since we are a forward analysis
//! ===================================================================

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGReachConsts::exitTransfer(ProcHandle proc, 
                                     OA_ptr<DataFlow::DataFlowSet> out)
{
    return out->clone();
}





OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGReachConsts::entryTransfer(ProcHandle proc, 
                                      OA_ptr<DataFlow::DataFlowSet> in)
{
    OA_ptr<DataFlow::DataFlowSet> inclone = in->clone();   
    //OA_ptr<RCPairSet> inRecast = inclone.convert<RCPairSet>();    
    return inclone;
}






  } // end of namespace ReachConsts
} // end of namespace OA

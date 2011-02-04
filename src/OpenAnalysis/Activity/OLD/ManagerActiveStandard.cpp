/*! \file
  
  \brief The AnnotationManager that generates ActiveStandard.

  \authors Michelle Strout
  \version $Id: ManagerActiveStandard.cpp,v 1.8 2005/06/10 02:32:02 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerActiveStandard.hpp"
#include <Utils/Util.hpp>

namespace OA {
  namespace Activity {

static bool debug = false;

/*!
*/
ManagerActiveStandard::ManagerActiveStandard(OA_ptr<ActivityIRInterface> _ir) 
    : mIR(_ir) 
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerActiveStandard:ALL", debug);
}

/*!
OA_ptr<ActiveStandard> ManagerActiveStandard::performAnalysis(ProcHandle proc, 
        OA_ptr<CFG::Interface> cfg,
        OA_ptr<Alias::Interface> alias, 
        OA_ptr<SideEffect::InterSideEffectInterface> interSE)
{
    return performAnalysis(proc, cfg, alias, interSE,
                           interSE->getUSEIterator(proc), 
                           interSE->getMODIterator(proc));
}
*/
 
/*!
*/
OA_ptr<ActiveStandard> ManagerActiveStandard::performAnalysis(ProcHandle proc, 
        OA_ptr<CFG::CFGInterface> cfg,
        OA_ptr<Alias::Interface> alias,
        OA_ptr<VaryStandard> vary,
        OA_ptr<UsefulStandard> useful)
{
  mAlias = alias;
  mActive = new ActiveStandard(proc);

  if (debug) {
    std::cout << "ManagerActiveStandard::performAnalysis: ";
    std::cout << "\tAnalyzing procedure " << mIR->toString(proc);
    std::cout << std::endl;
  }

  // first create DepStandard, VaryStandard, and UsefulStandard
  // Dep Analysis
//  OA_ptr<ManagerDepStandard> depman;
//  depman = new ManagerDepStandard(mIR);

  //mDep = depman->performAnalysis(proc, alias, cfg);

//  if (debug) { mDep->dump(std::cout, mIR); }

  // Vary
//  OA_ptr<ManagerVaryStandard> varyman;
//  varyman = new ManagerVaryStandard(mIR);

//  OA_ptr<VaryStandard> vary
//        = varyman->performAnalysis(proc, cfg, mDep, indepLocIter);

//  if (debug) { vary->dump(std::cout, mIR); }

  // Useful
//  OA_ptr<ManagerUsefulStandard> usefulman;
//  usefulman = new ManagerUsefulStandard(mIR);

//  OA_ptr<UsefulStandard> useful
 //       = usefulman->performAnalysis(proc, cfg, mDep, depLocIter);

//  if (debug) { useful->dump(std::cout, mIR); }

  // initialize OutVaryIterator to iterator over independent variables
  OA_ptr<LocIterator> outVaryIterPtr = vary->getIndepSetIterator();
  OA_ptr<LocIterator> depLocIter = useful->getDepSetIterator();
  StmtHandle prevStmt = StmtHandle(0);

  // for each statement in the procedure
  OA_ptr<OA::IRStmtIterator> stmtIterPtr = mIR->getStmtIterator(proc);
  for ( ; stmtIterPtr->isValid(); (*stmtIterPtr)++) {
    OA::StmtHandle stmt = stmtIterPtr->current();
 
    // send iterator for OutVary from previous stmt and InVary for current
    // stmt into a helper function that determines active locations, whether
    // the previous stmt was active, and which memory references in the
    // previous and current stmt are active
    calculateActive(prevStmt, outVaryIterPtr, 
                    stmt, useful->getInUsefulIterator(stmt));

    // get OutVary for this stmt
    outVaryIterPtr = vary->getOutVaryIterator(stmt);
    prevStmt = stmt;

  }

  // do calculation for point after last stmt
  calculateActive(prevStmt, outVaryIterPtr, StmtHandle(0), depLocIter);

   return mActive;
  
}

/*! 
    A helper function that determines active locations, whether
    the previous stmt was active, and which memory references in the
    previous and current stmt are active
*/
void ManagerActiveStandard::calculateActive(
        StmtHandle prevStmt, OA_ptr<LocIterator> prevOutVaryIter,
        StmtHandle stmt, OA_ptr<LocIterator> inUsefulIter)
{
    if (debug) {
        std::cout << "\tcalculateActive ---------------------" << std::endl;
    }

    // get set of active locations
    prevOutVaryIter->reset();
    for ( ; prevOutVaryIter->isValid(); (*prevOutVaryIter)++ ) {
        OA_ptr<Location> loc = prevOutVaryIter->current();
        if (debug) { 
            std::cout << "\t\tinVary loc = ";
            loc->dump(std::cout,mIR);
        }
        inUsefulIter->reset();
        for ( ; inUsefulIter->isValid(); (*inUsefulIter)++ ) {
            if (debug) { 
                std::cout << "\t\tinUseful loc = ";
                inUsefulIter->current()->dump(std::cout,mIR);
            }
            if (loc->mayOverlap(*(inUsefulIter->current())) ) {
                mActive->insertLoc(loc);
                mActive->insertLoc(inUsefulIter->current());
                if (debug) {
                  std::cout << "\t\tinserting active loc = ";
                  loc->dump(std::cout,mIR);
                  std::cout << "\t\tinserting active loc = ";
                  inUsefulIter->current()->dump(std::cout,mIR);
                }
            }
        }
    }

    // get all the defines from prevStmt and if any of those memory 
    // references have maylocs that may overlap with the active
    // locations then the prevStmt and the memory references are active
    if (prevStmt != StmtHandle(0)) {
      OA_ptr<MemRefHandleIterator> defIterPtr = mIR->getDefMemRefs(prevStmt);
      for ( ; defIterPtr->isValid(); (*defIterPtr)++ ) {
        MemRefHandle def = defIterPtr->current();
        if (debug) {
          std::cout << "\t\tdef (" << def.hval() << ") = ";
          mIR->dump(def,std::cout);
        }

        OA_ptr<LocIterator> activeIterPtr 
            = mActive->getActiveLocsIterator();
        OA_ptr<LocIterator> locIterPtr = mAlias->getMayLocs(def);
        for (; locIterPtr->isValid(); ++(*locIterPtr)) {
            OA_ptr<Location> loc = locIterPtr->current();
            activeIterPtr->reset();
            for (; activeIterPtr->isValid(); (*activeIterPtr)++ ) {
                if (loc->mayOverlap(*(activeIterPtr->current())) ) {
                    mActive->insertStmt(prevStmt);
                    mActive->insertMemRef(def);
                }
            }
        }
      }
    }
     
    // get all the uses from stmt and if any of those memory 
    // references have maylocs that may overlap with the active
    // locations then those memory references are active
    if (stmt != StmtHandle(0)) {
      OA_ptr<MemRefHandleIterator> useIterPtr = mIR->getUseMemRefs(stmt);
      for ( ; useIterPtr->isValid(); (*useIterPtr)++ ) {
        MemRefHandle use = useIterPtr->current();
        if (debug) {
          std::cout << "\t\tuse (" << use.hval() << ") = ";
          mIR->dump(use,std::cout);
        }

        OA_ptr<LocIterator> locIterPtr = mAlias->getMayLocs(use);
        OA_ptr<LocIterator> activeIterPtr 
            = mActive->getActiveLocsIterator();
        for (; locIterPtr->isValid(); ++(*locIterPtr)) {
            OA_ptr<Location> loc = locIterPtr->current();
            activeIterPtr->reset();
            for (; activeIterPtr->isValid(); (*activeIterPtr)++ ) {
                if (loc->mayOverlap(*(activeIterPtr->current())) ) {
                    mActive->insertMemRef(use);
                }
            }
        }
      }
    }
     
}


  } // end of namespace Activity
} // end of namespace OA

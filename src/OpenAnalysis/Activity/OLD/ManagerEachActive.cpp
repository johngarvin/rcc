/*! \file
  
  \brief The AnnotationManager that generates InterActive INTRAprocedurally.

  \authors Michelle Strout
  \version $Id: ManagerEachActive.cpp,v 1.4.6.1 2005/08/23 18:19:14 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  Side-effect results are used to estimate the effect of procedure calls.
  Also used to seed independent and dependent variables for procedures who
  have unknown loc in their independent or dependent location seeds.
*/

#include "ManagerEachActive.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace Activity {

static bool debug = false;

ManagerEachActive::ManagerEachActive(
    OA_ptr<Activity::ActivityIRInterface> _ir) : mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerEachActive:ALL", debug);
}

/*!
   Just calls ManagerActiveStandard::performAnalysis on each procedure
*/
OA_ptr<InterActive> 
ManagerEachActive::performAnalysis(
        OA_ptr<IRProcIterator> procIter,
        OA_ptr<Alias::InterAliasInterface> interAlias,
        OA_ptr<SideEffect::InterSideEffectInterface> interSE,
        OA_ptr<CFG::EachCFGInterface> eachCFG)
{
  OA_ptr<InterActive> retval;
  retval = new InterActive;

  // Iterate over the procedures
  for ( procIter->reset(); procIter->isValid(); ++(*procIter)) { 
    
    ProcHandle proc = (irhandle_t)(procIter->current().hval());

    // get Alias::Interface for this proc
    OA_ptr<Alias::Interface> alias;
    alias = interAlias->getAliasResults(proc);

    // get CFG for this proc
    OA_ptr<CFG::CFGInterface> cfg;
    cfg = eachCFG->getCFGResults(proc);

    // create Active manager
    OA_ptr<Activity::ManagerActiveStandard> activeman;
    activeman = new Activity::ManagerActiveStandard(mIR);

    // get iterator over independent locations for procedure
    OA_ptr<LocIterator> indepIter;
    OA_ptr<LocSet> indepSet; indepSet = new LocSet();
    OA_ptr<MemRefExprIterator> indepMREIter=mIR->getIndepMemRefExprIter(proc);
    for ( indepMREIter->reset(); indepMREIter->isValid(); (*indepMREIter)++ ) {
      OA_ptr<MemRefExpr> memref = indepMREIter->current();

      // get may locs for memref
      OA_ptr<LocIterator> locIter = alias->getMayLocs(*memref,proc);
      for (locIter->reset(); locIter->isValid(); (*locIter)++ ) {
        indepSet->insert(locIter->current());
      }
    }
    indepIter = new LocSetIterator(indepSet);
    
    // get iterator over dependent locations for procedure
    OA_ptr<LocIterator> depIter;
    OA_ptr<LocSet> depSet; depSet = new LocSet();
    OA_ptr<MemRefExprIterator> depMREIter=mIR->getDepMemRefExprIter(proc);
    for ( depMREIter->reset(); depMREIter->isValid(); (*depMREIter)++ ) {
      OA_ptr<MemRefExpr> memref = depMREIter->current();

      // get may locs for memref
      OA_ptr<LocIterator> locIter = alias->getMayLocs(*memref,proc);
      for (locIter->reset(); locIter->isValid(); (*locIter)++ ) {
        depSet->insert(locIter->current());
      }
    }
    depIter = new LocSetIterator(depSet);
 
    // if the Unknown Loc is in the indep set then it is better (and still 
    // conservatively correct) to estimate with the USE set for the procedure
    for ( indepIter->reset(); indepIter->isValid(); (*indepIter)++ ) {
        if (debug) { 
          std::cout << "indepIter->current() = "; 
          indepIter->current()->dump(std::cout,mIR);
          std::cout << std::endl;
        }
        if (indepIter->current()->isaUnknown()) {
            indepIter = interSE->getUSEIterator(proc);
            break;
        }
    }
    if (debug) {
      std::cout << "\tManagerEachActive, Indep locations: " << std::cout;
      for ( indepIter->reset(); indepIter->isValid(); (*indepIter)++) {
        indepIter->current()->dump(std::cout,mIR);
        //std::cout << mIR->toString(indepIter->current()) << " ";
      }
      std::cout << "----" << std::endl;
    }
    
    // if the Unknown Loc is in the dep set then it is better (and still 
    // conservatively correct) to estimate with the MOD set for the procedure
    for ( depIter->reset(); depIter->isValid(); (*depIter)++ ) {
        if (depIter->current()->isaUnknown()) {
            depIter = interSE->getMODIterator(proc);
            break;
        }
    }
    if (debug) {
      std::cout << "\tManagerEachActive, Dep locations: " << std::cout;
      for ( depIter->reset(); depIter->isValid(); (*depIter)++) {
        depIter->current()->dump(std::cout,mIR);
        //std::cout << mIR->toString(depIter->current()) << " ";
      }
      std::cout << "----" << std::endl;
    }
    
    assert(0); // this is broken as of 4/4/05
    //OA_ptr<ActiveStandard> active = activeman->performAnalysis(proc,
    //                                    cfg,alias,interSE, indepIter, depIter);

    // put activity results in InterMPICFGActivity
    //retval->mapProcToActive(proc,active);

  }
 
  return retval;
}
 

  } // end of namespace Activity
} // end of namespace OA

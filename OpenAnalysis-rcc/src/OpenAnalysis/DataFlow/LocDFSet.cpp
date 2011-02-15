      
/*! \file
  
  \brief DataflowSet that consists of a set of Locations.

  \authors Michelle Strout 
  \version $Id: LocDFSet.cpp,v 1.4 2005/07/01 02:49:57 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "LocDFSet.hpp"
#include <Utils/Util.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace DataFlow {

static bool debug = false;

LocDFSet::LocDFSet() 
  : mBaseLocToSetMapValid(true), mHasUnknownLoc(false)
{ 
    OA_DEBUG_CTRL_MACRO("DEBUG_LocDFSet:ALL", debug);

    mSetPtr = new LocSet; 

    mInvLocs = new LocSet;
}

LocDFSet::LocDFSet(const LocDFSet &other) 
    : mBaseLocToSetMapValid(other.mBaseLocToSetMapValid), 
      mHasUnknownLoc(other.mHasUnknownLoc)
{   mSetPtr = new LocSet; 
    *mSetPtr = *(other.mSetPtr);
    mInvLocs = new LocSet;
    *mInvLocs = *(other.mInvLocs);

    mBaseLocToSetMapValid = false; // recreate map if/when needed
}


OA_ptr<DataFlowSet> LocDFSet::clone()
{ 
  OA_ptr<LocDFSet> retval;
  retval = new LocDFSet(*this); 
  return retval; 
}
  
// param for these can't be const because will have to 
// dynamic cast to specific subclass
bool LocDFSet::operator ==(DataFlowSet &other) const
{ 
    LocDFSet& recastOther 
        = dynamic_cast<LocDFSet&>(other);
    return *mSetPtr == *(recastOther.mSetPtr); 
}

bool LocDFSet::operator !=(DataFlowSet &other) const
{ 
    LocDFSet& recastOther 
        = dynamic_cast<LocDFSet&>(other);
    return *mSetPtr != *(recastOther.mSetPtr); 
}

LocDFSet& LocDFSet::setUnion(DataFlowSet &other)
{ 
    LocDFSet& recastOther 
        = dynamic_cast<LocDFSet&>(other);
    OA_ptr<LocSet> temp; temp = new LocSet;
    std::set_union(mSetPtr->begin(), mSetPtr->end(), 
                   recastOther.mSetPtr->begin(), recastOther.mSetPtr->end(),
                   std::inserter(*temp,temp->end()));
    *mSetPtr = *temp;
    mBaseLocToSetMapValid = false;
    return *this;
}

LocDFSet& LocDFSet::setIntersect(LocDFSet &other)
{ 
    OA_ptr<LocSet> temp; temp = new LocSet;
    std::set_intersection(mSetPtr->begin(), mSetPtr->end(), 
                          other.mSetPtr->begin(), other.mSetPtr->end(),
                          std::inserter(*temp,temp->end()));
    *mSetPtr = *temp;
    mBaseLocToSetMapValid = false;
    return *this;
}

LocDFSet& LocDFSet::setDifference(LocDFSet &other)
{ 
    OA_ptr<LocSet> temp; temp = new LocSet;
    std::set_difference(mSetPtr->begin(), mSetPtr->end(), 
                        other.mSetPtr->begin(), other.mSetPtr->end(),
                        std::inserter(*temp,temp->end()));
    *mSetPtr = *temp;
    mBaseLocToSetMapValid = false;
    return *this;
}

//! converts this set of locations to equivalent set in callerToCallee
//! and returns a newly created set
OA_ptr<LocDFSet> LocDFSet::callerToCallee(ProcHandle caller,
          CallHandle call, ProcHandle callee,
          OA_ptr<Alias::InterAliasInterface> interAlias,
          OA_ptr<ParamBindings> paramBind,
          OA_ptr<CalleeToCallerVisitorIRInterface> ir)
{
  OA_ptr<DataFlow::LocDFSet> retval;
  retval = new DataFlow::LocDFSet();

  if (debug) { 
      std::cout << "callerToCallee" << std::endl;
  }

  // get alias results for caller procedure
  OA_ptr<Alias::Interface> callerAlias = interAlias->getAliasResults(caller);
  // get alias results for callee procedure
  OA_ptr<Alias::Interface> calleeAlias = interAlias->getAliasResults(callee);

  /***********
   * new pseudocode for callerToCallee 7/28/06
   * for each actual exprTree, act_etree 
   *   if there is a MemRefHandle at the top of act_etree put in memref 
   *     for each mre for memref, ir->getMemRefExprIterator 
   *       found_locs = true 
   *       deref_mre = mre clone
   *       while (found_locs) 
   *         found_locs = false 
   *         deref = new Deref of null memref
   *         deref_mre = deref->composeWith(deref_mre) 
   *         query the alias analysis of Caller for maylocs of deref_mre  
   *         for each mayLoc 
   *           found_locs = true 
   *           if mayloc mayOverlaps with anything in our LocDFSet 
   *                                                      (from caller) 
   *             -get mre_formal for mre_actual 
   *             -create as many derefs on the callee side as you did on the 
   *                caller side (just keep calling deref->composeWith on what 
   *                you have so far) to get a final_mre. 
   *             -query the alias analysis of Callee for maylocs of final_mre 
   *             -for each calleeMayLoc 
   *               -add mayloc to callee locDFSet 
   *************
   */

  if (debug) {
    std::cout << "++++++++ LocDFSet::callerToCallee()\n";
    std::cout << "    actual ExprTrees to formal may locations:";
  }
  // iterate over all actual parameter ExprTrees in the call
  OA_ptr<ExprHandleIterator> exprIter;
  exprIter = paramBind->getActualExprHandleIterator(call);
  for (; exprIter->isValid(); (*exprIter)++) {
    ExprHandle expr = exprIter->current();
    OA_ptr<ExprTree> eTree = paramBind->getActualExprTree(expr);
    
    // if there is a MemRefHandle at the top of this ExprTree
    MemRefHandle memref;
    EvalToMemRefVisitor evalVisitor;
    eTree->acceptVisitor(evalVisitor);
    if (debug) { 
      eTree->output(*ir); 
    }
    if ( evalVisitor.isMemRef() ) {
      memref = evalVisitor.getMemRef();
      if (debug) {
        std::cout << "        MemRef: "
                  << ir->toString(memref)
                  << std::endl;
      }
    } else {
      if (debug) {
        std::cout << "        No MemRef at top of ExprTree\n";
      }
      continue;
    }
    
    SymHandle sym_formal = paramBind->getCalleeFormal(call,memref,callee);
    if (debug) {
      std::cout << "            formal: " << ir->toString(sym_formal)
                << std::endl;
    }

    // for each MemRefExpr of this MemRefHandle
    OA_ptr<MemRefExprIterator> mreIter;
    mreIter = ir->getMemRefExprIterator(memref);
    for (; mreIter->isValid(); (*mreIter)++) {
      OA_ptr<MemRefExpr> mre_actual = mreIter->current();

      OA_ptr<MemRefExpr> deref_mre = mre_actual->clone(); //priming the pump
      bool found_locs = true;
      int numDerefs = 0;

      // loop to find all locations (on the callee side) that may overlap
      // with this actual mre (from the caller side)
      while (found_locs) {
        found_locs = false;

        // Deref mre
        OA_ptr<MemRefExpr> nullmre;
        OA_ptr<Deref> deref;
        deref = new Deref(MemRefExpr::USE,nullmre,1);
        deref_mre = deref->composeWith(deref_mre);
        numDerefs++;

        if (debug) {
          std::cout << "\t\tgetting maylocs from caller for deref_mre = ";
          deref_mre->output(*ir);
          std::cout << std::endl;
        }
        // query the alias analysis of Caller for maylocs of deref_mre  
        OA_ptr<LocIterator> mayLocIter;
        mayLocIter = callerAlias->getMayLocs(*deref_mre,caller);
        for (; mayLocIter->isValid(); (*mayLocIter)++) {
          found_locs = true; 
          OA_ptr<Location> loc = mayLocIter->current();
          if (debug) {
            std::cout << "\t\tmayloc for memref, loc = ";
            loc->output(*ir);
          }

          // if loc mayOverlaps with anything in our LocDFSet (from caller)
          if (hasOverlapLoc(loc)) {

            if (debug) {
              std::cout << "\t\t\tcaller LocDFSet hasOverlapLoc(loc) true\n";
            }
            
            // get mre_formal for mre_actual
            OA_ptr<MemRefExpr> mre_final;
            OA_ptr<NamedRef> mre_formal;
            mre_formal = new NamedRef(MemRefExpr::USE, sym_formal);
            mre_final = mre_formal->clone();

            // duplicate the number of derefs on callee side as on caller side
            for (int i = 0; i < numDerefs; i++) {
              // Deref mre
              OA_ptr<MemRefExpr> nullmre;
              OA_ptr<Deref> deref;
              deref = new Deref(MemRefExpr::USE,nullmre,1);
              mre_final = deref->composeWith(mre_final);
            }

            if (debug) {
              std::cout << "\t\tquerying callee with mre_final = ";
              mre_final->output(*ir);
              std::cout << std::endl;
            }

            // query the alias analysis of Callee for maylocs of mre_final 
            OA_ptr<LocIterator> mayLocIter;
            mayLocIter = calleeAlias->getMayLocs(*mre_final,callee);
            for (; mayLocIter->isValid(); (*mayLocIter)++) { 
              OA_ptr<Location> mayloc = mayLocIter->current();
              if (debug) {
                std::cout << "\t\tmayloc for mre_final, loc = ";
                mayloc->output(*ir);
              }
              // add mayloc to callee locDFSet
              retval->insert(mayloc);
            }
          } // end if (hasOverlapLoc(loc))
          else {
            if (debug) { 
              std::cout << "caller LocDFSet hasOverlapLoc(loc) false" << std::endl;
            }
          }

        } // end of foreach caller-mayloc 
      } // end of while (found_locs)
    } // end of foreach mre_actual
  } // end of foreach actual ExprTree in call
    
  return retval;
}

//! converts this set of locations transitively to equivalent set in callee
//! and returns a newly created set
OA_ptr<LocDFSet> LocDFSet::callerToCalleeTransitive(ProcHandle caller, 
          CallHandle call, ProcHandle callee,
          OA_ptr<Alias::InterAliasInterface> interAlias,
          OA_ptr<ParamBindings> paramBind,
          OA_ptr<CalleeToCallerVisitorIRInterface> ir)
{
  OA_ptr<DataFlow::LocDFSet> retval;
  retval = new DataFlow::LocDFSet();
  if (debug) { 
    std::cout << "callerToCalleeTransitive" << std::endl;
  }
  
  // get alias results for caller procedure
  OA_ptr<Alias::Interface> alias = interAlias->getAliasResults(caller);
  
  // iterate over the formal SymHandles of the callee proc
  OA_ptr<SymHandleIterator> symIter = paramBind->getFormalIterator(callee);
  for (; symIter->isValid(); (*symIter)++ ) {
    SymHandle formal = symIter->current();
    
    // get ExprTrees of the actual parameters for the call
    // transitive:  so step into ExprTrees to get all MemRefs
    ExprHandle expr = paramBind->getActualExprHandle(call,formal);
    OA_ptr<ExprTree> eTree = paramBind->getActualExprTree(expr);
    
    // get all memrefs in the actual ExprTree
    MemRefsVisitor eTreeVisitor;
    eTree->acceptVisitor(eTreeVisitor);
    if (debug) { eTree->dump(std::cout,ir); }
    if ( !eTreeVisitor.hasMemRef() ) {
      continue;
    }
    
    OA_ptr<MemRefHandleIterator> memrefIter;
    memrefIter = eTreeVisitor.getMemRefsIterator();
    for (; memrefIter->isValid(); (*memrefIter)++ ) {
      MemRefHandle memref = memrefIter->current();
      if (debug) { 
        std::cout << "\t caller memref = " << ir->toString(memref) 
                  << std::endl;
      }
      
      // loop over may locs for the memref
      OA_ptr<LocIterator> locIter = alias->getMayLocs(memref);
      for ( ; locIter->isValid(); (*locIter)++ ) {
        OA_ptr<Location> loc = locIter->current();
        if (debug) {
          std::cout << "\t\tcaller mayloc for memref, loc = ";
          loc->dump(std::cout);
        }
        if (hasOverlapLoc(loc)) {
          //assert(0);  // see MMS
          //symHandle sym;  // already have the sym handle for formal, above
          //SymHandle sym = paramBind->getCalleeFormal(call,memref);
          OA_ptr<Location> nloc;
          nloc = new NamedLoc(formal,true);
          retval->insert(nloc);
          if (debug) {
            std::cout << "\t\thasOverlapLoc so inserting formal sym = " 
                      << ir->toString(formal) << std::endl;
          }
        }
      } // end of loop over caller maylocs for a memref
    } // end of loop over caller memrefhandles assoc. with formal symhandle
  } // end of loop over formal symhandles
  return retval;
}

//! converts this set of locations to equivalent set in caller
//! and returns a newly created set
OA_ptr<LocDFSet> LocDFSet::calleeToCaller(ProcHandle callee, CallHandle call, 
          ProcHandle caller,
          OA_ptr<Alias::InterAliasInterface> interAlias,
          OA_ptr<ParamBindings> paramBind,
          OA_ptr<CalleeToCallerVisitorIRInterface> ir)
{
    OA_ptr<DataFlow::LocDFSet> retval;
    retval = new DataFlow::LocDFSet();
    if (debug) { 
      std::cout << "calleeToCaller" << std::endl;
    }

    // have visitor convert all locations into aliased locations in
    // the caller
    OA_ptr<DataFlow::CalleeToCallerVisitor> locVisitor; 
    locVisitor = new DataFlow::CalleeToCallerVisitor(callee, call, 
            caller, interAlias, paramBind, ir);

    OA_ptr<LocIterator> locIter = getLocIterator();
    for ( ; locIter->isValid(); (*locIter)++ ) {
        OA_ptr<Location> loc = locIter->current();
        loc->acceptVisitor(*locVisitor);
        OA_ptr<LocIterator> callerLoc = locVisitor->getCallerLocIterator();
        for ( ; callerLoc->isValid(); (*callerLoc)++ ) {
            retval->insert(callerLoc->current());
        }
    }
    // BK: don't need extra loop to find non-locals, handled in 
    // CalleeToCallerVisitor, above
    return retval;
}

//! converts this set of locations transitively to equivalent set in caller
//! and returns a newly created set
OA_ptr<LocDFSet> LocDFSet::calleeToCallerTransitive(ProcHandle callee, 
          CallHandle call, ProcHandle caller,
          OA_ptr<Alias::InterAliasInterface> interAlias,
          OA_ptr<ParamBindings> paramBind,
          OA_ptr<CalleeToCallerVisitorIRInterface> ir)
{
  OA_ptr<DataFlow::LocDFSet> retval;
  retval = new DataFlow::LocDFSet();
  if (debug) { 
    std::cout << "calleeToCallerTransitive" << std::endl;
  }
  
  // get alias results for caller procedure
  OA_ptr<Alias::Interface> alias = interAlias->getAliasResults(caller);
  
  // iterate over the formal SymHandles of the callee proc
  OA_ptr<SymHandleIterator> symIter = paramBind->getFormalIterator(callee);
  for (; symIter->isValid(); (*symIter)++ ) {
    SymHandle formal = symIter->current();
    OA_ptr<Location> formalLoc;
    formalLoc = new NamedLoc(formal,true);
    
    // if callee LocDFSet has any locs that may overlap with formal loc
    if (hasOverlapLoc(formalLoc)) {
      
      // get ExprTrees of the actual parameters for the call
      // transitive:  so step into ExprTrees to get all MemRefs
      ExprHandle expr = paramBind->getActualExprHandle(call,formal);
      OA_ptr<ExprTree> eTree = paramBind->getActualExprTree(expr);
      
      // get all memrefs in the actual ExprTree
      MemRefsVisitor eTreeVisitor;
      eTree->acceptVisitor(eTreeVisitor);
      if (debug) { eTree->dump(std::cout,ir); }
      if ( !eTreeVisitor.hasMemRef() ) {
        continue;
      }
      
      OA_ptr<MemRefHandleIterator> memrefIter;
      memrefIter = eTreeVisitor.getMemRefsIterator();
      for (; memrefIter->isValid(); (*memrefIter)++ ) {
        MemRefHandle memref = memrefIter->current();
        if (debug) { 
          std::cout << "\t caller memref = " << ir->toString(memref) 
                    << std::endl;
        }
        
        // loop over may locs for the memref
        OA_ptr<LocIterator> locIter = alias->getMayLocs(memref);
        for ( ; locIter->isValid(); (*locIter)++ ) {
          OA_ptr<Location> loc = locIter->current();
          if (debug) {
            std::cout << "\t\tcaller mayloc for memref, loc = ";
            loc->dump(std::cout);
          }
          // insert mayloc into caller LocDFSet
          retval->insert(loc);
        }
      }
    } // end of if (hasOverlapLoc)
  }// end of loop over formal symhandles
  return retval;
}

//! Identifies the residual locations within this set that cannot be 
//! converted to an equivalent set in callee.
//! and returns a newly created set of those residual locations.
//! Since this needs an IRInterface, probably only callable from within
//! a Manager
OA_ptr<LocDFSet> LocDFSet::callerToCalleeNot(ProcHandle caller,
          CallHandle call, ProcHandle callee,
          OA_ptr<Alias::InterAliasInterface> interAlias,
          OA_ptr<ParamBindings> paramBind,
          OA_ptr<CalleeToCallerVisitorIRInterface> ir)
{

  LocDFSet killSet;

  OA_ptr<LocDFSet> retval;

  // get alias results for caller procedure
  OA_ptr<Alias::Interface> callerAlias = interAlias->getAliasResults(caller);
  // get alias results for callee procedure
  OA_ptr<Alias::Interface> calleeAlias = interAlias->getAliasResults(callee);

  if (debug) {
    std::cout << "++++++++ LocDFSet::callerToCalleeNot()\n";
    std::cout << "    actual ExprTrees to formal may locations:";
  }
  // iterate over all actual parameter ExprTrees in the call
  OA_ptr<ExprHandleIterator> exprIter;
  exprIter = paramBind->getActualExprHandleIterator(call);
  for (; exprIter->isValid(); (*exprIter)++) {
    ExprHandle expr = exprIter->current();
    OA_ptr<ExprTree> eTree = paramBind->getActualExprTree(expr);
    
    // if there is a MemRefHandle at the top of this ExprTree
    MemRefHandle memref;
    EvalToMemRefVisitor evalVisitor;
    eTree->acceptVisitor(evalVisitor);
    if (debug) { 
      eTree->output(*ir); 
    }
    if ( evalVisitor.isMemRef() ) {
      memref = evalVisitor.getMemRef();
      if (debug) {
        std::cout << "        MemRef: "
                  << ir->toString(memref)
                  << std::endl;
      }
    } else {
      if (debug) {
        std::cout << "        No MemRef at top of ExprTree\n";
      }
      continue;
    }
    
    SymHandle sym_formal = paramBind->getCalleeFormal(call,memref,callee);
    if (debug) {
      std::cout << "            formal: " << ir->toString(sym_formal)
                << std::endl;
    }

    // for each MemRefExpr of this MemRefHandle
    OA_ptr<MemRefExprIterator> mreIter;
    mreIter = ir->getMemRefExprIterator(memref);
    for (; mreIter->isValid(); (*mreIter)++) {
      OA_ptr<MemRefExpr> mre_actual = mreIter->current();

      OA_ptr<MemRefExpr> deref_mre = mre_actual->clone(); //priming the pump
      bool found_locs = true;
      int numDerefs = 0;

      // loop to find all locations (on the callee side) that may overlap
      // with this actual mre (from the caller side)
      while (found_locs) {
        found_locs = false;

        // Deref mre
        OA_ptr<MemRefExpr> nullmre;
        OA_ptr<Deref> deref;
        deref = new Deref(MemRefExpr::USE,nullmre,1);
        deref_mre = deref->composeWith(deref_mre);
        numDerefs++;

        if (debug) {
          std::cout << "\t\tgetting maylocs from caller for deref_mre = ";
          deref_mre->output(*ir);
          std::cout << std::endl;
        }
        // query the alias analysis of Caller for maylocs of deref_mre  
        OA_ptr<LocIterator> mayLocIter;
        mayLocIter = callerAlias->getMayLocs(*deref_mre,caller);
        for (; mayLocIter->isValid(); (*mayLocIter)++) {
          found_locs = true; 
          OA_ptr<Location> loc = mayLocIter->current();
          if (debug) {
            std::cout << "\t\tmayloc for memref, loc = ";
            loc->output(*ir);
          }

          // if loc mayOverlaps with anything in our LocDFSet (from caller)
          if (hasOverlapLoc(loc)) {

            if (debug) {
              std::cout << "\t\t\tcaller LocDFSet hasOverlapLoc(loc)\n";
            }
            
            // get mre_formal for mre_actual
            OA_ptr<MemRefExpr> mre_final;
            OA_ptr<NamedRef> mre_formal;
            mre_formal = new NamedRef(MemRefExpr::USE,
                                      sym_formal);
            mre_final = mre_formal->clone();

            // duplicate the number of derefs on callee side as on caller side
            for (int i = 0; i < numDerefs; i++) {
              // Deref mre
              OA_ptr<MemRefExpr> nullmre;
              OA_ptr<Deref> deref;
              deref = new Deref(MemRefExpr::USE,nullmre,1);
              mre_final = deref->composeWith(mre_final);
            }

            if (debug) {
              std::cout << "\t\tquerying callee with mre_final = ";
              mre_final->output(*ir);
              std::cout << std::endl;
            }

            // query the alias analysis of Callee for maylocs of mre_final 
            OA_ptr<LocIterator> mayLocIter;
            mayLocIter = calleeAlias->getMayLocs(*mre_final,callee);
            if (mayLocIter->isValid()) {
              // then there was at least one mayLoc added to the callee
              // set for this caller loc:  add mayLocs(loc) to kill set
              OA_ptr<LocIterator> locIter = getOverlapLocIterator(loc);
              for (; locIter->isValid(); (*locIter)++) {
                if (debug) {
                  std::cout << "\t\tinserting loc into kill set: ";
                  (locIter->current())->output(*ir);
                  std::cout << "\n";
                }

                killSet.insert(locIter->current());
              }
            }
          } // end if (hasOverlapLoc(loc))
        } // end of foreach caller-mayloc 
      } // end of while (found_locs)
    } // end of foreach mre_actual
  } // end of foreach actual ExprTree in call

  if (debug) {
    std::cout << "\n\t\tkill Set:";
    killSet.output(*ir);
    std::cout << "\n";
  }

  retval = new LocDFSet((*this).setDifference(killSet));
  if (debug) {
    std::cout << "\n\t\tDifference Set: ";
    retval->output(*ir);
    std::cout << "\n";
  }

  return retval;
}

void LocDFSet::dump(std::ostream &os)
{
  os << "\nLocDFSet: mapValid = " << mBaseLocToSetMapValid << "\n\t mSet = ";
  // iterate over IRHandle's and have the IR print them out
  OA_ptr<LocIterator> locIter = getLocIterator();
  for ( ; locIter->isValid(); (*locIter)++ ) {
    OA_ptr<Location> loc = locIter->current();
    loc->dump(os);
    os << ", ";
  }
  os << std::endl;
  os << "\tmInvLocs = ";
  LocSet::iterator lIter;
  lIter = (*mInvLocs).begin();
  for ( ; lIter!=(*mInvLocs).end(); lIter++ ) {
    OA_ptr<Location> loc = *lIter;
    loc->dump(os);
    os << ", ";
  }
  os << std::endl;
  os << "\tmBaseLoc to SetMap: ";
  std::map<IRHandle,OA_ptr<LocSet> >::iterator mapIter;
  mapIter = mBaseLocToSetMap.begin();
  for ( ; mapIter!=mBaseLocToSetMap.end(); mapIter++) {
    OA_ptr<LocSet> lset = mapIter->second;
    //os << "\t\t symhandle: " << mapIter->first;
    os << "\t\t\t loc set for sym: ";
    if (lset.ptrEqual(0)) {
      os << "   empty set\n";
    } else {
      lIter = (*lset).begin();
      for ( ; lIter!=(*lset).end(); lIter++ ) {
        OA_ptr<Location> loc = *lIter;
        loc->dump(os);
        os << ", ";
      }
    }
  }
  os << "\nend of LocDFSet dump --------------\n\n";
}



void LocDFSet::dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> ir)
{
    os << "\nLocDFSet: mapValid = " << mBaseLocToSetMapValid << "\n\t mSet = ";
    // iterate over IRHandle's and have the IR print them out
    OA_ptr<LocIterator> locIter = getLocIterator();
    for ( ; locIter->isValid(); (*locIter)++ ) {
        OA_ptr<Location> loc = locIter->current();
        loc->dump(os,ir);
        os << ", ";
    }
    os << std::endl;
    os << "\tmInvLocs = ";
    LocSet::iterator lIter;
    lIter = (*mInvLocs).begin();
    for ( ; lIter!=(*mInvLocs).end(); lIter++ ) {
      OA_ptr<Location> loc = *lIter;
      loc->dump(os);
      os << ", ";
    }
    os << std::endl;
    os << "\tmBaseLoc to SetMap: ";
    std::map<IRHandle,OA_ptr<LocSet> >::iterator mapIter;
    mapIter = mBaseLocToSetMap.begin();
    for ( ; mapIter!=mBaseLocToSetMap.end(); mapIter++) {
      OA_ptr<LocSet> lset = mapIter->second;
      //os << "\t\t symhandle: " << ir->toString(mapIter->first);
      os << "\t\t\t loc set for sym: ";
      if (lset.ptrEqual(0)) {
        os << "   empty set\n";
      } else {
        lIter = (*lset).begin();
        for ( ; lIter!=(*lset).end(); lIter++ ) {
          OA_ptr<Location> loc = *lIter;
          loc->dump(os);
          os << ", ";
        }
      }
    }
    os << "\nend of LocDFSet dump --------------\n\n";
}

void LocDFSet::output(OA::IRHandlesIRInterface& ir)
{
  // iterate over IRHandle's and have the IR print them out
  sOutBuild->listStart();
  OA_ptr<LocIterator> locIter = getLocIterator();
  for ( ; locIter->isValid(); (*locIter)++ ) {
    OA_ptr<Location> loc = locIter->current();
    sOutBuild->listItemStart();
      loc->output(ir);
      //      ostringstream separator;
      //      separator << "," << indt;
      //      sOutBuild->outputString(separator.str());
    sOutBuild->listItemEnd();
  }
  sOutBuild->listEnd();
}

void 
LocDFSet::associateWithBaseHandle(IRHandle baseHandle, OA_ptr<Location> loc)
{
    if (mBaseLocToSetMap[baseHandle].ptrEqual(0)) {
        mBaseLocToSetMap[baseHandle] = new LocSet;
    }
    mBaseLocToSetMap[baseHandle]->insert(loc);
}

// NOTE: can't use a visitor because will be inserting loc into
// sets that want OA_ptrs.  More compact this way anyway.
void LocDFSet::addToMap(OA_ptr<Location> loc)
{
    // get the base location
    OA_ptr<Location> baseLoc = loc->getBaseLoc();

    // Named Location
    if (baseLoc->isaNamed()) {
        OA_ptr<NamedLoc> namedLoc = baseLoc.convert<NamedLoc>();
        associateWithBaseHandle(namedLoc->getSymHandle(), loc);

        OA_ptr<SymHandleIterator> symIter = namedLoc->getFullOverlapIter();
        for ( ; symIter->isValid(); (*symIter)++ ) {
            associateWithBaseHandle(symIter->current(), loc);
        }
        symIter = namedLoc->getPartOverlapIter();
        for ( ; symIter->isValid(); (*symIter)++ ) {
            associateWithBaseHandle(symIter->current(), loc);
        }

    // UnnamedLoc
    } else if (baseLoc->isaUnnamed()) {
        OA_ptr<UnnamedLoc> unnamedLoc = baseLoc.convert<UnnamedLoc>();
        associateWithBaseHandle(unnamedLoc->getExprHandle(), loc);

    // InvisibleLoc
    } else if (baseLoc->isaInvisible()) {
        mInvLocs->insert(loc);

    // UnknownLoc
    } else if (baseLoc->isaUnknown()) {
        mHasUnknownLoc = true;
    }
 
}

void LocDFSet::insert(OA_ptr<Location> loc) 
{ 
    if ( ! hasLoc(loc) ) {
        mSetPtr->insert(loc); 
        addToMap(loc);
    }
}


void 
LocDFSet::disassociateWithBaseHandle(IRHandle baseHandle, OA_ptr<Location> loc)
{
    if (!mBaseLocToSetMap[baseHandle].ptrEqual(0)) {
        mBaseLocToSetMap[baseHandle]->erase(loc);
    }
}

void LocDFSet::removeFromMap(OA_ptr<Location> loc)
{
    // get the base location
    OA_ptr<Location> baseLoc = loc->getBaseLoc();

    // Named Location
    if (baseLoc->isaNamed()) {
        OA_ptr<NamedLoc> namedLoc = baseLoc.convert<NamedLoc>();
        disassociateWithBaseHandle(namedLoc->getSymHandle(), loc);

        OA_ptr<SymHandleIterator> symIter = namedLoc->getFullOverlapIter();
        for ( ; symIter->isValid(); (*symIter)++ ) {
            disassociateWithBaseHandle(symIter->current(), loc);
        }
        symIter = namedLoc->getPartOverlapIter();
        for ( ; symIter->isValid(); (*symIter)++ ) {
            disassociateWithBaseHandle(symIter->current(), loc);
        }

    // UnnamedLoc
    } else if (baseLoc->isaUnnamed()) {
        OA_ptr<UnnamedLoc> unnamedLoc = baseLoc.convert<UnnamedLoc>();
        disassociateWithBaseHandle(unnamedLoc->getExprHandle(), loc);

    // InvisibleLoc
    } else if (baseLoc->isaInvisible()) {
        mInvLocs->erase(loc);

    // UnknownLoc
    } else if (baseLoc->isaUnknown()) {
        mHasUnknownLoc = false;
    }
}

void LocDFSet::remove(OA_ptr<Location> loc) 
{ 
    if ( hasLoc(loc) ) {
        mSetPtr->erase(loc); 
        removeFromMap(loc);
    }
}

//! whether or not the set is empty
bool LocDFSet::empty() { return mSetPtr->empty(); }

void LocDFSet::updateMap()
{
    mBaseLocToSetMap.clear();

    OA_ptr<LocIterator> locIter = getLocIterator();
    for ( ; locIter->isValid(); (*locIter)++ ) {
        addToMap(locIter->current());
    }

    mBaseLocToSetMapValid = true;

}

//! whether the give loc overlaps with anything in the given set
bool LocDFSet::setHasOverlapLoc(OA_ptr<Location> loc, OA_ptr<LocSet> aSet)
{
    LocSet::iterator locIter;

    if (! aSet.ptrEqual(0) ) {
      for (locIter=aSet->begin(); locIter!=aSet->end(); locIter++) {
        OA_ptr<Location> temp = *locIter;
        if (loc->mayOverlap(*temp)) {
            return true;
        }
      }
    }

    return false;
}


//! whether or not the set contains a location that overlaps with
//! the given location
bool LocDFSet::hasOverlapLoc(OA_ptr<Location> loc) 
{ 
    // make sure the map is up-to-date
    if (!mBaseLocToSetMapValid) {
        updateMap();
    }

    if (mHasUnknownLoc) {
        return true;
    }
       
    std::map<IRHandle,OA_ptr<LocSet> >::iterator mapIter;

    // get the base location
    OA_ptr<Location> baseLoc = loc->getBaseLoc();

    // Named Location
    if (baseLoc->isaNamed()) {
        OA_ptr<NamedLoc> namedLoc = baseLoc.convert<NamedLoc>();
        bool result = false;
        std::map<IRHandle,OA_ptr<LocSet> >::iterator mapIter;
        mapIter = mBaseLocToSetMap.find(namedLoc->getSymHandle());
        if ( mapIter !=  mBaseLocToSetMap.end()) {
          result = setHasOverlapLoc(loc, mapIter->second);
        }
        return result;

    // UnnamedLoc
    } else if (baseLoc->isaUnnamed()) {
        OA_ptr<UnnamedLoc> unnamedLoc = baseLoc.convert<UnnamedLoc>();
        bool result = false;
        mapIter = mBaseLocToSetMap.find(unnamedLoc->getExprHandle());
        if ( mapIter !=  mBaseLocToSetMap.end()) {
          result = setHasOverlapLoc(loc, mapIter->second);
        }
        return result;

    // InvisibleLoc
    } else if (baseLoc->isaInvisible()) {
        return setHasOverlapLoc(loc, mInvLocs);

    // UnknownLoc
    } else if (baseLoc->isaUnknown()) {
        return true;
    }

}

//! whether the give loc overlaps with anything in the given set
OA_ptr<LocDFSet>  
LocDFSet::overlapLocSet(OA_ptr<Location> loc, OA_ptr<LocSet> aSet)
{
    OA_ptr<LocDFSet> retset;
    retset = new LocDFSet();

    if (! aSet.ptrEqual(0) ) {
        LocSet::iterator locIter;
        for (locIter=aSet->begin(); locIter!=aSet->end(); locIter++) {
            OA_ptr<Location> temp = *locIter;
            if (loc->mayOverlap(*temp)) {
               retset->insert(temp);
            }
        }
    }
    
    return retset;
}

//! iterator over locations in set that overlap with given location
OA_ptr<LocIterator> LocDFSet::getOverlapLocIterator(OA_ptr<Location> loc) 
{ 
    // make sure the map is up-to-date
    if (!mBaseLocToSetMapValid) {
        updateMap();
    }

    OA_ptr<LocDFSet> retset;
    retset = new LocDFSet();

    if (loc->isaUnknown() ) {
        // iterator over all locations in this set
        return getLocIterator();
    }

    // add the unknown location if it is in this set
    if  (mHasUnknownLoc) {
        OA_ptr<Location> unknown;
        unknown = new UnknownLoc();
        retset->insert(unknown);
    }

    // get the base location
    OA_ptr<Location> baseLoc = loc->getBaseLoc();

    // Named Location
    if (baseLoc->isaNamed()) {
        OA_ptr<NamedLoc> namedLoc = baseLoc.convert<NamedLoc>();
        OA_ptr<LocDFSet> tempSet = overlapLocSet(loc, 
                mBaseLocToSetMap[namedLoc->getSymHandle()]) ;
        retset->setUnion( *tempSet );

    // UnnamedLoc
    } else if (baseLoc->isaUnnamed()) {
        OA_ptr<UnnamedLoc> unnamedLoc = baseLoc.convert<UnnamedLoc>();
        OA_ptr<LocDFSet> tempSet = overlapLocSet(loc, 
                mBaseLocToSetMap[unnamedLoc->getExprHandle()]);
        retset->setUnion( *tempSet );

    // InvisibleLoc
    } else if (baseLoc->isaInvisible()) {
        OA_ptr<LocDFSet> tempSet = overlapLocSet(loc, mInvLocs);
        retset->setUnion( *tempSet );
    }

    OA_ptr<LocDFSetIterator> retval;
    retval = new LocDFSetIterator(*retset);
    return retval;
}

//! whether or not the set contains the given location
bool LocDFSet::hasLoc(OA_ptr<Location> loc) 
{ 
    return mSetPtr->find(loc)!=mSetPtr->end(); 
}

//! iterator over locations
OA_ptr<LocIterator> LocDFSet::getLocIterator()
{
    OA_ptr<LocDFSetIterator> retval;
    retval = new LocDFSetIterator(*this);
    return retval;
}


  } // end of DataFlow namespace
} // end of OA namespace


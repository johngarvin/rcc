/*! \file
  
  \brief The AnnotationManager that generates InterReachConsts using ICFGDFSolver.

  \authors Michelle Strout
  \version $Id: ManagerICFGReachConsts.cpp,v 1.1.2.1 2006/02/10 07:57:31 mstrout Exp $

  Copyright (c) 2002-2004, Rice University <br>
  Copyright (c) 2004, University of Chicago <br>  
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>

*/

#include "ManagerICFGReachConsts.hpp"


namespace OA {
  namespace ReachConsts {

static bool debug = false;

/*!
*/
ManagerICFGReachConsts::ManagerICFGReachConsts( 
    OA_ptr<ReachConstsIRInterface> _ir) : mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerICFGReachConsts:ALL", debug);
    mSolver = new DataFlow::ICFGDFSolver(DataFlow::ICFGDFSolver::Forward,*this);
}

OA_ptr<InterReachConsts> 
ManagerICFGReachConsts::performAnalysis(
        OA_ptr<ICFG::ICFGInterface> icfg,
        OA_ptr<DataFlow::ParamBindings> paramBind,
        OA_ptr<Alias::InterAliasInterface> interAlias,
        OA_ptr<SideEffect::InterSideEffectInterface> interSE,
        DataFlow::DFPImplement algorithm)
{
  // store results that will be needed in callbacks
  mICFG = icfg;
  mParamBind = paramBind;
  mInterAlias = interAlias;
  mInterSE = interSE;

  // create an empty InterReachConsts
  mInterReachConsts = new InterReachConsts();

  // call iterative data-flow solver for ICFG
  mSolver->solve(icfg,algorithm);
    
  // assign results for each procedure into InterReachConsts
  std::map<ProcHandle,OA_ptr<ReachConstsStandard> >::iterator mapIter;
  for (mapIter=mReachConstsMap.begin(); mapIter!=mReachConstsMap.end(); 
       mapIter++) 
  {
    mInterReachConsts->mapProcToReachConsts(mapIter->first,mapIter->second);
  }

  return mInterReachConsts;
}

//========================================================
// implementation of ICFGDFProblem interface
//========================================================
//--------------------------------------------------------
// initialization callbacks
//--------------------------------------------------------

/*!
   Data-flow set passed around on the call graph is an
   LocDFSet.  The top value for this is an empty set.
*/
OA_ptr<DataFlow::DataFlowSet> ManagerICFGReachConsts::initializeTop()
{
    OA_ptr<ConstDefSet> retval;
    retval = new ConstDefSet(TOP);
    return retval;
}

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGReachConsts::initializeNodeIN(OA_ptr<ICFG::NodeInterface> n)
{
    OA_ptr<ConstDefSet> retval;
    
    // if node is entry node for whole ICFG
    // FIXME: the size thing is a hack to get around the weird
    // return nodes
    if (n->isAnEntry() && (n->size()==0)) {
        retval  = new ConstDefSet(BOTTOM);
    } else {
        retval = new ConstDefSet(TOP);
    }
    return retval;
}

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGReachConsts::initializeNodeOUT(OA_ptr<ICFG::NodeInterface> n)
{
    return initializeTop();
 }


//--------------------------------------------------------
// solver callbacks 
//--------------------------------------------------------
  
//! OK to modify set1 and return it as result, because solver
//! only passes a tempSet in as set1
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGReachConsts::meet(OA_ptr<DataFlow::DataFlowSet> set1, 
                             OA_ptr<DataFlow::DataFlowSet> set2)
{
  OA_ptr<DataFlow::DataFlowSet> set1clone = set1->clone(); 
  OA_ptr<DataFlow::DataFlowSet> set2clone = set2->clone(); 
  OA_ptr<ConstDefSet> cdset1 = set1clone.convert<ConstDefSet>();
  OA_ptr<ConstDefSet> cdset2 = set2clone.convert<ConstDefSet>();
  // needed to clone them, as we change their contents during meet
    
  if (debug) {
    std::cout << "--------- ManagerICFGReachConsts::meet" << std::endl;
    std::cout << "\tcdset1 = ";
    cdset1->dump(std::cout, mIR);
    std::cout << "\tcdset2 = ";
    cdset2->dump(std::cout, mIR);
  }
  
  /*********************** now done after while loop
  // if either has a default constant value of BOTTOM then
  // the result will have a default constant value of BOTTOM
  // for all locations
  OA_ptr<ConstDefSet> retval; 
  if ((cdset1->getDefaultConstDef() == BOTTOM)
      || (cdset2->getDefaultConstDef() == BOTTOM) ) {
    retval = new ConstDefSet(BOTTOM);
  } else {
    retval = new ConstDefSet(TOP);
  }
  ***********************/
  
  bool changeOccurred = true; // set to true to enable initial pass
  // need to loop over two sets multiple times because there might be locations
  // A and B in set one that overlap each other and only A overlaps location
  // C that is in set two?
  while (changeOccurred) {
    changeOccurred = false; 
    
    // loop through all assignments of locations to lattice values 
    // (either a constant, top, or bottom) in the first set
    for (int i=1; i<=2; i++) {
      OA_ptr<ConstDefSet> set1ptr, set2ptr;
      if (i==1) {
        set1ptr = cdset1; set2ptr = cdset2;
      } else {
        set1ptr = cdset2; set2ptr = cdset1;
      }
      
      ConstDefSetIterator set1Iter(*set1ptr);
      ConstDefSetIterator set2Iter(*set2ptr);
      
      ConstDefSet killSet1;
      ConstDefSet genSet1;
      
      // for each ConstDefLoc in Set1
      for (set1Iter.reset(); set1Iter.isValid(); ++(set1Iter)) {
        OA_ptr<ConstDef> constDef1 = set1Iter.current();
        OA_ptr<Location> cdLocPtr1 = constDef1->getLocPtr();
        OA_ptr<ConstValBasicInterface> cdConstPtr1 
          = constDef1->getConstPtr();
        ConstDefType cdType1 = constDef1->getConstDefType();
        
        // did we find at least one overlap
        bool overlapfound = false;
        OA_ptr<ConstValBasicInterface> nullVal;
        
        ConstDefType defaultVal2 = set2ptr->getDefaultConstDef();
        // conservatively assume that it could mayOverlap with  
        // anything in the second set
        if ((cdType1==BOTTOM) && (defaultVal2 != BOTTOM)) {
          defaultVal2 = BOTTOM;
          set2ptr->setDefaultConstDef(BOTTOM);
          changeOccurred = true;
        }

        // for each ConstDefLoc in Set2
        for (set2Iter.reset(); set2Iter.isValid(); ++(set2Iter)) {
          OA_ptr<ConstDef> constDef2 = set2Iter.current();
          OA_ptr<Location> cdLocPtr2 = constDef2->getLocPtr();
          OA_ptr<ConstValBasicInterface> cdConstPtr2 
            = constDef2->getConstPtr();
          ConstDefType cdType2 = constDef2->getConstDefType();
          
          // find all the locations that must overlap with this location
          // in the second set and perform meets
          if (cdLocPtr1->mustOverlap(*cdLocPtr2)) {
            overlapfound = true;
            if ((cdType1==BOTTOM) || (cdType2==BOTTOM)) {
              cdType1 = BOTTOM; cdConstPtr1 = nullVal;
            } else if ((cdType1==TOP) && (cdType2==TOP)) {
              cdType1 = TOP; cdConstPtr1 = nullVal;
            } else if ((cdType1==TOP) && (cdType2==VALUE)) {
              cdType1 = VALUE;  cdConstPtr1 = cdConstPtr2;
            } else if ((cdType1==VALUE) && (cdType2==TOP)) {
              cdType1 = VALUE;  
            } else if ((cdType1==VALUE) && (cdType2==VALUE)) {
              if (cdConstPtr1 != cdConstPtr2) {
                cdType1 = BOTTOM; cdConstPtr1 = nullVal;
              } // needed else cdType1 can continue as is, no changes
            }
          }
          
          /****************** doing something different here.  
          // if there are any locations that only may overlap, do a meet
          // with their value and the default value for set 2
          // only do this if didn't already find a must overlap
          if (!overlapfound && cdLocPtr1->mayOverlap(*cdLocPtr2)) {
            overlapfound = true;
            if ((cdType1==BOTTOM) || (cdType2==BOTTOM) 
                || (defaultVal2==BOTTOM)) 
              {
                cdType1 = BOTTOM; cdConstPtr1 = nullVal;
              } else if ((cdType1==TOP) && (cdType2==TOP) && 
                         (defaultVal2==TOP)) {
              cdType1 = TOP; cdConstPtr1 = nullVal;
              
              // if neither of the above are true then defaultVal2==TOP
              // and can be ignored
            } else if ((cdType1==TOP) && (cdType2==VALUE)) {
              cdType1 = TOP;  cdConstPtr1 = nullVal;
            } else if ((cdType1==VALUE) && (cdType2==TOP)) {
              cdType1 = VALUE;  
            } 
          }
          *********************/


        } // iterating over set 2
        
        // if there are no locations that may or must overlap, do a meet
        // with the default value for set 2
        if (!overlapfound && defaultVal2==TOP) {
          cdType1 = cdType1;
        } else if (!overlapfound && defaultVal2==BOTTOM) {
          cdType1 = BOTTOM;  cdConstPtr1 = nullVal;
        }

        /****************** replaced with above
        if (!overlapfound) {
          if ((cdType1==BOTTOM) || (defaultVal2==BOTTOM))  {
            cdType1 = BOTTOM; cdConstPtr1 = nullVal;
          }
          // other possibilities result in no changes to cd1
        }
        ******************/
        
        // Check for changes
        if (cdType1 != constDef1->getConstDefType()) {
          changeOccurred = true;
          killSet1.insert(constDef1);
          OA_ptr<ConstDef> cd;
          cd = new ConstDef(cdLocPtr1,cdConstPtr1,cdType1);
          genSet1.insert(cd);
        }

        /*********************** handled below, after while loop
        if (((cdType1==BOTTOM) && (retval->getDefaultConstDef()==BOTTOM))
            || ((cdType1==TOP) && (retval->getDefaultConstDef()==TOP)) )
          {
            // do nothing
          } else {
          // insert the const def
          OA_ptr<ConstDef> cd; 
          cd = new ConstDef(cdLocPtr1, cdConstPtr1, cdType1);
          retval->insert(cd);
        }
        ***********************/

      } // iterating over set 1

      // Update set1
      // Apply Kills
      ConstDefSetIterator setIter(killSet1);
      for (; setIter.isValid(); ++(setIter)) {
        OA_ptr<ConstDef> constDef = setIter.current();
        if (i==1) { // set 1 == cdset1
          if (debug) {
            int k = cdset1->removeANDtell(constDef);
            std::cout << "MEET: killed "<<k<<" from set1: (cd "
                      << constDef->toString(mIR) << ")" << std::endl;
          } else {
            cdset1->remove(constDef);
          }
        } else { // i==2 means set 1 == cdset2
          if (debug) {
            int k = cdset2->removeANDtell(constDef);
            std::cout << "MEET: killed "<<k<<" from set2: (cd "
                      << constDef->toString(mIR) << ")" << std::endl;
          } else {
            cdset2->remove(constDef);
          }
        }
      } // end of Apply Kills

      // Apply Gens
      ConstDefSetIterator vsetIter(genSet1);
      for (; vsetIter.isValid(); ++(vsetIter)) {
        OA_ptr<ConstDef> constDef = vsetIter.current();
        if (i==1) { // set 1 == cdset1
          cdset1->replace(constDef->getLocPtr(),constDef->getConstPtr(),
                          constDef->getConstDefType());
          if (debug) {
            std::cout << "MEET: Val replacement in set1: (cd "
                      << constDef->toString(mIR) << ")" << std::endl;
          }
        } else { // i==2, set 1 == cdset2
          cdset2->replace(constDef->getLocPtr(),constDef->getConstPtr(),
                          constDef->getConstDefType());
          if (debug) {
            std::cout << "MEET: Val replacement in set2: (cd "
                      << constDef->toString(mIR) << ")" << std::endl;
          }
        }
      } // end of Apply Gens
      
    } // doing the comparison both directions

  } // while(changeOccurred)

  // compose retval

  // if either has a default constant value of BOTTOM then
  // the result will have a default constant value of BOTTOM
  // for all locations
  OA_ptr<ConstDefSet> retval;
  ConstDefType retDefaultVal;
  if ((cdset1->getDefaultConstDef() == BOTTOM)
      || (cdset2->getDefaultConstDef() == BOTTOM) ) {
    retDefaultVal = BOTTOM;
  } else {
    retDefaultVal = TOP;
  }
  retval = new ConstDefSet(retDefaultVal);

  // insert cdset1
  ConstDefSetIterator set1Iter(*cdset1);
  for (set1Iter.reset(); set1Iter.isValid(); ++(set1Iter)) {
    OA_ptr<ConstDef> constDef1 = set1Iter.current();
    if (constDef1->getConstDefType() == retDefaultVal) {
      // do nothing
    } else {
      // insert into retval
      OA_ptr<ConstDef> cd; 
      cd = new ConstDef(constDef1->getLocPtr(), 
                        constDef1->getConstPtr(), 
                        constDef1->getConstDefType());
      retval->insert(cd);
    }
  }    
  
  // insert cdset2 (if we insert a ConstDef with the same Location that
  //                already exists in retval, the original remains.
  //                This should not be a problem, as any ConstDefs between
  //                cdset1 and cdset2 with the same Location would
  //                eventually get to the same value in while loop above
  ConstDefSetIterator set2Iter(*cdset2);
  for (set2Iter.reset(); set2Iter.isValid(); ++(set2Iter)) {
    OA_ptr<ConstDef> constDef2 = set2Iter.current();
    if (constDef2->getConstDefType() == retDefaultVal) {
      // do nothing
    } else {
      // insert into retval
      OA_ptr<ConstDef> cd; 
      cd = new ConstDef(constDef2->getLocPtr(), 
                        constDef2->getConstPtr(), 
                        constDef2->getConstDefType());
      retval->insert(cd);
    }
  }
  
  if (debug) {
    std::cout << "\tretval = ";
    retval->dump(std::cout, mIR);
  }
  return retval;
}

/*!
   Transfer function implementation (copied from ManagerReachConstsStandard)

    KILL is the set of reaching const definitions <Loc,Const> 
       where Loc MayOverlap a MayLoc for this statement
       (remember that MustOverlap is a subset of MayOverlap
             and that MustLoc is a subset of MayLoc)
    killREPLACE is the new ConstDef definition for the ConstDef in the KILL set
       These replace the reaching ConstDef with a ConstDefType of BOTTOM. 

    GEN:  When statement expression evaluates to a non-null ConstValBasic,
        GEN is the set of const definitions <Loc,Const> 
        where Loc is a MustLoc and MustOverlap for targets of this statement
        (The loc doesn't have to be in the inSet and it has to be a
         MustLoc.  Loop through all the MustLocs for the MemRefExpr and pair
         them up with the ConstValBasic for the expression. Insert these
         into the inSet, replacing any existing ConstDef for that MustLoc.

 */
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGReachConsts::transfer(ProcHandle proc,
                          OA_ptr<DataFlow::DataFlowSet> in, OA::StmtHandle stmt)
{    
  // if there isn't a ReachConsts for this proc then make one
  if (mReachConstsMap.find(proc)==mReachConstsMap.end()) {
      mReachConstsMap[proc] = new ReachConstsStandard(proc);
  }

  // get alias results for procedure
  OA_ptr<Alias::Interface> alias = mInterAlias->getAliasResults(proc);

  ConstDefSet killSet;
  ConstDefSet replaceSet;
  ConstDefSet genSet;
  OA_ptr<DataFlow::DataFlowSet> inclone = in->clone();
  OA_ptr<ConstDefSet> inRecast = inclone.convert<ConstDefSet>();
  // need another copy because can't iterate over set and update it at same time
  inclone = in->clone();  // need this or get segfault
  OA_ptr<ConstDefSet> inRecastCopy = inclone.convert<ConstDefSet>();

  // set the constant defs for this stmt
  // first remove any old defs
  mReachConstsMap[proc]->resetConstDefSet(stmt,
                                          inRecastCopy->getDefaultConstDef());
  ConstDefSetIterator cdIter(*inRecast);
  for (; cdIter.isValid(); ++(cdIter)) {
    OA_ptr<ConstDef> constDef = cdIter.current();
    mReachConstsMap[proc]->insertConstDef(stmt,constDef,
                                          inRecastCopy->getDefaultConstDef());
  }

  if (debug) {
    std::cout << "--- --- --- --- --- Top of Transfer " << std::endl;
  }

  // 1) Update mapping for all use memrefs for this statement
  setUseMemRef2Const(proc, stmt, *inRecastCopy);

  if (debug) {
      std::cout << "------------------------------------------------"
                << std::endl;
      std::cout << "transfer: stmt = (" << stmt.hval() << ": "
                << mIR->toString(stmt) << ") :" << std::endl;
      std::cout << "\tConstDefsIn: ";
      inRecast->dump(std::cout,mIR);
      std::cout.flush();
  }

  // KILLSET construction algorithm (incl. KILLSET application)
  //--------------------------------
  // for each MemRefHandle of the DefMemRefs for this stmt:
  //   for each MayLoc of this MemRefHandle: 
  //     if the Loc of a ConstDef in inSet MayOverlap the MayLoc:
  //       replace ConstDef in the inSet with 
  //               (ConstDef.getLocPtr(),BOTTOM)
  //--------------------------------
    
  { // beginning of KILLSET creation/application 
    
    // for each def mem ref for this statement
    OA_ptr<MemRefHandleIterator> defIterPtr = mIR->getDefMemRefs(stmt);
    for (; defIterPtr->isValid(); (*defIterPtr)++) {
        MemRefHandle memref = defIterPtr->current();
        
        // for each mayLoc for this mem ref
        OA_ptr<LocIterator> locIterPtr = alias->getMayLocs(memref);
        for (; locIterPtr->isValid(); ++(*locIterPtr)) {
          OA_ptr<Location> mayLocPtr = locIterPtr->current();
          
          // for each ConstDefLoc in inSet
          OA_ptr<ConstDefSetIterator> inIter;
          inIter = new ConstDefSetIterator(*inRecastCopy);
          for (; inIter->isValid(); ++(*inIter)) {
            OA_ptr<ConstDef> constDef = inIter->current();
            OA_ptr<Location> cdLocPtr = constDef->getLocPtr();
            
            // if Loc mayOverLap MayLoc, replace any constDef with cdLocPtr
            // with ConstDef(cdLocPtr,BOTTOM)
            if (cdLocPtr->mayOverlap(*mayLocPtr)) {
              OA_ptr<ConstValBasicInterface> nullVal; nullVal = 0;
              if (inRecast->getDefaultConstDef()!=BOTTOM) {
                inRecast->replace(cdLocPtr,nullVal,BOTTOM);
              } else {
                // just remove it
                inRecast->remove(constDef);
              }
            }
          }
        }
    }

    // loop through all func calls in this statement and what they MOD
    OA_ptr<IRCallsiteIterator> callsiteItPtr = mIR->getCallsites(stmt);
    for ( ; callsiteItPtr->isValid(); ++(*callsiteItPtr)) {
      CallHandle expr = callsiteItPtr->current();

      OA_ptr<LocIterator> locIterPtr;
      // MOD
      locIterPtr = mInterSE->getMODIterator(expr);
      for ( ; locIterPtr->isValid(); (*locIterPtr)++) {
          OA_ptr<Location> modLocPtr = locIterPtr->current();
          
          // for each ConstDefLoc in inSet
          OA_ptr<ConstDefSetIterator> inIter;
          inIter = new ConstDefSetIterator(*inRecastCopy);
          for (; inIter->isValid(); ++(*inIter)) {
            OA_ptr<ConstDef> constDef = inIter->current();
            OA_ptr<Location> cdLocPtr = constDef->getLocPtr();
            
            // if Loc mayOverLap modLoc, replace any constDef with cdLocPtr
            // with ConstDef(cdLocPtr,BOTTOM)
            if (cdLocPtr->mayOverlap(*modLocPtr)) {
              OA_ptr<ConstValBasicInterface> nullVal; nullVal = 0;
              if (inRecast->getDefaultConstDef()!=BOTTOM) {
                inRecast->replace(cdLocPtr,nullVal,BOTTOM);
              } else {
                // just remove it
                inRecast->remove(constDef);
              }
            }
          }
       } 

    }
 
  } // end of KILLSET creation/application 


  // 3) Generate GEN set
  // GENSET construction/application algorithm for EXPR_STMT
  //--------------------------------
  // for each AssignPair -- <MemRefHandle,ExprTree> -- for this stmt:
  //   get ExprConst from ExprTree if it exists
  //   if ExprConst exists:
  //     for each MustLoc of the MemRefHandle
  //       replace any ConstDef(MustLoc) in inSet with
  //                   ConstDef(MustLoc,ExprConst,VALUE)
  //--------------------------------
 
  OA_ptr<AssignPairIterator> espIterPtr 
        = mIR->getAssignPairIterator(stmt);
  for ( ; espIterPtr->isValid(); (*espIterPtr)++) {
     // unbundle pair
     MemRefHandle mref = espIterPtr->currentTarget();
     ExprHandle expr = espIterPtr->currentSource();
        
     // getConstValBasic if possible
     OA_ptr<ConstValBasicInterface> cvbiPtr; cvbiPtr = 0;
     OA_ptr<ExprTree> eTreePtr = mIR->getExprTree(expr);

     // get ConstValBasicInterface by evaluating expression tree
     EvalToConstVisitor evalVisitor(mIR,mReachConstsMap[proc]);
     eTreePtr->acceptVisitor(evalVisitor);
     cvbiPtr = evalVisitor.getConstVal();

     if (!cvbiPtr.ptrEqual(NULL)) { // GEN a constant value
       if (debug) {
        std::cout << "After expr eval, cvbiPtr = (" << cvbiPtr <<")"<<std::endl;
        std::cout.flush();
        std::cout << "           *cvbiPTr = (" << cvbiPtr->toString()
                  << ") found for memRefHandle = <" 
                  << mIR->toString(mref) << ">"
                  << std::endl;
        std::cout.flush();
       }
          
       // for each MustLoc, replace any ConstDef(MustLoc,...) in inSet
       //     with new constant value: ConstDef(MustLoc,cvbiPtr,VALUE)
       OA_ptr<LocIterator> lIterPtr = alias->getMustLocs(mref);
       if (debug) {
          if (!lIterPtr->isValid()) {
            std::cout << "\n:( :( :( But no MustLocs found! :( :(\n\n";
          }
       }
       
       for ( ; lIterPtr->isValid() ; ++(*lIterPtr)) {
          OA_ptr<Location> lPtr = lIterPtr->current();
          inRecast->replace(lPtr,cvbiPtr,VALUE); 
       }
     } // end of "if we have a usable const val"
  } // end of loop over AssignPairList
 
      
  // 4) update reach consts for all mayLocs(???) of the DefMemRefs(stmt)
  setDefMemRef2Const(proc, stmt, *inRecast);
  
  if (debug) {
    std::cout << "\tConstDefsOut: ";
    inRecast->dump(std::cout,mIR);
  }
  
  return inRecast;
}

//! Propagate a data-flow set from caller to callee
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGReachConsts::callerToCallee(ProcHandle caller,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle callee)
{
    OA_ptr<ConstDefSet> inRecast = dfset.convert<ConstDefSet>();
    OA_ptr<ConstDefSet> retval;
    // if the call node in the caller hasn't been visited yet, inRecast
    // will be TOP and we don't want to prematurely set the output to BOTTOM
    if (inRecast->getDefaultConstDef()==TOP) {
        retval = new ConstDefSet(TOP);
    } else {
        retval = new ConstDefSet(BOTTOM);
    }

    if (debug) {
        std::cout << "-------- ManagerICFGReachConsts::callerToCallee" 
                  << "\n\tinRecast = ";
        inRecast->dump(std::cout, mIR);
        std::cout << "\n\tcall = <" << mIR->toString(call) << ">\n\n";
    }

    // get alias results for caller procedure
    OA_ptr<Alias::Interface> callerAlias;
    callerAlias = mInterAlias->getAliasResults(caller);
    // get alias results for callee procedure
    OA_ptr<Alias::Interface> calleeAlias;
    calleeAlias = mInterAlias->getAliasResults(callee);
    
    // iterate over all the formal parameters for the call
    // (because we cannot go from exprHandle to symHandle, just the other
    //  direction)
    OA_ptr<SymHandleIterator> formalIter;
    formalIter = mParamBind->getFormalIterator(callee);
    for (; formalIter->isValid(); (*formalIter)++) {
      SymHandle formal = formalIter->current();
      ExprHandle actual = mParamBind->getActualExprHandle(call,formal);
      OA_ptr<ExprTree> etree = mParamBind->getActualExprTree(actual);
      
      if (debug) {
        std::cout << "\tformal = " << mIR->toString(formal) << "\n";
        std::cout << "\nactual = " << mIR->toString(actual) << "\n";
        std::cout << "\teTree = ";
        etree->output(*mIR);
      }
      
      // get ConstValBasicInterface by evaluating expression tree
      OA_ptr<ConstValBasicInterface> constVal; 
      EvalToConstVisitor evalVisitor(mIR,mReachConstsMap[caller]);
      etree->acceptVisitor(evalVisitor);
      constVal = evalVisitor.getConstVal();
      
      // if the actual parameter evaluates to a constant value
      if (!constVal.ptrEqual(NULL)) {          
        OA_ptr<Location> nloc;
        nloc = new NamedLoc(formal,true);
        OA_ptr<ConstDef> cd; 
        cd = new ConstDef(nloc, constVal, VALUE);
        retval->insert(cd);
        if (debug) {
          std::cout << "\nloc = ";
          nloc->dump(std::cout, mIR);
          std::cout << "\tconstVal = " << constVal->toString() 
                    << std::endl;
        }
      } else {
        // if there is a MemRefHandle at the top of this ExprTree
        MemRefHandle memref;
        EvalToMemRefVisitor evalVisitor;
        etree->acceptVisitor(evalVisitor);
        if (debug) { 
          etree->output(*mIR); 
        }
        if ( evalVisitor.isMemRef() ) {
          memref = evalVisitor.getMemRef();
          if (debug) {
            std::cout << "        MemRef: "
                      << mIR->toString(memref)
                      << std::endl;
          }
        } else {
          if (debug) {
            std::cout << "        No MemRef at top of ExprTree\n";
          }
          continue;
        }
        
        // check for isAddrOf locations with constant values ...

        // for each MemRefExpr of this MemRefHandle
        OA_ptr<MemRefExprIterator> mreIter;
        mreIter = mIR->getMemRefExprIterator(memref);
        for (; mreIter->isValid(); (*mreIter)++) {
          OA_ptr<MemRefExpr> mre_actual = mreIter->current();
          
          // prime the pump
          OA_ptr<MemRefExpr> deref_mre = mre_actual->clone();
          bool found_locs = true;
          int numDerefs = 0;
          
          // loop to find all locations (on the caller side) that may overlap
          // with this actual mre (from the caller side)
          while (found_locs) {
            found_locs = false;
            
            // Deref mre
            OA_ptr<MemRefExpr> nullmre;
            OA_ptr<Deref> deref;
            //deref = new Deref(false,true,MemRefExpr::USE,nullmre,1); 
            
            /* PLM 1/23/07
            deref = new Deref(true,MemRefExpr::USE,nullmre,1);
            */

            deref = new Deref(MemRefExpr::USE,nullmre,1);
            deref_mre = deref->composeWith(deref_mre);
            numDerefs++;

            if (debug) {
              std::cout << "\t\tgetting maylocs from caller for deref_mre = ";
              deref_mre->output(*mIR);
              std::cout << std::endl;
            }
            // query the alias analysis of Caller for maylocs of deref_mre  
            OA_ptr<LocIterator> mayLocIter;
            mayLocIter = callerAlias->getMayLocs(*deref_mre,caller);

            // continue if there are no maylocs
            if (!mayLocIter->isValid()) {
              continue;
            }
            found_locs = true;

            // see if all of these mayLocs have the same constant values
            bool allValuesAgree = true;

            // treat first mayLoc separately
            OA_ptr<Location> loc = mayLocIter->current();
            OA_ptr<ConstDef> cdPtr = inRecast->find(loc);
            (*mayLocIter)++;

            if (cdPtr.ptrEqual(NULL)) {
              // must be TOP or BOTTOM
              allValuesAgree = false; // first location not found in inSet
              if (debug) {
                std::cout << "deref'd mre (";
                deref_mre->output(*mIR);
                std::cout << ") "
                  << "missing ConstDef for first mayLoc ( "
                  << loc->toString(mIR) << ")" << std::endl;
                std::cout.flush();
              }
            } else if (cdPtr->getConstDefType() != VALUE) {
              allValuesAgree = false;
              // first value of mayLoc for this deref memref is not a value
            } 

            // Now check that all other mayLocs agree
            for (; (allValuesAgree) && (mayLocIter->isValid()); 
                 ++(*mayLocIter)) {
              loc = mayLocIter->current();
              OA_ptr<ConstDef> cdNextPtr = inRecast->find(loc);
              if (cdPtr.ptrEqual(NULL)) {
                allValuesAgree = false; // next location not found in inSet
              } else if (!((*cdPtr).equiv(*cdNextPtr))) {
                allValuesAgree = false; // next location not equiv to previous
              }
            }
            if (allValuesAgree) {
              // create similar deref depth on formal parameter
              // get mre_formal for mre_actual
              OA_ptr<MemRefExpr> mre_final;
              OA_ptr<NamedRef> mre_formal;
              //mre_formal = new NamedRef(false,true,MemRefExpr::USE,formal);

              
              /* PLM 1/23/07 deprecated accuracy 
              mre_formal = new NamedRef(true,MemRefExpr::USE,formal);
              */

              mre_formal = new NamedRef(MemRefExpr::USE,formal);
              mre_final = mre_formal->clone();
              
              // duplicate number of derefs on callee side as on caller side
              for (int i = 0; i < numDerefs; i++) {
                // Deref mre
                OA_ptr<MemRefExpr> nullmre;
                OA_ptr<Deref> deref;
                //deref = new Deref(false,true,MemRefExpr::USE,nullmre,1); 

                /* PLM 1/23/07
                deref = new Deref(true,MemRefExpr::USE,nullmre,1);
                */

                deref = new Deref(MemRefExpr::USE,nullmre,1);
                mre_final = deref->composeWith(mre_final);
              }
              
              if (debug) {
                std::cout << "\t\tquerying callee with mre_final = ";
                mre_final->output(*mIR);
                std::cout << std::endl;
              }

              // query the alias analysis of Callee for maylocs of mre_final 
              OA_ptr<LocIterator> mayLocIter;
              mayLocIter = calleeAlias->getMayLocs(*mre_final,callee);
              for (; mayLocIter->isValid(); (*mayLocIter)++) { 
                OA_ptr<Location> mayloc = mayLocIter->current();
                if (debug) {
                  std::cout << "\t\tmayloc for mre_final, loc = ";
                  mayloc->output(*mIR);
                }
                // record the value in cdPtr to mapping for this formal
                OA_ptr<ConstDef> cd;
                cd = new ConstDef(mayloc, cdPtr->getConstPtr(), VALUE);
                retval->insert(cd);
                if (debug) {
                  std::cout << "\tconstVal = " 
                            << (cdPtr->getConstPtr())->toString() 
                            << std::endl;
                }
              } // end mayLocs in callee
            } // end if all values agree
          } // end while found locs
        } // end mre loop
      } // end else check for addrOf MemRef at top of ExprTree
    } // end formal parameter loop


    //commented out previous MustMayActive treatment for above. BK 8/06
    /*    
    OA_ptr<ExprHandleIterator> paramIter 
        = mParamBind->getActualExprHandleIterator(call);
    for (; paramIter->isValid(); (*paramIter)++ ) {
      ExprHandle param = paramIter->current();
      OA_ptr<ExprTree> etree = mParamBind->getActualExprTree(param);
      if (debug) { 
          std::cout << "\teTree = ";
          etree->dump(std::cout, mIR);
      }
      
      // get ConstValBasicInterface by evaluating expression tree
      OA_ptr<ConstValBasicInterface> constVal; 
      EvalToConstVisitor evalVisitor(mIR,mReachConstsMap[caller]);
      etree->acceptVisitor(evalVisitor);
      constVal = evalVisitor.getConstVal();

      // if the parameter evaluates to a constant value
      if (!constVal.ptrEqual(NULL)) { 
          SymHandle sym = mParamBind->getCalleeFormal(call,param);
          OA_ptr<Location> nloc;
          nloc = new NamedLoc(sym,true);
          OA_ptr<ConstDef> cd; 
          cd = new ConstDef(nloc, constVal, VALUE);
          retval->insert(cd);
          if (debug) {
              std::cout << "\nloc = ";
              nloc->dump(std::cout, mIR);
              std::cout << "\tconstVal = " << constVal->toString() 
              << std::endl;
          }
      }
    } 

    // loop over all locations in incoming set and pass through those
    // that are not local
    ConstDefSetIterator setIter(*inRecast);
    
    // for each ConstDefLoc in Set1
    for (setIter.reset(); setIter.isValid(); ++(setIter)) {
        OA_ptr<ConstDef> constDef = setIter.current();
        OA_ptr<Location> cdLocPtr = constDef->getLocPtr();
        OA_ptr<ConstValBasicInterface> cdConstPtr = constDef->getConstPtr();
        ConstDefType cdType = constDef->getConstDefType();

        if (cdLocPtr->isLocal()==false) {
          if (debug) {
              std::cout << "\tinserting non-local = ";
              cdLocPtr->dump(std::cout,mIR);
          }
          OA_ptr<ConstDef> cd; 
          cd = new ConstDef(*constDef);
          retval->insert(cd);
        }
    }
 
    */
    // probably will want to eventually do something with the non-locals, but
    // current, new alias analysis breaks with non-locals coming through.
    // so results are currently more conservative than they might have been
    // in the MustMayActive CVS branch.  BK 8/06

    if (debug) {
        std::cout << "\tretval = ";
        retval->dump(std::cout, mIR);
    }
    return retval;
}
  
//! Propagate a data-flow set from callee to caller
/*!
 Could have the default be top and if a variable is defined
 at all in the callee then it will be set to bottom or to a constant
 then the call to return will just pass along the inset for the call node.
*/
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGReachConsts::calleeToCaller(ProcHandle callee,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle caller)
{
    OA_ptr<ConstDefSet> inRecast = dfset.convert<ConstDefSet>();
    if (debug) {
        std::cout << "-------- ManagerICFGReachConsts::calleeToCaller" 
                  << std::endl;
        std::cout << "\tinRecast = ";
        inRecast->dump(std::cout, mIR);
    }
    OA_ptr<ConstDefSet> calleeRetVal;
    // default will be top
    calleeRetVal = new ConstDefSet(TOP);

    // set all locations that are modified by the callee indirectly
    // or directly to BOTTOM
    OA_ptr<LocIterator> locIterPtr = mInterSE->getMODIterator(callee);
    for ( ; locIterPtr->isValid(); (*locIterPtr)++) {
      OA_ptr<Location> modLocPtr = locIterPtr->current();
      OA_ptr<ConstDef> cd;
      OA_ptr<ConstValBasicInterface> cdConstPtr;
      cd = new ConstDef(modLocPtr, cdConstPtr, BOTTOM);
      calleeRetVal->insert(cd);
    }

    // Then pick up any constant assignments (they can over-ride the BOTTOMs)
    ConstDefSetIterator inIter(*inRecast);
    for (inIter.reset(); inIter.isValid(); ++(inIter)) {
      OA_ptr<ConstDef> constDef = inIter.current();
      calleeRetVal->replace(constDef);
    }

    // have visitor convert all locations into aliased locations in
    // the caller
    OA_ptr<DataFlow::CalleeToCallerVisitor> locVisitor; 
    locVisitor = new DataFlow::CalleeToCallerVisitor(callee, call, 
                              caller, mInterAlias, mParamBind, mIR);
    
    OA_ptr<ConstDefSet> retval;
    // default will be top
    retval = new ConstDefSet(TOP);

    // loop over all locations in calleeRetVal, xform them into aliased
    // locations in the caller, to make retval set
    ConstDefSetIterator setIter(*calleeRetVal);
    for (setIter.reset(); setIter.isValid(); ++(setIter)) {
        OA_ptr<ConstDef> constDef = setIter.current();
        OA_ptr<Location> cdLocPtr = constDef->getLocPtr();
        OA_ptr<ConstValBasicInterface> cdConstPtr = constDef->getConstPtr();
        ConstDefType cdType = constDef->getConstDefType();
        if (debug) {
            std::cout << "\tcallee constDef = ";
            constDef->output(*mIR);
        }

        cdLocPtr->acceptVisitor(*locVisitor);
        OA_ptr<LocIterator> callerLoc = locVisitor->getCallerLocIterator();
        for ( ; callerLoc->isValid(); (*callerLoc)++ ) {
            if (debug) {
              std::cout << "\tGOT a callerLoc = ";
              (callerLoc->current())->output(*mIR);
              std::cout << std::endl;
            }
            if (cdType!=retval->getDefaultConstDef()) {
                OA_ptr<ConstDef> cd; 
                cd = new ConstDef(callerLoc->current(), cdConstPtr, cdType);
                retval->insert(cd);
            }
        }
    }


    if (debug) {
        std::cout << "\tretval = ";
        retval->dump(std::cout, mIR);
    }
    return retval;
}

//! Sets mMemRef2ReachConst[useMemRef] for useMemRefs in given statement
//! ConstDefSet, in, should not be changed
void 
ManagerICFGReachConsts::setUseMemRef2Const(ProcHandle proc, StmtHandle stmt, 
        const ConstDefSet& in) 
{

  // get alias results for procedure
  OA_ptr<Alias::Interface> alias = mInterAlias->getAliasResults(proc);
  if (debug) {
    std::cout << "in setUseMemRef2Const: using alias for "
              << mIR->toString(proc) << "\n";
  }


  // commenting out old code from MustMayActive branch
  // replacing with current technique found in ManagerReachConstsStandard
  // BK 8/06
  /*
  // for each use mem ref in this statement
  OA_ptr<MemRefHandleIterator> useIterPtr = mIR->getUseMemRefs(stmt);
  for (; useIterPtr->isValid(); (*useIterPtr)++) {
    MemRefHandle ref = useIterPtr->current();

    // only use must location for this mem ref to determine if
    // it is a constant
    OA_ptr<LocIterator> locIterPtr = alias->getMustLocs(ref);
    
    // get first location info
    if (! locIterPtr->isValid()) { continue; }
    OA_ptr<Location> loc = locIterPtr->current();
    // if there are more than one must locs then just conservatively move on
    (*locIterPtr)++;  if (locIterPtr->isValid()) { continue; }
    OA_ptr<ConstDef> cdPtr = in.find(loc);

    if (cdPtr.ptrEqual(NULL) || (cdPtr->getConstDefType() != VALUE)) {
      if (debug) {
        std::cout << "a useMemRef (" << mIR->toString(ref) << ") "
                  << "missing ConstDef for loc ( "
                  << loc->toString(mIR) << ")" << std::endl;
        std::cout.flush();
      }
      // record no constant value for this memref
      OA_ptr<ConstValBasicInterface> nullVal;  nullVal = 0;
      mReachConstsMap[proc]->updateReachConst(ref,nullVal);

    // it does have a constant value
    } else {
      mReachConstsMap[proc]->updateReachConst(ref,cdPtr->getConstPtr());
    }

  } // end of useMemRef loop

  */

  // for each use mem ref in this statement
  OA_ptr<MemRefHandleIterator> useIterPtr = mIR->getUseMemRefs(stmt);
  for (; useIterPtr->isValid(); (*useIterPtr)++) {
    MemRefHandle ref = useIterPtr->current();

    // get all may locations for this mem ref
    OA_ptr<LocIterator> locIterPtr = alias->getMayLocs(ref);
   
    if (!locIterPtr->isValid()) {
      continue; // actual parameters (isAddrOf==1) do not have mayLocs
    }

    // set up first location info
    OA_ptr<Location> loc = locIterPtr->current();
    if (debug) {
      std::cout << "First may location (" << loc->toString(mIR) << ")" 
                << std::endl;
      std::cout.flush();
    }

    OA_ptr<ConstDef> cdPtr = in.find(loc);

    // keep track of useful info for this memref
    bool allValuesAgree = true;
    if (cdPtr.ptrEqual(NULL)) {
      // first location not found in inSet ==> must be TOP or BOTTOM
      allValuesAgree = false;
    } else if (cdPtr->getConstDefType() != VALUE) {
      allValuesAgree = false;
      // first value of mayLoc for this memref is not a value
    } 

    // Now check that all other mayLocs agree
    for (; (allValuesAgree) && (locIterPtr->isValid()); ++(*locIterPtr)) {
      OA_ptr<Location> loc = locIterPtr->current();
      OA_ptr<ConstDef> cdNextPtr = in.find(loc);
      if (cdNextPtr.ptrEqual(NULL)) {
        allValuesAgree = false; // next location not found in inSet
      } else if (!((*cdPtr).equiv(*cdNextPtr))) {
        allValuesAgree = false; // next location not equiv to previous
      }
    }
    if (allValuesAgree) {
      // record the value in cdPtr to mapping for this memref
      mReachConstsMap[proc]->updateReachConst(ref,cdPtr->getConstPtr());
    } else {
      // record no constant value for this memref
      OA_ptr<ConstValBasicInterface> nullVal;  nullVal = 0;
      mReachConstsMap[proc]->updateReachConst(ref,nullVal);
    }
  } // end of useMemRef loop


  if (debug) {
    OA_ptr<MemRefHandleIterator> useIterPtr = mIR->getUseMemRefs(stmt);
    for (; useIterPtr->isValid(); (*useIterPtr)++) {
      MemRefHandle ref = useIterPtr->current();
      OA_ptr<ConstValBasicInterface> cvbiPtr 
          = mReachConstsMap[proc]->getReachConst(ref);
      std::cout << "useMemRef(" << mIR->toString(ref) ;
      if (cvbiPtr.ptrEqual(NULL)) {
        std::cout << ") has no constant value." << std::endl;
      } else {
        std::cout  << ") maps to VALUE="
                   << cvbiPtr->toString() << std::endl;
      }
    }
  }
} // end of setUseMemRef2Const()


//! Sets mMemRef2ReachConst[defMemRef] for defMemRefs in given statement
//! ConstDefSet, in, should not be changed
void 
ManagerICFGReachConsts::setDefMemRef2Const(ProcHandle proc, StmtHandle stmt, 
        const ConstDefSet& in) 
{

  // get alias results for procedure
  OA_ptr<Alias::Interface> alias = mInterAlias->getAliasResults(proc);

  // update mReachConstsMap for all mayLocs(???) of the DefMemRefs(stmt)
  OA_ptr<MemRefHandleIterator> defIterPtr = mIR->getDefMemRefs(stmt);
  for (; defIterPtr->isValid(); (*defIterPtr)++) {
    MemRefHandle ref = defIterPtr->current();
    
    // get all may locations for this mem ref
    OA_ptr<LocIterator> locIterPtr = alias->getMayLocs(ref);
    
    // set up first location info
    OA_ptr<Location> loc = locIterPtr->current();
    if (debug) {
      std::cout << "Finding def location (" << loc->toString(mIR) << ")" 
                << std::endl;
      std::cout.flush();
    }
    
    OA_ptr<ConstDef> cdPtr = in.find(loc);
    
    // keep track of useful info for this memref
    bool allValuesAgree = true;
    if (cdPtr.ptrEqual(NULL)) {
      allValuesAgree = false; // first location not found in inSet
      if (debug) {
        std::cout << "a defMemRef (" << mIR->toString(ref) << ") "
                  << "missing ConstDef for first mayLoc ( "
                  << loc->toString(mIR) << ")" << std::endl;
        std::cout.flush();
      }
    } else if (cdPtr->getConstDefType() != VALUE) {
      allValuesAgree = false;
      // first value of mayLoc for this memref is not a value
    } 
    
    // Now check that all other mayLocs agree
    for (; (allValuesAgree) && (locIterPtr->isValid()); ++(*locIterPtr)) {
      OA_ptr<Location> loc = locIterPtr->current();
      OA_ptr<ConstDef> cdNextPtr = in.find(loc);
      if (cdNextPtr.ptrEqual(NULL)) {
        allValuesAgree = false; // next location not found in inSet
      } else if (!((*cdPtr).equiv(*cdNextPtr))) {
        allValuesAgree = false; // next location not equiv to previous
      }
    }
    if (allValuesAgree) {
      // record the value in cdPtr to mapping for this memref
      mReachConstsMap[proc]->updateReachConst(ref,cdPtr->getConstPtr());
    } else {
      // record no constant value for this memref
      OA_ptr<ConstValBasicInterface> nullValue; nullValue = NULL;
      mReachConstsMap[proc]->updateReachConst(ref,nullValue);
    }
  } // end of defMemRef loop
  
  if (debug) {
    OA_ptr<MemRefHandleIterator> defIterPtr = mIR->getDefMemRefs(stmt);
    for (; defIterPtr->isValid(); (*defIterPtr)++) {
      MemRefHandle ref = defIterPtr->current();
      OA_ptr<ConstValBasicInterface> cvbiPtr;
      cvbiPtr = mReachConstsMap[proc]->getReachConst(ref);
      std::cout << "defMemRef(" << mIR->toString(ref);
      if (cvbiPtr.ptrEqual(NULL)) {
        std::cout << ") has no constant value." << std::cout;
      } else {
        std::cout << ") maps to VALUE="
                  << (mReachConstsMap[proc]->getReachConst(ref))->toString() 
                  << std::endl;
      }
    }
  }
} // end of setDefMemRef2Const()

//! Nothing special for exit nodes in a procedure
//! This should never be called since we are a forward analysis
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGReachConsts::exitTransfer(ProcHandle proc, 
                                     OA_ptr<DataFlow::DataFlowSet> out)
{
    return out;
}

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGReachConsts::entryTransfer(ProcHandle proc, 
                                      OA_ptr<DataFlow::DataFlowSet> in)
{
    return in;
}

//! Propagate a data-flow set from call node to return node
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGReachConsts::callToReturn(ProcHandle caller,
                                     OA_ptr<DataFlow::DataFlowSet> dfset, 
                                     CallHandle call, ProcHandle callee)
{
  if (debug) {
    std::cout << "-------- ManagerICFGReachConsts::callToReturn" << std::endl;
  }
  // When this was a C_FLOW_EDGE, we just used the dfset
  // so, just return a clone of the dfset
  
  // BK:  not sure reasoning above is correct.  The whole reason we added
  //      a CALL_RETURN edge was so that it would NOT be treated as a
  //      C_FLOW edge.  
  
  // stopped here BK XXXXX
  
  // create a default TOP set
  //OA_ptr<ConstDefSet> retval;
  //retval = new ConstDefSet(TOP);
  
  // pass on a copy of input
  return dfset->clone();
  
  // why don't we have to do something like (BK): // haven't really looked
  /*
  // create retval as copy of dfset
  OA_ptr<ConstDefSet> remapdfset = dfset.convert<ConstDefSet>();
  OA_ptr<ConstDefSet> retval;
  OA_ptr<DataFlow::DataFlowSet> temp = remapdfset->clone();
  retval = temp.convert<ConstDefSet>();
  return retval;
  */

}


  } // end of namespace ReachConsts
} // end of namespace OA

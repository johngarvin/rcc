/*! \file
  
  \brief The AnnotationManager that generates DepStandard.

  \authors Michelle Strout
  \version $Id: ManagerDepStandard.cpp,v 1.10.6.1 2005/09/19 05:25:17 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerDepStandard.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace Activity {

static bool debug = false;


/*!
*/
ManagerDepStandard::ManagerDepStandard(OA_ptr<ActivityIRInterface> _ir) 
    : mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerDepStandard:ALL", debug);
    mSolver = new DataFlow::CFGDFSolver(DataFlow::CFGDFSolver::Forward,*this);
}

OA_ptr<DataFlow::DataFlowSet> ManagerDepStandard::initializeTop()
{
    OA_ptr<DepDFSet>  retval;
    retval = new DepDFSet;
    return retval;
}

OA_ptr<DataFlow::DataFlowSet> ManagerDepStandard::initializeBottom()
{
    assert(0);
    // would have to know all of the accessible locations
    OA_ptr<DepDFSet>  retval;
    return retval;
}

/*!
    Uses the CFGDFProblem::solve functionality to get In and Out
    Dep set for each basic block.   In transfer function 
    creates a DepDFSet for each stmt and composes that with the In
    DepDFSet to generate the out DepDFSet.
*/
OA_ptr<DepStandard> ManagerDepStandard::performAnalysis(ProcHandle proc, 
        OA_ptr<Alias::Interface> alias,
        OA_ptr<CFG::CFGInterface> cfg, OA_ptr<InterDep> interDep,
        OA_ptr<DataFlow::ParamBindings> paramBind,
        DataFlow::DFPImplement algorithm)
{
  if (debug) {
    std::cout << "In ManagerDepStandard::performAnalysis" << std::endl;
  }
  mDep = new DepStandard();

  // store information for use within callbacks
  mAlias = alias;
  mInterDep = interDep;
  mParamBind = paramBind;

  // store CFG for use in initialization
  mCFG = cfg;
  if (debug) {
  //    cfg->dump(std::cout,mIR);
  }

  // use the dataflow solver to get the In and Out sets for the BBs
//  OA_ptr<DataFlow::DataFlowSet> finalDep = DataFlow::CFGDFSolver::solve(cfg);
  OA_ptr<DataFlow::DataFlowSet> finalDep = mSolver->solve(cfg,algorithm);
  
  if (debug) {
      std::cout << "ManagerDepStandard::performAnalysis, finalDep = ";
      finalDep->dump(std::cout,mIR);
  }
  mDep->mapFinalDeps(finalDep.convert<DepDFSet>());
  
  return mDep;

}

//------------------------------------------------------------------
// Implementing the callbacks for CFGDFProblem
//------------------------------------------------------------------
//void ManagerDepStandard::initializeNode(OA_ptr<CFG::Interface::Node> n)
//{
//    mNodeInSetMap[n] = initializeTop();
//    mNodeOutSetMap[n] = initializeTop();
//}


/*!
 *    Not doing anything special at entries and exits.
 *     */
OA_ptr<DataFlow::DataFlowSet>
ManagerDepStandard::initializeNodeIN(OA_ptr<CFG::NodeInterface> n)
{
     OA_ptr<DataFlow::LocDFSet> retval;
     retval = new DataFlow::LocDFSet;
     return retval;
}

OA_ptr<DataFlow::DataFlowSet>
ManagerDepStandard::initializeNodeOUT(OA_ptr<CFG::NodeInterface> n)
{
      OA_ptr<DataFlow::LocDFSet> retval;
      retval = new DataFlow::LocDFSet;
      return retval;
}



OA_ptr<DataFlow::DataFlowSet> 
ManagerDepStandard::meet (OA_ptr<DataFlow::DataFlowSet> set1orig, 
                           OA_ptr<DataFlow::DataFlowSet> set2orig)
{
    OA_ptr<DepDFSet> set1 = set1orig.convert<DepDFSet>();
    if (debug) {
        std::cout << "ManagerDepStandard::meet" << std::endl;
        std::cout << "\tset1 = ";
        set1->dump(std::cout,mIR);
        std::cout << ", set2 = ";
        set2orig->dump(std::cout,mIR);
    }
       
    DepDFSet retval = set1->setUnion(*set2orig);
    if (debug) {
        std::cout << std::endl << "\tretval set = ";
        retval.dump(std::cout,mIR);
        std::cout << std::endl;
    }
       
    return retval.clone();
}

/*!
    \brief Core of analysis algorithm.  Transfer function for a stmt.

    Create a DepDFSet for this stmt and then compose it with in to get
    out DepDFSet.
*/

OA_ptr<DataFlow::DataFlowSet> 
ManagerDepStandard::transfer(OA_ptr<DataFlow::DataFlowSet> in, 
                              OA::StmtHandle stmt) 
{
    OA_ptr<DepDFSet> inRecast = in.convert<DepDFSet>();
    if (debug) {
        std::cout << "In transfer, stmt(hval=" << stmt.hval() << ")= ";
        mIR->dump(stmt,std::cout);
        std::cout << "\tinRecast = ";
        inRecast->dump(std::cout,mIR);
    }
    
    // new DepDFSet for this stmt
    OA_ptr<DepDFSet> stmtDepDFSet;
    stmtDepDFSet = new DepDFSet;
    
    // set of must defs and differentiable uses for this statement
    LocSet mustDefSet;
    LocSet mayDefSet;
    LocSet diffUseSet;

    // set of expressions to analyze for differentiable uses
    std::set<ExprHandle> exprSet;

    OA_ptr<AssignPairIterator> espIterPtr 
         = mIR->getAssignPairIterator(stmt);
    if(!espIterPtr.ptrEqual(0)) {
        
        for ( ; espIterPtr->isValid(); ++(*espIterPtr)) {
            // unbundle pair
            MemRefHandle mref = espIterPtr->currentTarget();
            ExprHandle expr = espIterPtr->currentSource();
            if (debug) {
                std::cout << "\tmref = " << mIR->toString(mref) << ", ";
                std::cout << "expr = " << mIR->toString(expr) << std::endl;
            }

            // add this to list of expressions we need to analyze
            exprSet.insert(expr);
            if (debug) { 
                std::cout << "Inserting memref = expr into exprSet" 
                          << std::endl;
                OA_ptr<ExprTree> etree = mIR->getExprTree(expr);
                etree->dump(std::cout,mIR); 
            }

            // keep track of def mustlocs
            OA_ptr<LocIterator> locIterPtr = mAlias->getMustLocs(mref);
            for ( ; locIterPtr->isValid(); (*locIterPtr)++ ) {
                mustDefSet.insert(locIterPtr->current());
            }
            // maylocs need to be used for Dep pairs so that we get
            // conservative activity results
            locIterPtr = mAlias->getMayLocs(mref);
            for ( ; locIterPtr->isValid(); (*locIterPtr)++ ) {
                mayDefSet.insert(locIterPtr->current());
            }
        }
    
    // all other statement types just get all uses and defs
    } else {
        if (debug) {
            std::cout << "\tstmt is not EXPR_STMT, stmt = ";
        }
    
        OA_ptr<MemRefHandleIterator> mrIterPtr = mIR->getUseMemRefs(stmt);
        for (; mrIterPtr->isValid(); (*mrIterPtr)++ ) {
            MemRefHandle mref = mrIterPtr->current();
            OA_ptr<LocIterator> locIterPtr = mAlias->getMayLocs(mref);
            for ( ; locIterPtr->isValid(); (*locIterPtr)++ ) {
                diffUseSet.insert(locIterPtr->current());
            }
        }
        mrIterPtr = mIR->getDefMemRefs(stmt);
        for (; mrIterPtr->isValid(); (*mrIterPtr)++ ) {
            MemRefHandle mref = mrIterPtr->current();
            OA_ptr<LocIterator> locIterPtr = mAlias->getMustLocs(mref);
            for ( ; locIterPtr->isValid(); (*locIterPtr)++ ) {
                mustDefSet.insert(locIterPtr->current());
            }
            locIterPtr = mAlias->getMayLocs(mref);
            for ( ; locIterPtr->isValid(); (*locIterPtr)++ ) {
                mayDefSet.insert(locIterPtr->current());
            }
        }
    }

    // iterate over all calls in the statement
    // and union use,def pairs for the call into DepDFSet for
    // the current stmt 
    // also find all expressions that are callsite params and not 
    // reference parameters
    OA_ptr<IRCallsiteIterator> callsiteItPtr = mIR->getCallsites(stmt);
    for ( ; callsiteItPtr->isValid(); ++(*callsiteItPtr)) {
        CallHandle call = callsiteItPtr->current();
        if (debug) {
          std::cout << "\nhandling all callsite params and side-effects, ";
          std::cout << "call = " << mIR->toString(call) << std::endl;
        }

        // get DepDFSet results for this call and put them in results
        // for this stmt, will do meet with procedure DepDFSet at end
        OA_ptr<DepDFSet> callDepDFSet = mInterDep->getDepForCall(call);
        if (debug) { std::cout << "meet involving call DepDFSet" 
                               << std::endl; 
                  //   callDepDFSet->dump(std::cout,mIR);
                   }
        OA_ptr<DataFlow::DataFlowSet> tmp = meet(stmtDepDFSet,callDepDFSet);
        stmtDepDFSet = tmp.convert<DepDFSet>();
        if (debug) {
            std::cout << "\tstmtDepDFSet after meet with callDepDFSet = ";
            //stmtDepDFSet->dump(std::cout, mIR);
        }

        // looping over actual params
        OA_ptr<IRCallsiteParamIterator> cspIterPtr 
            = mIR->getCallsiteParams(call);
        for ( ; cspIterPtr->isValid(); ++(*cspIterPtr)) {
            ExprHandle param = cspIterPtr->current();

            // determine if dealing with a reference parameter
            bool isRefParam = false;
            OA_ptr<ExprTree> eTreePtr = mIR->getExprTree(param);
            EvalToMemRefVisitor evalVisitor;
            eTreePtr->acceptVisitor(evalVisitor);
            if (debug) { eTreePtr->dump(std::cout,mIR); }
            if ( evalVisitor.isMemRef() ) {
                MemRefHandle memref = evalVisitor.getMemRef();
                assert(0); // need to rewrite alg, see MMS
                //if (mParamBind->isRefParam(
                //        mParamBind->getCalleeFormal(call,memref)) )
                //{
                //  isRefParam = true;
               // }
            }

            // record expr for those that aren't reference parameters
            if (isRefParam==false) {
                exprSet.insert(param);
                if (debug) { 
                    std::cout << "Inserting param expr into exprSet" 
                              << std::endl;
                    eTreePtr->dump(std::cout,mIR); 
                }
            } 
        }
    }

    // get differentiable locations from all expressions in stmt
    if (debug) { std::cout << "ExprTree's:" << std::endl; }
    DifferentiableLocsVisitor dlVisitor(mAlias);
    std::set<ExprHandle>::iterator exprIter;
    for (exprIter=exprSet.begin(); exprIter!=exprSet.end(); exprIter++) {
        OA_ptr<ExprTree> etree = mIR->getExprTree(*exprIter);
        if (debug) { etree->dump(std::cout,mIR); }
        etree->acceptVisitor(dlVisitor);
        OA_ptr<LocIterator> locIterPtr 
            = dlVisitor.getDiffLocsIterator();
        for ( ; locIterPtr->isValid(); (*locIterPtr)++ ) {
            diffUseSet.insert(locIterPtr->current());
        }
    }
    
    // store must defs in Dep results as well
    // and remove the associated implicit dep from DepDFSet
    LocSet::iterator useIter, defIter;
    for (defIter=mustDefSet.begin(); defIter!=mustDefSet.end(); defIter++) {
        mDep->insertMustDefForStmt(stmt,*defIter);
        stmtDepDFSet->removeImplicitDep(*defIter,*defIter);
    }
   
    // map all uses to may defs and vice versa for this statement
    // have to do this after removing implicit deps in case it will be
    // explicitly put back in
    OA_ptr<Location> use, def;
    for (useIter=diffUseSet.begin(); useIter!=diffUseSet.end(); useIter++) {
      for (defIter=mayDefSet.begin(); defIter!=mayDefSet.end(); defIter++) {
          stmtDepDFSet->insertDep(*useIter,*defIter);
      }
    }

    // map stmtDepDFSet to stmt in depResults
    mDep->mapStmtToDeps(stmt, stmtDepDFSet);

    //// compose the DepDFSet for this stmt with one coming in
    // make a copy of inRecast so we don't overwrite IN
    OA_ptr<DepDFSet> tmpCopy; tmpCopy = new DepDFSet(*inRecast);
    (*tmpCopy) = tmpCopy->compose(*stmtDepDFSet);
    //OA_ptr<DepDFSet> tmpCopy;
    //tmpCopy = inRecast->compose(stmtDepDFSet);
    if (debug) {
        std::cout << "\tManagerDepStandard::transfer, returning tmpCopy = ";
        tmpCopy->dump(std::cout,mIR);
        std::cout << "\t------ other dump" << std::endl;
        tmpCopy->dump(std::cout);
    }
    return tmpCopy;
}

  } // end of namespace Activity
} // end of namespace OA

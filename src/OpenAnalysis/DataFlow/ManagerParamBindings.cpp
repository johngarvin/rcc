/*! \file
  
  \brief The AnnotationManager that generates ParamBindings.

  \authors Michelle Strout
  \version $Id: ManagerParamBindings.cpp,v 1.5 2005/08/15 19:29:55 utke Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerParamBindings.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace DataFlow {

static bool debug = false;

ManagerParamBindings::ManagerParamBindings(
    OA_ptr<DataFlow::ParamBindingsIRInterface> _ir) : mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerParamBindings:ALL", debug);
}

OA_ptr<ParamBindings> 
ManagerParamBindings::performAnalysis(OA_ptr<CallGraph::CallGraphInterface> callGraph )
{
  // empty set of parameter bindings that we are going to fill
  OA_ptr<ParamBindings> retval;
  retval = new ParamBindings;

  if (debug) {
      std::cout << "In ManagerParamBindings::performAnalysis" << std::endl;
  }

  // for each edge in the call graph get parameter binding
  OA_ptr<CallGraph::EdgesIteratorInterface> 
      edgeIter = callGraph->getCallGraphEdgesIterator();
 
  //! Iterate over callgraph 
  for ( ; edgeIter->isValid(); ++(*edgeIter) ) {
    
    OA_ptr<CallGraph::EdgeInterface> edge = edgeIter->currentCallGraphEdge();

    //! Get Caller and Callee and Call
    OA_ptr<CallGraph::NodeInterface> caller, callee;
    callee = edge->getCallGraphSink();
    caller = edge->getCallGraphSource();
    CallHandle call = edge->getCallHandle();
 
    if(!callee->isDefined()) { continue;  }

    // iterate over parameters at call site
    OA_ptr<IRCallsiteParamIterator> paramIterPtr;
    paramIterPtr = mIR->getCallsiteParams(call);

    for ( ; paramIterPtr->isValid(); (*paramIterPtr)++ ) {
        
        //! Get the actual parameter at the callsite
        ExprHandle param = paramIterPtr->current();
        
        //! get formal associated with callsite param
        SymHandle formal = mIR->getFormalForActual(caller->getProc(), call,
                                                   callee->getProc(), param);
     
        //! Get MemRefExpr for formal
        OA::OA_ptr<OA::MemRefExpr> mre
                    =  mIR->convertSymToMemRefExpr(formal);
 

        //! Get Expression Tree for actual
        OA_ptr<ExprTree> eTreePtr = mIR->getExprTree(param);

        if (debug) {
          std::cout << "Caller : "
                    << mIR->toString(caller->getProc())
                    << std::endl;

          std::cout << "Actual : "
                    << mIR->toString(param)
                    << std::endl;

          std::cout << "Actual ExprTree : " << std::endl;
          mIR->getExprTree(param)->output(*mIR);
          std::cout << std::endl;

          std::cout << "Formal"
                    << mIR->toString(formal)
                    << std::endl;

          std::cout << "Formal MRE :: " << std::endl;
          mre->output(*mIR);
          std::cout << std::endl;          
        }


        assert(mre->isaDeref());

        mre = mre.convert<RefOp>()->getMemRefExpr();    
            
        // map the formal MemRefExpr to the actual ExprHandle
        retval->mapFormalToActualForCall(call, mre, eTreePtr);

    } // loop over parameters at callsite

  }

  return retval;
}

  } // end of namespace DataFlow
} // end of namespace OA

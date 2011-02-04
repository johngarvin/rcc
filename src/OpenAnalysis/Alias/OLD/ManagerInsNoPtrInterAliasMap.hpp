/*! \file

NOTE: deprecated!!  No longer being compiled.
6/2/06, Removed ManagerInsNoPtrInterAliasMap because it is no longer
valid.  It used the CallGraph data-flow analysis framework
and dependended on the isRefParam() call, which has been deprecated.
  
  \brief Declarations of the AnnotationManager that generates an InterAliasMap
         in a context and flow insensitive way and handles pointers 
         conservatively. 

  \authors Michelle Strout
  \version $Id: ManagerInsNoPtrInterAliasMap.hpp,v 1.2 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  Any memory reference involving a dereference is just mapped to the
  UnknownLoc.
*/

#ifndef ManagerInsNoPtrInterAliasMap_H
#define ManagerInsNoPtrInterAliasMap_H

//--------------------------------------------------------------------
#include <cassert>

// OpenAnalysis headers
#include <OpenAnalysis/Alias/InterAliasMap.hpp>
#include <OpenAnalysis/IRInterface/AliasIRInterface.hpp>
#include <OpenAnalysis/DataFlow/CallGraphDFProblemNew.hpp>
#include <OpenAnalysis/DataFlow/ParamBindings.hpp>
#include <OpenAnalysis/Alias/SymAliasSets.hpp>
#include <OpenAnalysis/Alias/ManagerSymAliasSetsTop.hpp>
#include <OpenAnalysis/Alias/ManagerSymAliasSetsBottom.hpp>

namespace OA {
  namespace Alias {

/*! 
    Creates InterAliasMap, which can be queried for an AliasMap for
    each procedure.
*/
class ManagerInsNoPtrInterAliasMap : private DataFlow::CallGraphDFProblemNew
{ 
public:
  ManagerInsNoPtrInterAliasMap(OA_ptr<AliasIRInterface> _ir);
  ~ManagerInsNoPtrInterAliasMap () {}

  OA_ptr<Alias::InterAliasMap> 
  performAnalysis(OA_ptr<CallGraph::Interface> callGraph,
                  OA_ptr<DataFlow::ParamBindings> paramBind);

private:
  //========================================================
  // implementation of CallGraphDFProblemNew callbacks
  //========================================================
  //--------------------------------------------------------
  // initialization callbacks
  //--------------------------------------------------------

  //! Return an initialized top set
  OA_ptr<DataFlow::DataFlowSet>  initializeTop();

  //! Return an initialized bottom set
  OA_ptr<DataFlow::DataFlowSet>  initializeBottom();

  //! Should generate an initial DataFlowSet for a procedure
  OA_ptr<DataFlow::DataFlowSet> initializeNode(ProcHandle proc);
  //! Should generate an initial DataFlowSet, use if for a call if both caller
  //! and callee are defined
  OA_ptr<DataFlow::DataFlowSet> initializeEdge(ExprHandle call, 
                                     ProcHandle caller,
                                     ProcHandle callee);
  //! Should generate an initial DataFlowSet for a call,
  //! called when callee is not defined in call graph and therefore
  //! doesn't have a procedure definition handle
  OA_ptr<DataFlow::DataFlowSet> initializeEdge(ExprHandle call, 
                                     ProcHandle caller,
                                     SymHandle callee);

  //--------------------------------------------------------
  // solver callbacks 
  //--------------------------------------------------------
  
  //! OK to modify set1 and return it as result, because solver
  //! only passes a tempSet in as set1
  OA_ptr<DataFlow::DataFlowSet> meet(OA_ptr<DataFlow::DataFlowSet> set1, 
                           OA_ptr<DataFlow::DataFlowSet> set2); 

  //! What the analysis does for the particular procedure
  OA_ptr<DataFlow::DataFlowSet>
  atCallGraphNode(OA_ptr<DataFlow::DataFlowSet> inSet, OA::ProcHandle proc);

  //! What the analysis does for a particular call
  OA_ptr<DataFlow::DataFlowSet>
  atCallGraphEdge(OA_ptr<DataFlow::DataFlowSet> inSet, OA::ExprHandle call,
                  ProcHandle caller, ProcHandle callee);

  //! translate results from procedure node to callee edge if top-down
  //! or to caller edge if bottom-up
  OA_ptr<DataFlow::DataFlowSet>  nodeToEdge(ProcHandle proc, 
          OA_ptr<DataFlow::DataFlowSet> procDFSet, ExprHandle call);

  //! translate results from caller edge to procedure node if top-down
  //! or from callee edge if bottom-up
  OA_ptr<DataFlow::DataFlowSet>  edgeToNode(ExprHandle call, 
          OA_ptr<DataFlow::DataFlowSet> callDFSet, ProcHandle proc);


private:
  OA_ptr<AliasIRInterface> mIR;
  OA_ptr<InterAliasMap> mInterAliasMap;
  OA_ptr<DataFlow::ParamBindings> mParamBind;
  std::map<ExprHandle,OA_ptr<SymAliasSets> > mCallToDFSet;

};

  } // end of Alias namespace
} // end of OA namespace

#endif

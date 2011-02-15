/*! \file
  
  \brief Declarations of the AnnotationManager that generates a DUGStandard
         from the CFG standards for each procedure

  \authors Michelle Strout
  \version $Id: ManagerDUGStandard.hpp,v 1.2 2005/06/10 02:32:04 mstrout Exp $

  Copyright (c) 2002-2004, Rice University <br>
  Copyright (c) 2004, University of Chicago <br>  
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>

*/

#ifndef DUGManagerStandard_h
#define DUGManagerStandard_h

//--------------------------------------------------------------------
// OpenAnalysis headers
#include <OpenAnalysis/IRInterface/DUGIRInterface.hpp>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>
#include <OpenAnalysis/CFG/EachCFGInterface.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectStandard.hpp>
#include <OpenAnalysis/SideEffect/ManagerInterSideEffectStandard.hpp>
#include <OpenAnalysis/Activity/DepDFSet.hpp>
#include <OpenAnalysis/CSFIActivity/DUGStandard.hpp>
#include <OpenAnalysis/ExprTree/CollectMREVisitor.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExprVisitor.hpp>


/*
#include <queue>
#include <set>
#include <map>
#include <string>
#include <algorithm>
#include <cctype>
*/

namespace OA {
  namespace DUG {

/*! 
   The AnnotationManager for DUGStandard.
   This class can build an DUGStandard, 
   (eventually) read one in from a file, and write one out to a file.
*/
class ManagerDUGStandard { 
public:
  ManagerDUGStandard(OA_ptr<DUGIRInterface> _ir, 
                     OA_ptr<Activity::ActivityIRInterface> _air);
 ~ManagerDUGStandard () {}

  OA_ptr<DUGStandard> performAnalysis( OA_ptr<IRProcIterator>,
                                       OA_ptr<DataFlow::ParamBindings>,
                                       OA_ptr<OA::CallGraph::CallGraphInterface>);

  void transitiveClosureDepMatrix(OA_ptr<OA::CallGraph::CallGraphInterface>);

private: // helper functions

  bool stmt_has_call(StmtHandle stmt);

  void insertEdge(SymHandle, SymHandle, EdgeType, CallHandle, ProcHandle, ProcHandle, ProcHandle);
  void labelCallRetEdges(StmtHandle, ProcHandle);
  void labelUseDefEdges(StmtHandle, ProcHandle);
  void collectIndependentSyms( ProcHandle);
  void collectDependentSyms( ProcHandle);

  void setDepMatrix(ProcHandle, SymHandle, SymHandle);
  void transitiveClosure(ProcHandle proc);
  void edgesBetweenActuals(ProcHandle proc);

  void setDepMatrix4Globals(SymHandle, SymHandle, ProcHandle);
  bool isPathThruOtherProcs(SymHandle, SymHandle, ProcHandle);
  bool isOutgoingToOtherProcs(SymHandle, ProcHandle);
  bool isIncomingFromOtherProcs(SymHandle, ProcHandle);
  bool hasEdgesToOtherProc(SymHandle, ProcHandle);
  bool hasEdgesFromOtherProc(SymHandle, ProcHandle);
  bool isLocal(SymHandle, ProcHandle);



 
  class SymFromMREVisitor : public virtual MemRefExprVisitor {
  public:
    SymFromMREVisitor(OA_ptr<DUGIRInterface> _ir) : mIR(_ir)
    { SymSet = new std::set<SymHandle>(); }

    ~SymFromMREVisitor() {}
    void visitNamedRef(NamedRef& ref)
      {
         SymSet->insert(ref.getSymHandle());
      }

    void visitAddressOf(AddressOf& ref) 
      {  
         OA::OA_ptr<OA::MemRefExpr> mre;
         mre = ref.getMemRefExpr();
         mre->acceptVisitor(*this);
      }

    void visitUnnamedRef(UnnamedRef& ref) 
      {
         ExprHandle expr = ref.getExprHandle();
         OA_ptr<ExprTree> eTreePtr = mIR->getExprTree(expr);
         CollectMREVisitor cVisitor;
         eTreePtr->acceptVisitor(cVisitor);
         //! ======= Collect all use MREs from ExprTree =========
         OA::set<OA_ptr<MemRefExpr> > mSet;
         mSet = cVisitor.getMemRef();
         OA::OA_ptr<std::set<OA_ptr<MemRefExpr> > > mSetPtr;
         mSetPtr = new std::set<OA_ptr<MemRefExpr> >(mSet);
         OA_ptr<MemRefExprIterator> mreIter;
         mreIter = new OA::MemRefExprIterator(mSetPtr);
         for( ; mreIter->isValid(); ++(*mreIter) ) {
             OA::OA_ptr<OA::MemRefExpr> mre;
             mre = mreIter->current();
             mre->acceptVisitor(*this);
         }
      }

    void visitUnknownRef(UnknownRef& ref) { }

    void visitDeref(Deref& ref) 
      { 
         OA::OA_ptr<OA::MemRefExpr> mre;
         mre = ref.getMemRefExpr();
         mre->acceptVisitor(*this);
      }

    // default handling of more specific SubSet specificiations
    void visitSubSetRef(SubSetRef& ref)
      {
         OA::OA_ptr<OA::MemRefExpr> mre;
         mre = ref.getMemRefExpr();
         mre->acceptVisitor(*this);
      }

    OA_ptr<std::set<SymHandle> > getSymSet() { return SymSet; }      

  private:
    OA_ptr<DUGIRInterface> mIR;
    OA_ptr<std::set<SymHandle> > SymSet;
};
  





private: // member variables

  OA_ptr<DUGIRInterface> mIR;
  OA_ptr<Activity::ActivityIRInterface> mActIR;
  OA_ptr<DUGStandard> mDUG;     
  OA_ptr<DataFlow::ParamBindings> mParamBind;

  // dependence matrix
  std::map<ProcHandle, 
           std::map<SymHandle, 
                    std::map<SymHandle, bool> > > mProcToMatrix;
  std::map<ProcHandle, std::set<SymHandle> > mProcToSymSet;
  // just to prevent duplicate edges between the same nodes
  std::map<EdgeType, std::map<SymHandle, 
                              std::map<SymHandle, bool> > > mMatrix;

  // for making edges between actual parameters
  // proc -> call sites that call 'proc'
  std::map<ProcHandle, std::set<CallHandle> > mProcToCallsiteSet;
  // proc -> formal parameters of proc
  std::map<ProcHandle, std::set<SymHandle> > mProcToFormalSet;
  // call site -> (formal -> set of actuals)
  std::map<CallHandle, 
           std::map<SymHandle, 
                    std::set<SymHandle> > > mFormalToActualMap;
  // call site -> proc containing the call site, i.e. the caller
  std::map<CallHandle, ProcHandle> mCallsiteToProc;

  // procedures to be processed. 
  // all procedures reachable from the ones with DEP and INDEP vars
  std::set<ProcHandle> mProcsOfInterest;
};


  } // end of DUG namespace
} // end of OA namespace

#endif

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
#include <OpenAnalysis/Location/Locations.hpp>
#include <OpenAnalysis/Activity/DepDFSet.hpp>
#include <OpenAnalysis/CSFIActivity/DUGStandard.hpp>


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

	//    typedef std::pair<IRSymHandle,ProcHandle> GlobalNode;
	class GlobalNode {
	public:
	    GlobalNode(IRSymHandle i, ProcHandle p, CallHandle c, ProcHandle useP)
		: mIRnSym(i), mProc(p), mCall(c), mUseProc(useP) {};
	    GlobalNode(const GlobalNode& gn)
		: mIRnSym(gn.mIRnSym), mProc(gn.mProc), mCall(gn.mCall), 
		  mUseProc(gn.mUseProc) {};
	    ~GlobalNode(){};

	    bool operator==(const GlobalNode& other) const
		{
		    if (mIRnSym == other.mIRnSym && mProc == other.mProc &&
			mCall == other.mCall && mUseProc == other.mUseProc) 
			return true;
		    return false;
		}

	    bool operator<(const GlobalNode& other) const
		{
		    if (mIRnSym < other.mIRnSym) return true;
		    if (mIRnSym != other.mIRnSym) return false;
		    if (mProc < other.mProc) return true;
		    if (mProc != other.mProc) return false;
		    if (mCall < other.mCall) return true;
		    if (mCall != other.mCall) return false;
		    if (mUseProc < other.mUseProc) return true;
		    return false;
		}

	    IRSymHandle mIRnSym;
	    ProcHandle  mProc;
	    CallHandle  mCall;
	    // procedure of the statement where this global is used. This procedure
	    // can be different when this global is used as an actual parameter. 
	    // In this case, mProc is the callee procedure.
	    ProcHandle  mUseProc;
	};



/*! 
   The AnnotationManager for DUGStandard.
   This class can build an DUGStandard, 
   (eventually) read one in from a file, and write one out to a file.
*/
class ManagerDUGStandard { 
public:
  ManagerDUGStandard(OA_ptr<DUGIRInterface> _ir, 
                     OA_ptr<Activity::ActivityIRInterface> _air,
                     std::map<ProcHandle,OA_ptr<UDDUChains::UDDUChainsStandard> >&);
 ~ManagerDUGStandard () {}

  OA_ptr<DUGStandard> performAnalysis( OA_ptr<IRProcIterator>,
                                       OA_ptr<DataFlow::ParamBindings>,
                                       OA_ptr<OA::CallGraph::CallGraphInterface>);

  void transitiveClosureDepMatrix(OA_ptr<OA::CallGraph::CallGraphInterface>);
  void connectGlobals();
  void printIRSymHandle(IRSymHandle, ostream&, OA_ptr<DUGIRInterface>);
  void printEdge(IRSymHandle, IRSymHandle, EdgeType);
    void printEdgeDot(IRSymHandle, ProcHandle, IRSymHandle, ProcHandle);

private: // helper functions

  bool stmt_has_call(StmtHandle stmt);
  bool isGlobal(SymHandle sym){ 
    return mGlobals.find(sym) != mGlobals.end();
  }
  bool isFormal(SymHandle sym, ProcHandle proc){ 
    return mProcToFormalSet[proc].find(sym) != mProcToFormalSet[proc].end();
  }

  SymHandle getModSymFromMRE(OA_ptr<OA::MemRefExpr>);
  SymHandle getSymFromMRE(OA_ptr<OA::MemRefExpr>);
  void collectDefsUsesInStmt(StmtHandle, ProcHandle);
  std::set<SymHandle>& getUses(StmtHandle, ProcHandle);
  std::set<SymHandle>& getDefs(StmtHandle, ProcHandle);
  std::set<SymHandle>& getCallDefs(StmtHandle, ProcHandle);

  void downwardExposedDefs(SymHandle, StmtHandle, ProcHandle, OA_ptr<UDDUChains::UDDUChainsStandard>);
  void upwardExposedUses(SymHandle, StmtHandle, ProcHandle, OA_ptr<UDDUChains::UDDUChainsStandard>);
  void insertEdge(IRSymHandle, IRSymHandle, EdgeType, CallHandle, ProcHandle, 
		  ProcHandle, ProcHandle);
  void labelCallRetEdges(StmtHandle, ProcHandle);
  void labelUseDefEdges(StmtHandle, ProcHandle);
  void collectIndependentSyms( ProcHandle);
  void collectDependentSyms( ProcHandle);
  void collectFormalParameters(ProcHandle);
  void getCallInfo(StmtHandle, ProcHandle);

  void setDepMatrix(ProcHandle, IRSymHandle, IRSymHandle);
  void transitiveClosure(ProcHandle proc);
  void edgesBetweenFormals(ProcHandle proc);

  void setDepMatrix4Globals(IRSymHandle, IRSymHandle, ProcHandle);
  bool isPathThruOtherProcs(IRSymHandle, IRSymHandle, ProcHandle);
  bool isOutgoingToOtherProcs(IRSymHandle, ProcHandle);
  bool isIncomingFromOtherProcs(IRSymHandle, ProcHandle);
  void findOutgoingNodes(IRSymHandle, ProcHandle, std::set<IRSymHandle>&);
  void findIncomingNodes(IRSymHandle, ProcHandle, std::set<IRSymHandle>&);
    bool hasEdgesToOtherProc(IRSymHandle ish, ProcHandle proc);
    bool hasEdgesFromOtherProc(IRSymHandle ish, ProcHandle proc);

private: // member variables

  OA_ptr<DUGIRInterface> mIR;
  OA_ptr<Activity::ActivityIRInterface> mActIR;
  OA_ptr<DUGStandard> mDUG;     
  OA_ptr<DataFlow::ParamBindings> mParamBind;
  std::map<ProcHandle,OA_ptr<UDDUChains::UDDUChainsStandard> >& mProcToUDDUChainsMap;

  // dependence matrix
  std::map<ProcHandle,std::map<IRSymHandle, std::map<IRSymHandle, bool> > > 
    mProcToMatrix;
  std::map<ProcHandle, std::set<IRSymHandle> > mProcToIRSymSet;
  // just to prevent duplicate edges between the same nodes
  std::map<EdgeType,std::map<IRSymHandle,std::map<IRSymHandle,bool> > > 
    mMatrix;

  // for making edges between actual parameters
  // proc -> call sites that call 'proc'
  std::map<ProcHandle, std::set<CallHandle> > mProcToCallsiteSet;
  // proc -> formal parameters of proc
  std::map<ProcHandle, std::set<SymHandle> > mProcToFormalSet;
  // call site -> (formal -> set of actuals)
  std::map<CallHandle, std::map<IRSymHandle, std::set<IRSymHandle> > > 
    mFormalToActualMap;
  // call site -> proc containing the call site, i.e. the caller
  std::map<CallHandle, ProcHandle> mCallsiteToProc;
  // call -> formal -> set of actuals
  std::map<CallHandle, std::map<SymHandle, std::set<SymHandle> > > mParamMap;
  // call -> set of actuals called by reference
  std::map<StmtHandle, std::set<SymHandle> > mCallByRefActuals;
  std::set<StmtHandle> mCallInfoReady;

  // procedures to be processed. 
  // all procedures reachable from the ones with DEP and INDEP vars
  std::set<ProcHandle> mProcsOfInterest;

  // statements for which the uses are collected
  std::set<StmtHandle> mDefsUsesCollected;
  // statement -> used symbols
  std::map<StmtHandle, std::set<SymHandle> > mStmtToUsedSyms;
  // statement -> defined symbols
  std::map<StmtHandle, std::set<SymHandle> > mStmtToDefdSyms;
  // statement -> symbols defined by call-by-reference function calls
  std::map<StmtHandle, std::set<SymHandle> > mStmtToCallDefdSyms;
  // global variables
  std::set<SymHandle> mGlobals;
  // For connecting global variable nodes
  // symbol -> set of IRSymHandle
  std::map<SymHandle, std::set<GlobalNode> > mGlobalUpUses;
  std::map<SymHandle, std::set<GlobalNode> > mGlobalDnDefs;
};

//! Visitors added by PLM 09/22/06

class IndepLocVisitor : public virtual LocationVisitor {
  public:
      
    IndepLocVisitor(OA_ptr<DUGStandard> dug, 
                    ProcHandle proc, 
                    std::set<ProcHandle> ProcsOfInterest,
                    OA_ptr<DUGIRInterface> ir
                    ) 
                  : mDUG(dug), mProc(proc), mProcsOfInterest(ProcsOfInterest), mIR(ir) 
    { }
    ~IndepLocVisitor() { }

    void visitNamedLoc(NamedLoc& loc);
    void visitUnnamedLoc(UnnamedLoc& loc);
    void visitInvisibleLoc(InvisibleLoc& loc);
    void visitUnknownLoc(UnknownLoc& loc);
    void visitLocSubSet(LocSubSet& loc);
    
    std::set<ProcHandle> getmProcsOfInterest() 
    {
        return mProcsOfInterest;
    }
  private:
    OA_ptr<DUGStandard> mDUG;
    ProcHandle mProc;
    std::set<ProcHandle> mProcsOfInterest;
    OA_ptr<DUGIRInterface> mIR;

};

class depLocVisitor : public virtual LocationVisitor {
  public:
    depLocVisitor(OA_ptr<DUGStandard> dug, 
                  ProcHandle proc,
                  std::set<ProcHandle> ProcsOfInterest ,
                  OA_ptr<DUGIRInterface> ir
                  )
                 : mDUG(dug), mProc(proc), mProcsOfInterest(ProcsOfInterest), mIR(ir)
    { }
    ~depLocVisitor() {}

    void visitNamedLoc(NamedLoc& loc);
    void visitUnnamedLoc(UnnamedLoc& loc);
    void visitInvisibleLoc(InvisibleLoc& loc);
    void visitUnknownLoc(UnknownLoc& loc);
    void visitLocSubSet(LocSubSet& loc);
    std::set<ProcHandle> getmProcsOfInterest()
    {
        return mProcsOfInterest;
    }

  private: 
     OA_ptr<DUGStandard> mDUG;
     ProcHandle mProc; 
     std::set<ProcHandle> mProcsOfInterest;
     OA_ptr<DUGIRInterface> mIR;
};



  } // end of DUG namespace
} // end of OA namespace

#endif

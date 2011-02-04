/*! \file
  
  \brief The AnnotationManager that generates DUG

  \authors Michelle Strout
  \version $Id: ManagerDUGStandard.cpp,v 1.2 2005/06/10 02:32:04 mstrout Exp $

  Copyright (c) 2002-2004, Rice University <br>
  Copyright (c) 2004, University of Chicago <br>  
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>

*/

#include "ManagerDUGStandard.hpp"



#if defined(DEBUG_ALL) || defined(DEBUG_ManagerDUGStandard)
static bool debug = true;
#else
static bool debug = false;
#endif

namespace OA {
    namespace DUG {


/*!
  Visitor over memory reference expressions that creates an
  appropriate Location data structure for any MemRefExpr.
   
  Conservatively handles addressOf (only way UnnamedRefs happen) and 
  derefs.  Therefore resulting mLoc will be UnknownLoc for those.
*/
class CreateLocationVisitor : public virtual MemRefExprVisitor {
public:
    OA_ptr<Location> mLoc;
    CreateLocationVisitor(OA_ptr<DUGIRInterface> ir,
                          ProcHandle proc) : mIR(ir),mProc(proc) {}
    ~CreateLocationVisitor() {}
    void visitNamedRef(NamedRef& ref) 
	{
	    mLoc = mIR->getLocation(mProc,ref.getSymHandle());
	}

    void visitAddressOf(AddressOf& ref) { mLoc = new UnknownLoc(); }
    void visitUnnamedRef(UnnamedRef& ref) { mLoc = new UnknownLoc; }
    void visitUnknownRef(UnknownRef& ref) { mLoc = new UnknownLoc; }
    void visitDeref(Deref& ref) { mLoc = new UnknownLoc; }
    // default handling of more specific SubSet specificiations
    void visitSubSetRef(SubSetRef& ref) 
	{
	    // will set mLoc to our base location
	    ref.getMemRefExpr()->acceptVisitor(*this);
	    if (mLoc->isaNamed()) {
		mLoc = new LocSubSet(mLoc,false);
	    }
	}

private:
    OA_ptr<DUGIRInterface> mIR;
    ProcHandle mProc;

};

ManagerDUGStandard::ManagerDUGStandard(OA_ptr<DUGIRInterface> _ir, 
                                       OA_ptr<Activity::ActivityIRInterface> _air,
                                       std::map<ProcHandle,OA_ptr<UDDUChains::UDDUChainsStandard> >& 
                                       ProcToUDDUChainsMap) 
    : mIR(_ir), mActIR(_air), mProcToUDDUChainsMap(ProcToUDDUChainsMap)

{ 
}


bool ManagerDUGStandard::stmt_has_call(StmtHandle stmt)
{
    bool callflag = false;
    OA_ptr<IRCallsiteIterator> callsiteItPtr = mIR->getCallsites(stmt);
    for ( ; callsiteItPtr->isValid(); ++(*callsiteItPtr)) {
	CallHandle call = callsiteItPtr->current();
	SymHandle sym = mIR->getSymHandle(call);
	ProcHandle proc = mIR->getProcHandle(sym);
#ifdef DEBUG_DUAA
	if (debug) {
	    std::cout << "sym for callee = " 
		      << mIR->toString(sym) << std::endl;
	}
#endif
	if (proc!=ProcHandle(0)) {
	    callflag = true;
	}
    }
    return callflag;
}


void ManagerDUGStandard::printIRSymHandle(IRSymHandle ish,
                                          ostream& os, 
                                          OA_ptr<DUGIRInterface> ir)
{
    IRHandle  ih  = ish.first;
    SymHandle sym = ish.second;
    os << ir->toString(sym) << "@" << ih.hval();
}




void ManagerDUGStandard::printEdge(IRSymHandle from,
                                   IRSymHandle to,
                                   EdgeType edgeType)
{
    static const char *sEdgeTypeToString[] = {
	"CFLOW",
	"CALL",
	"RETURN",
	"PARAM"
    };
    printIRSymHandle(from, std::cout, mIR);
    std::cout << " -> ";
    printIRSymHandle(to, std::cout, mIR);
    std::cout << " (" << sEdgeTypeToString[edgeType] 
	      << ")" << std::endl;
}




/*!
  Creates a DUG for the program
*/
void ManagerDUGStandard::insertEdge( 
    IRSymHandle      from,
    IRSymHandle      to,
    EdgeType         etype,
    CallHandle       expr,
    ProcHandle       fromProc,
    ProcHandle       toProc,
    ProcHandle       proc)
{
    OA_ptr<Node> fromNode = mDUG->getNode(from, fromProc);
    OA_ptr<Node> toNode   = mDUG->getNode(to,   toProc);

    if (fromNode == toNode) {
	fromNode->setSelfDependent();
#ifdef DEBUG_DUAA
	std::cout << "*** insertEdge: head and tail nodes are same:";
	printIRSymHandle(from, std::cout, mIR); 
	std::cout << std::endl; 
#endif
	return;
    }

    // build dependence matrix
    if (etype == CFLOW_EDGE) setDepMatrix(fromProc, from, to);

    // prevent duplicate edges but not for CALL nor RETURN edges
    if ((etype == CFLOW_EDGE || etype == PARAM_EDGE) && 
	mMatrix[etype][from][to]) {
#ifdef DEBUG_DUAA
	std::cout << "*** insertEdge: edge exists:";
	printEdge(from, to, etype);
#endif
	return;
    }
    mMatrix[etype][from][to] = true;

#ifdef DEBUG_DUAA
    std::cout << "insertEdge(inserting): ";
    printEdge(from, to, etype);
#endif

    // edge between 'from' and 'to' node
    OA_ptr<Edge> dugEdge;
    dugEdge = new Edge(mDUG,fromNode, toNode, etype, expr, proc);
    mDUG->addEdge(dugEdge);
}



/*!
  Extract call site information
*/
void ManagerDUGStandard::getCallInfo(StmtHandle stmt, ProcHandle proc)
{
    if (mCallInfoReady.find(stmt) != mCallInfoReady.end()) return;
    mCallInfoReady.insert(stmt);
  
    // for each call to a defined procedure,
    OA_ptr<IRCallsiteIterator> callsiteItPtr;
    callsiteItPtr = mIR->getCallsites(stmt);

    for ( ; callsiteItPtr->isValid(); ++(*callsiteItPtr)) {
	CallHandle call = callsiteItPtr->current();

	SymHandle calleesym = mIR->getSymHandle(call);
	ProcHandle callee = mIR->getProcHandle(calleesym);
	// Undefined procedures are not processed
	if (callee==ProcHandle(0)) { continue; }
#ifdef DEBUG_DUAA
	std::cout << "found a call to " << mIR->toString(callee) << std::endl;
#endif

	// to process only reachable procedures
	mProcsOfInterest.insert(callee);
	mProcToCallsiteSet[callee].insert(call); 
	mCallsiteToProc[call] = proc;

	// iterate over formal parameters for callee procedure
	// iterate over actual parameters at the same time
	OA_ptr<IRCallsiteParamIterator> actualIter;
	actualIter = mIR->getCallsiteParams(call);
	for (int formalCnt=0; actualIter->isValid(); ++(*actualIter), formalCnt++) 
	{
	    // formal
	    SymHandle formalSym = mIR->getFormalSym(callee, formalCnt);
#ifdef DEBUG_DUAA
	    std::cout << "formal:" << mIR->toString(formalSym) << std::endl;
#endif
	    std::set<SymHandle>& actualSet = mParamMap[call][formalSym];

	    // actual
	    ExprHandle param = actualIter->current();
	    OA_ptr<ExprTree> eTreePtr; eTreePtr = mIR->getExprTree(param);
	    ExprTree::NodesIterator nodes_iter(*eTreePtr);

	    for ( ; nodes_iter.isValid(); ++nodes_iter) {
		OA_ptr<ExprTree::Node> exprTreeNode; 
		exprTreeNode = nodes_iter.current();

		if ( exprTreeNode->isaMemRefNode() ) {
		    OA_ptr<ExprTree::MemRefNode> memRefNode;
		    memRefNode = exprTreeNode.convert<ExprTree::MemRefNode>();
		    MemRefHandle memref = memRefNode->getHandle();

#ifdef DEBUG_DUAA
		    std::cout << "isaMemRefNode" << std::endl;
#endif
		    // get the memory reference expressions for this handle
		    OA_ptr<MemRefExprIterator> mreIter;
		    mreIter = mIR->getMemRefExprIterator(memref);
		    // for each mem-ref-expr associated with this memref
		    for (; mreIter->isValid(); (*mreIter)++) {
			OA_ptr<OA::MemRefExpr> mre; mre = mreIter->current();
			SymHandle actualSym = getSymFromMRE(mre);
			if (actualSym == SymHandle(0)) continue;

#ifdef DEBUG_DUAA
			std::cout << "actual:" << mIR->toString(actualSym) << std::endl;
#endif
			mDUG->mapSymToMemRefSet(actualSym, memref);
			mDUG->mapSymToStmtSet(actualSym, stmt);
			// for use in labelCallRetEdges
			actualSet.insert(actualSym);
		    }
		}
	    }
	    // actual symbols defined by call-by-reference 
	    if (actualSet.size() == 1){
		mCallByRefActuals[stmt].insert(*(actualSet.begin()));
	    }
	}
    }
}



/*!
  Creates an DUG for the program
*/
void ManagerDUGStandard::labelCallRetEdges(
    StmtHandle stmt, ProcHandle proc)
{
#ifdef DEBUG_DUAA
    std::cout << "labelCallRetEdges:proc(" << mIR->toString(proc) << "), stmt(" 
	      << stmt.hval() << ")" << std::endl;
#endif
    getCallInfo(stmt, proc);

    // get reaching definitions for 'stmt'
    OA_ptr<UDDUChains::UDDUChainsStandard> UDDUChainForProc;
    UDDUChainForProc = mProcToUDDUChainsMap[proc];

    // for each call to a defined procedure,
    // add a call Edge and return edge
    OA_ptr<IRCallsiteIterator> callsiteItPtr;
    callsiteItPtr = mIR->getCallsites(stmt);
    for ( ; callsiteItPtr->isValid(); ++(*callsiteItPtr)) {
	CallHandle call = callsiteItPtr->current();
	SymHandle calleesym = mIR->getSymHandle(call);
	ProcHandle callee = mIR->getProcHandle(calleesym);
	// Undefined procedures are not processed
	if (callee==ProcHandle(0)) { continue; }

	// iterate over formal parameters for callee procedure
	int formalCnt=0;
	SymHandle formalSym = mIR->getFormalSym(callee, formalCnt);
	for (; formalSym != SymHandle(0); 
	     formalSym = mIR->getFormalSym(callee, ++formalCnt)) 
	{
	    // formal
	    IRSymHandle def(StmtHandle(1),formalSym);

	    // actual
	    std::set<SymHandle>& actualSet = mParamMap[call][formalSym];
	    std::set<SymHandle>::iterator actualIter = actualSet.begin();
	    for ( ; actualIter != actualSet.end(); actualIter++) 
	    {
		SymHandle actualSym = *actualIter;

		// CALL edges
		OA_ptr<StmtHandleIterator> UDStmtIter;
		UDStmtIter = UDDUChainForProc->getUDChainStmtIterator(stmt);
		for (; UDStmtIter->isValid(); (*UDStmtIter)++ ) {
		    StmtHandle rdStmt = UDStmtIter->current();
#ifdef DEBUG_DUAA
		    std::cout << "rdStmt(" << rdStmt.hval() << ")" 
			      << " found" << std::endl;
#endif

		    // upward exposed uses: formals, globals
		    if (rdStmt == StmtHandle(0)){
#ifdef DEBUG_DUAA
			std::cout << "rdStmt(0): actualSym(" << mIR->toString(actualSym);
#endif
			// This is an overestimation. Not all uses of formal parameters
			// may have an upward exposed use.
			if (isFormal(actualSym, proc)){
			    IRSymHandle use(StmtHandle(1),actualSym);// incoming formals
			    insertEdge(use, def, CALL_EDGE, call, proc, callee, proc);
			    mFormalToActualMap[call][def].insert(use);
#ifdef DEBUG_DUAA
			    std::cout << ") is formal.\n"; 
#endif
			}
			else if (isGlobal(actualSym)){
			    mGlobalUpUses[actualSym].insert(GlobalNode(def,callee,call,proc));
#ifdef DEBUG_DUAA
			    std::cout << ") is global.\n"; 
#endif
			}
#ifdef DEBUG_DUAA
			else {
			    std::cout << ") is neither formal nor global.\n"; 
			}
#endif
			continue;
		    }

		    std::set<SymHandle>& rdDefSet = getDefs(rdStmt, proc);
		    std::set<SymHandle>& callDefSet = getCallDefs(rdStmt, proc);
		    if (rdDefSet.find(actualSym) != rdDefSet.end() ||
			callDefSet.find(actualSym) != callDefSet.end() ){
#ifdef DEBUG_DUAA
			std::cout << "rdStmt defines the actualSym" << std::endl;
#endif
			IRSymHandle use(rdStmt,actualSym);
			insertEdge(use, def, CALL_EDGE, call, proc, callee, proc);
			mFormalToActualMap[call][def].insert(use);
		    }
#ifdef DEBUG_DUAA
		    else
			std::cout << "actualSym is not defined by rdDefSet.\n"; 
#endif
		}

		// RETURN edges
		if (actualSet.size() != 1) continue;
#ifdef DEBUG_DUAA
		std::cout << "RETURN_EDGE" << std::endl;
#endif
		IRSymHandle useRet(IRHandle(2),formalSym);
		IRSymHandle defRet(stmt,actualSym);
		insertEdge(useRet, defRet, RETURN_EDGE, call, callee, proc, proc);
		mFormalToActualMap[call][useRet].insert(defRet);

		downwardExposedDefs(actualSym, stmt, proc, UDDUChainForProc);
	    }
	}
    }
}


/*!
  collect independent variables
*/
void
ManagerDUGStandard::collectIndependentSyms( ProcHandle proc)
{
    OA_ptr<MemRefExprIterator> indepIter =mIR->getIndepMemRefExprIter(proc);
    for ( indepIter->reset(); indepIter->isValid(); (*indepIter)++ ) {
      
	OA_ptr<MemRefExpr> memref = indepIter->current();
	if(memref->isaRefOp()) {
	    while(memref->isaRefOp()) {
		OA_ptr<RefOp> refOp = memref.convert<RefOp>();
		memref = refOp->getMemRefExpr();
	    }
	} 

	if(memref->isaNamed()) {
	    OA_ptr<NamedRef> named = memref.convert<NamedRef>(); 
	    SymHandle sym = named->getSymHandle();
	    mDUG->insertIndepSymList(sym, proc);
	    mProcsOfInterest.insert(proc);

	    OA_ptr<Location> loc;loc = mIR->getLocation(proc, sym);
	    if (!loc.ptrEqual(0)){
		OA_ptr<NamedLoc> nloc;nloc = loc.convert<NamedLoc>();
		OA_ptr<SymHandleIterator> symIter = nloc->getFullOverlapIter();
		for ( ; symIter->isValid(); (*symIter)++) {
		    mDUG->insertIndepSymList(symIter->current(), proc);
		}
	    }
	}
    }
}



/*!
  collect dependent variables
*/
void
ManagerDUGStandard::collectDependentSyms( ProcHandle proc)
{
    OA_ptr<MemRefExprIterator> depIter =mIR->getDepMemRefExprIter(proc);
    for ( depIter->reset(); depIter->isValid(); (*depIter)++ ) {

	OA_ptr<MemRefExpr> memref = depIter->current();
	if(memref->isaRefOp()) {
	    while(memref->isaRefOp()) {
		OA_ptr<RefOp> refOp = memref.convert<RefOp>();
		memref = refOp->getMemRefExpr();
	    }
	}

	if(memref->isaNamed()) {
	    OA_ptr<NamedRef> named = memref.convert<NamedRef>();
	    SymHandle sym = named->getSymHandle();
	    mDUG->insertDepSymList(sym, proc);
	    mProcsOfInterest.insert(proc);

	    OA_ptr<Location> loc;loc = mIR->getLocation(proc, sym);
	    if (!loc.ptrEqual(0)){
		OA_ptr<NamedLoc> nloc;nloc = loc.convert<NamedLoc>();
		OA_ptr<SymHandleIterator> symIter = nloc->getFullOverlapIter();
		for ( ; symIter->isValid(); (*symIter)++) {
		    mDUG->insertDepSymList(symIter->current(), proc);
		}
	    }
	}
    }
}



void whatIsIt(std::string name, OA_ptr<OA::MemRefExpr> mre)
{
    std::cout << name << ": mre is ";
    if (mre->isaNamed()) std::cout << "Named";
    if (mre->isaUnnamed()) std::cout << "Unnamed ";
    if (mre->isaUnknown()) std::cout << "Unknown ";
    if (mre->isaRefOp()) std::cout << "RefOp ";
    if (mre->isaDeref()) std::cout << "Deref ";
    if (mre->isaAddressOf()) std::cout << "AddressOf ";
    if (mre->isaSubSetRef()) std::cout << "SubSetRef ";
    if (mre->isaIdxAccess()) std::cout << "IdxAccess ";
    if (mre->isaIdxExprAccess()) std::cout << "IdxExprAccess ";
    if (mre->isaFieldAccess()) std::cout << "FieldAccess ";
    std::cout << std::endl;
}



SymHandle
ManagerDUGStandard::getModSymFromMRE(OA_ptr<OA::MemRefExpr> mre)
{
    whatIsIt("getModSymFromMRE", mre);
    if(mre->isaRefOp())
    {
	OA_ptr<RefOp> refOp = mre.convert<RefOp>();
            
	if(refOp->isaAddressOf()) {
	    OA_ptr<MemRefExpr> subMemRef = refOp->getMemRefExpr();
               
	    if(!subMemRef->isaUnnamed())
	    {
#ifdef DEBUG_DUAA
		std::cout << "    getModSymFromMRE::RefOp,AddressOf,getBaseSym:: "
			  << mIR->toString(refOp->getBaseSym()) << std::endl;
#endif
		return refOp->getBaseSym();
	    } 
	} 
    }

    return SymHandle(0);
}




SymHandle
ManagerDUGStandard::getSymFromMRE(OA_ptr<OA::MemRefExpr> mre)
{
#ifdef DEBUG_DUAA
    whatIsIt("  getSymFromMRE", mre);
#endif
    if(mre->isaNamed())
    {
	OA_ptr<NamedRef> namedRef = mre.convert<NamedRef>();
#ifdef DEBUG_DUAA
	std::cout << "    getSymFromMRE-1::Named:: "
		  << mIR->toString(namedRef->getSymHandle()) << std::endl;
#endif
	return namedRef->getSymHandle();
    }

    if(mre->isaRefOp())
    {
	OA_ptr<RefOp> refOp = mre.convert<RefOp>();
            
	if(refOp->isaAddressOf()) {
	    return getSymFromMRE(refOp->getMemRefExpr());
	} 
	else {    
#ifdef DEBUG_DUAA
	    std::cout << "    getSymFromMRE-2::RefOp,non-AddressOf,getBaseSym:: "
		      << mIR->toString(refOp->getBaseSym()) << std::endl;
#endif
	    return refOp->getBaseSym();
	}   
    }

    return SymHandle(0);
}



// collect all symbols in 'stmt' and returns them in 'allSyms'
void ManagerDUGStandard::collectDefsUsesInStmt(
    StmtHandle stmt, ProcHandle proc)
{
#ifdef DEBUG_DUAA
    std::cout << "--------------------------------------------------\n";
    std::cout << "collectDefsUses(stmt:" << stmt.hval() << ")\n";
#endif

    // collect all locations in the stmt
    OA_ptr<MemRefHandleIterator> mrIterPtr;
    mrIterPtr = mIR->getAllMemRefs(stmt);
    for (; mrIterPtr->isValid(); (*mrIterPtr)++ )
    {
	MemRefHandle memref = mrIterPtr->current();
	// get the memory reference expressions for this handle
	OA_ptr<MemRefExprIterator> mreIter;
	mreIter = mIR->getMemRefExprIterator(memref);
     
	// for each mem-ref-expr associated with this memref
	for (; mreIter->isValid(); (*mreIter)++) {
	    OA_ptr<OA::MemRefExpr> mre; mre = mreIter->current();

	    SymHandle sym = getSymFromMRE(mre);
	    if (sym == SymHandle(0)) continue;

	    if (mre->isDef()) mStmtToDefdSyms[stmt].insert(sym);
	    if (mre->isUse()) mStmtToUsedSyms[stmt].insert(sym);

#ifdef DEBUG_DUAA
	    std::cout << mIR->toString(sym) << "(";
	    if (mre->isDef()) std::cout << "def";
	    if (mre->isUse()) std::cout << " use";
	    std::cout << ")" << std::endl;
#endif

	    OA_ptr<Location> loc = mIR->getLocation(proc, sym);
	    if (!loc->isLocal()) mGlobals.insert(sym);
	}
    }
#ifdef DEBUG_DUAA
    std::cout << std::endl;
#endif
}





// returns all 'used' symbols in 'stmt'.
// if not collected already, collect them before returning them.
std::set<SymHandle>&
ManagerDUGStandard::getUses(StmtHandle stmt, ProcHandle proc)
{
#ifdef DEBUG_DUAA
    std::cout << "getUses(stmt:" << stmt.hval() << ")" << std::endl;
#endif
    // Collect uses and defs if not collected previously.
    if (mDefsUsesCollected.find(stmt) == mDefsUsesCollected.end()){
	collectDefsUsesInStmt(stmt, proc);
	mDefsUsesCollected.insert(stmt);
    }

    return mStmtToUsedSyms[stmt];
}



// returns all 'defined' symbols in 'stmt' 
// if not already collected, collect them.
std::set<SymHandle>& 
ManagerDUGStandard::getDefs(StmtHandle stmt, ProcHandle proc)
{
#ifdef DEBUG_DUAA
    std::cout << "getDefs(stmt:" << stmt.hval() << ")" << std::endl;
#endif
    // Collect uses and defs if not collected previously.
    if (mDefsUsesCollected.find(stmt) == mDefsUsesCollected.end()){
	collectDefsUsesInStmt(stmt, proc);
	mDefsUsesCollected.insert(stmt);
    }

    return mStmtToDefdSyms[stmt];
}




// returns all 'defined' symbols in the call sites of the 'stmt' 
// if not already collected, collect them.
std::set<SymHandle>& 
ManagerDUGStandard::getCallDefs(StmtHandle stmt, ProcHandle proc)
{
#ifdef DEBUG_DUAA
    std::cout << "getCallDefs(stmt:" << stmt.hval() << ")" << std::endl;
#endif
    // Return call parameter info
    if (mCallInfoReady.find(stmt) == mCallInfoReady.end()){
	getCallInfo(stmt, proc);
    }

    return mCallByRefActuals[stmt];
}




// - connect in the VDG the used variables of the statement
// - upward exposed defs: formals, globals using UD chain
void
ManagerDUGStandard::upwardExposedUses(SymHandle def, StmtHandle stmt, 
                                      ProcHandle proc, 
                                      OA_ptr<UDDUChains::UDDUChainsStandard> UDDUChainForProc)
{
#ifdef DEBUG_DUAA
    std::cout << "upwardExposedUses(stmt:" << stmt.hval() << "): " << std::endl;
#endif
    IRSymHandle defIRH(stmt,def);
    std::set<SymHandle>& useSet = getUses(stmt, proc);

    OA_ptr<StmtHandleIterator> rdStmtIter;
    rdStmtIter = UDDUChainForProc->getUDChainStmtIterator(stmt);
    for (rdStmtIter->reset(); rdStmtIter->isValid(); (*rdStmtIter)++ ){
	StmtHandle rdStmt = rdStmtIter->current();

	// upward exposed uses: formals, globals
	if (rdStmt == StmtHandle(0)){
	    // This is an overestimation. Not all uses of formal parameters
	    // may have an upward exposed use.
	    std::set<SymHandle>::iterator useIter;
	    for (useIter=useSet.begin(); useIter!=useSet.end(); useIter++) {
		if (isFormal(*useIter, proc)){
		    IRSymHandle useIRH(StmtHandle(1),*useIter);// incoming formals
		    insertEdge(useIRH, defIRH, CFLOW_EDGE, CallHandle(0), proc, proc, proc);
		}
		else if (isGlobal(*useIter)){
		    mGlobalUpUses[*useIter].insert(GlobalNode(defIRH,proc,CallHandle(0),proc));
		}
	    }
	    continue;
	}
    }
}




// - connect in the VDG the defined variables of the statement
// - downward exposed defs: formals, globals using DU chain
void
ManagerDUGStandard::downwardExposedDefs(SymHandle def, StmtHandle stmt, 
                                        ProcHandle proc, 
                                        OA_ptr<UDDUChains::UDDUChainsStandard> UDDUChainForProc)
{
    IRSymHandle defIRH(stmt,def);

    if (!isFormal(def, proc) && !isGlobal(def)) return;
#ifdef DEBUG_DUAA
    std::cout << "DUChain for stmt(" <<  stmt.hval() << "), var("  
	      << mIR->toString(def) << "): ";
#endif
    OA_ptr<StmtHandleIterator> useStmtIter;
    useStmtIter = UDDUChainForProc->getDUChainStmtIterator(stmt);
    for (; useStmtIter->isValid(); (*useStmtIter)++ ){
	StmtHandle useStmt = useStmtIter->current();
#ifdef DEBUG_DUAA
	std::cout << "useStmt(" <<  useStmt.hval() << "), ";
#endif
	// Regular 'uses' will be processed using 'UD-chains'.
	if (useStmt != StmtHandle(0)) continue;

	if (isFormal(def, proc)){
	    IRSymHandle useIRH(StmtHandle(2),def); // outgoing formals
	    insertEdge(defIRH, useIRH, CFLOW_EDGE, CallHandle(0), proc, proc, proc);
	}
	else if (isGlobal(def)){
	    mGlobalDnDefs[def].insert(GlobalNode(defIRH, proc, CallHandle(0), proc));
#ifdef DEBUG_DUAA
	    std::cout << mIR->toString(def) << "is added to GlobalDnDefs in stmt(" 
		      << stmt.hval() << ")" << std::endl;
#endif
	}
    }
}




// display an edge from a node key
void 
ManagerDUGStandard::printEdgeDot(IRSymHandle from, ProcHandle fromProc,
				 IRSymHandle to,   ProcHandle toProc)
{
    OA_ptr<Node> fromNode = mDUG->getNode(from, fromProc);
    OA_ptr<Node> toNode   = mDUG->getNode(to,   toProc);
    
    fromNode->dumpdot(std::cout, mIR);
    std::cout << "->";
    toNode->dumpdot(std::cout, mIR);
    std::cout << std::endl;
}




// for each (use, def) pair of a stmt
void ManagerDUGStandard::labelUseDefEdges(
    StmtHandle stmt, ProcHandle proc)
{
#ifdef DEBUG_DUAA
    std::cout << "labelUseDefEdges(stmt:" << stmt.hval() << "): " << std::endl;
    bool debug=false;
    unsigned debug_stmt_no = 1083755148;
    if (stmt.hval() == debug_stmt_no){
	std::cout << "==============debug_stmt_no================" << std::endl;
	debug = true;
    }
#endif
    std::set<SymHandle>& defSet = getDefs(stmt, proc);
    std::set<SymHandle>& useSet = getUses(stmt, proc);

    OA_ptr<UDDUChains::UDDUChainsStandard> UDDUChainForProc;
    UDDUChainForProc = mProcToUDDUChainsMap[proc];

    // for each def
    std::set<SymHandle>::iterator defIter;
    for (defIter=defSet.begin(); defIter!=defSet.end(); defIter++) {
	IRSymHandle def(stmt,*defIter);
#ifdef DEBUG_DUAA
	if (debug) 
	    std::cout << "  debug_stmt_no: inside defIter:" 
		      << mIR->toString(*defIter) << std::endl;
#endif

	upwardExposedUses(*defIter, stmt, proc, UDDUChainForProc);

	// for each use
	std::set<SymHandle>::iterator useIter=useSet.begin();
	for (; useIter!=useSet.end(); useIter++) {
#ifdef DEBUG_DUAA
	    if (debug) 
		std::cout << "  debug_stmt_no: inside useIter:" 
			  << mIR->toString(*useIter) << std::endl;
#endif
	    // for each reaching definition
	    OA_ptr<StmtHandleIterator> rdStmtIter;
	    rdStmtIter = UDDUChainForProc->getUDChainStmtIterator(stmt);
	    for (; rdStmtIter->isValid(); (*rdStmtIter)++ ){
		StmtHandle rdStmt = rdStmtIter->current();
#ifdef DEBUG_DUAA
		if (debug) 
		    std::cout << "  debug_stmt_no: inside rdStmtIter:" 
			      << rdStmt.hval() << std::endl;
#endif
		std::set<SymHandle>& rdDefSet = getDefs(rdStmt, proc);
		std::set<SymHandle>& rdCallDefs = getCallDefs(rdStmt, proc);
		if (rdDefSet.find(*useIter)   != rdDefSet.end()   ||
		    rdCallDefs.find(*useIter) != rdCallDefs.end() ){
		    IRSymHandle use(rdStmt,*useIter);
#ifdef DEBUG_DUAA
		    std::cout << "inserting ... "; printEdge(use, def, CFLOW_EDGE);
		    std::cout << std::endl;
#endif
		    insertEdge(use, def, CFLOW_EDGE, CallHandle(0), proc, proc, proc);
		}
#ifdef DEBUG_DUAA
		else
		    std::cout << mIR->toString(*useIter) << " is not defined in rdStmt(" 
			      << rdStmt.hval() << ")" << std::endl;
#endif
	    }
	}

	downwardExposedDefs(*defIter, stmt, proc, UDDUChainForProc);
    }
}



/*!
  Connect global variable nodes
*/
void ManagerDUGStandard::connectGlobals()
{
#ifdef DEBUG_DUAA
    std::cout << "connectGlobals: ";
#endif
    std::map<SymHandle, std::set<GlobalNode> >::iterator globalUseIter;
    globalUseIter=mGlobalUpUses.begin();
    for(; globalUseIter!=mGlobalUpUses.end(); globalUseIter++){
	SymHandle global = globalUseIter->first;
	std::set<GlobalNode>& globalUses = globalUseIter->second;
	std::set<GlobalNode>& globalDefs = mGlobalDnDefs[global];
#ifdef DEBUG_DUAA
	std::cout << mIR->toString(global) << ", ";
#endif

	std::set<GlobalNode>::iterator dIter, uIter;
	for(uIter=globalUses.begin(); uIter!=globalUses.end(); uIter++){
	    IRSymHandle use         = uIter->mIRnSym;
	    ProcHandle  useProc     = uIter->mProc;
	    ProcHandle  useStmtProc = uIter->mUseProc;
	    EdgeType    etype   = (uIter->mCall == CallHandle(0) ? CFLOW_EDGE : CALL_EDGE);
	    for(dIter=globalDefs.begin(); dIter!=globalDefs.end(); dIter++){
		IRSymHandle def       = dIter->mIRnSym;
		ProcHandle defProc    = dIter->mProc;

		insertEdge(def, use, etype, uIter->mCall, defProc, useProc, useStmtProc);
	    }
	    if (globalDefs.empty() && (mDUG->isIndependent(useProc, global))){
		IRSymHandle indepGlobal(IRHandle(0), global);
		insertEdge(indepGlobal, use, CFLOW_EDGE, CallHandle(0), useProc, 
			   useProc, useStmtProc);
	    }
	}
    }
#ifdef DEBUG_DUAA
    std::cout << std::endl;
#endif
}


void 
ManagerDUGStandard::collectFormalParameters(ProcHandle proc)
{
    int cnt = 0;

    SymHandle sym = mIR->getFormalSym(proc, cnt++);
    while (sym != SymHandle(0)) 
    {
	mProcToFormalSet[proc].insert(sym); 
#ifdef DEBUG_DUAA
	std::cout << "collectFormalParameters(inserting): "
		  << mIR->toString(sym) << std::endl;
#endif
	sym = mIR->getFormalSym(proc, cnt++);
    }
}



/*!
  Creates a DUG for the program
*/
OA_ptr<DUGStandard> ManagerDUGStandard::performAnalysis( 
    OA_ptr<IRProcIterator> procIter,
    OA_ptr<DataFlow::ParamBindings> paramBind,
    OA_ptr<OA::CallGraph::CallGraphInterface> cgraph)
{

    mParamBind  = paramBind;

    // create a Manager that generates dep information for each statement in
    OA_ptr<DUGStandard> dug; 
    mDUG = dug = new DUGStandard( mIR, paramBind);
  
#ifdef DEBUG_DUAA
    std::cout << "ManagerDUGStandard::performAnalysis: ---" << std::endl;
#endif
    // Mark 'independent' and 'dependent' variables
    for ( procIter->reset(); procIter->isValid(); (*procIter)++ ) {
	//    std::cout << "Inside trying to get Dep and Indep Symbols" << std::endl;
	collectIndependentSyms(procIter->current());
	collectDependentSyms  (procIter->current());
	collectFormalParameters(procIter->current());
    }

    OA_ptr<OA::CallGraph::NodesIteratorInterface> callGraphIter;
    callGraphIter = cgraph->getCallGraphReversePostDFSIterator(DGraph::DEdgeOrg);

    for ( ; callGraphIter->isValid(); ++(*callGraphIter)) {
	OA_ptr<CallGraph::NodeInterface> node; 
	node = callGraphIter->currentCallGraphNode();
	ProcHandle proc = node->getProc();
	dug->assignActiveStandard(proc);

	// skip unreachable procedures
	if(mProcsOfInterest.find(proc) == mProcsOfInterest.end()) continue;

	OA_ptr<OA::IRStmtIterator> sItPtr; 
	sItPtr = mIR->getStmtIterator(proc);
	for ( ; sItPtr->isValid(); (*sItPtr)++) {
	    StmtHandle stmt = sItPtr->current();
	    labelUseDefEdges(stmt, proc);
	    if (stmt_has_call(stmt)){
		labelCallRetEdges(stmt, proc);
	    }
	}
    }
    connectGlobals();

    return dug;
}




/*!
  Set the dependence matrix entry to 'true'
*/
void ManagerDUGStandard::setDepMatrix( 
    ProcHandle   proc, 
    IRSymHandle  use, 
    IRSymHandle  def)
{
    if (use == def) return;
    mProcToIRSymSet[proc].insert(use);
    mProcToIRSymSet[proc].insert(def);
    mProcToMatrix[proc][use][def] = true;

#ifdef DEBUG_DUAA
    std::cout << mIR->toString(proc) << ": ";
    printIRSymHandle(use, std::cout, mIR); 
    std::cout << " ---> ";
    printIRSymHandle(def, std::cout, mIR); 
    std::cout << std::endl;
#endif
}




/*!
  returns a set of nodes of 'proc' that have outgoing 
  edges for other procedures
*/
void ManagerDUGStandard::findOutgoingNodes(
    IRSymHandle ish, ProcHandle proc, 
    std::set<IRSymHandle>& OutGoingNodeSet)
{
    if (!mDUG->isNode(ish, proc)) return;
    OA_ptr<Node> node = mDUG->getNode(ish, proc);

    std::set<IRSymHandle> visited;
    node->findOutgoingNodes(proc, OutGoingNodeSet, visited);
}




/*!
  returns a set of nodes of 'proc' that have incoming 
  edges for other procedures
*/
void ManagerDUGStandard::findIncomingNodes(
    IRSymHandle ish, ProcHandle proc, 
    std::set<IRSymHandle>& IncomingNodeSet)
{
    if (!mDUG->isNode(ish, proc)) return;
    OA_ptr<Node> node = mDUG->getNode(ish, proc);

    std::set<IRSymHandle> visited;
    node->findIncomingNodes(proc, IncomingNodeSet, visited);
}




bool ManagerDUGStandard::hasEdgesToOtherProc(IRSymHandle ish, ProcHandle proc)
{
    // to filter out a fake parameter '.len' representing the length of 
    // a string parameter.
    if (!mDUG->isNode(ish, proc)) return false;
    OA_ptr<Node> node = mDUG->getNode(ish, proc);

    std::set<IRSymHandle> visited;
    return node->hasEdgesToOtherProc(proc, visited);
}




bool ManagerDUGStandard::hasEdgesFromOtherProc(IRSymHandle ish, ProcHandle proc)
{
    // to filter out a fake parameter '.len' representing the length of 
    // a string parameter.
    if (!mDUG->isNode(ish, proc)) return false;
    OA_ptr<Node> node = mDUG->getNode(ish, proc);

    std::set<IRSymHandle> visited;
    return node->hasEdgesFromOtherProc(proc, visited);
}



/*!
  'true' if 'ish(IR SymHandle)' has an outgoing edge to other procedure
  'true' if 'ish' is defined inside and used outside.
*/
bool ManagerDUGStandard::isOutgoingToOtherProcs(
    IRSymHandle ish, ProcHandle proc )
{
    assert(mDUG->isNode(ish, proc));
    OA_ptr<Node> node = mDUG->getNode(ish, proc);

    bool definedInside = false;
    OA_ptr<EdgesIteratorInterface> predIterPtr
	= node->getDUGIncomingEdgesIterator();
    for (; predIterPtr->isValid() && !definedInside; ++(*predIterPtr)) {
	OA_ptr<EdgeInterface> predEdge = predIterPtr->currentDUGEdge();
	if (predEdge->getType() != CFLOW_EDGE) continue;
  
	if (predEdge->getProc() == proc) definedInside = true;
    }
    if (!definedInside) return false;


    bool usedOutside = false;
    OA_ptr<EdgesIteratorInterface> succIterPtr
	= node->getDUGOutgoingEdgesIterator();
    for (; succIterPtr->isValid() && !usedOutside; ++(*succIterPtr)) {
	OA_ptr<EdgeInterface> succEdge = succIterPtr->currentDUGEdge();
	if (succEdge->getType() != CFLOW_EDGE) continue;
  
	if (succEdge->getProc() != proc) return true;
    }

    return false;
}




/*!
  'true' if 'ish(IR SymHandle)' has an incoming edge for other 
  procedures and an outgoing edge for this procedure: 
  I.e., 'true' if ish is defined outside and used inside
*/
bool ManagerDUGStandard::isIncomingFromOtherProcs(
    IRSymHandle ish, ProcHandle proc )
{
    assert(mDUG->isNode(ish, proc));
    OA_ptr<Node> node = mDUG->getNode(ish, proc);

    bool definedOutside = false;
    OA_ptr<EdgesIteratorInterface> predIterPtr
	= node->getDUGIncomingEdgesIterator();
    for (; predIterPtr->isValid() && !definedOutside; ++(*predIterPtr)) {
	OA_ptr<EdgeInterface> predEdge = predIterPtr->currentDUGEdge();
	if (predEdge->getType() != CFLOW_EDGE) continue;
  
	if (predEdge->getProc() != proc) definedOutside = true;
    }
    if (!definedOutside) return false;


    bool usedInside = false;
    OA_ptr<EdgesIteratorInterface> succIterPtr
	= node->getDUGOutgoingEdgesIterator();
    for (; succIterPtr->isValid() && !usedInside; ++(*succIterPtr)) {
	OA_ptr<EdgeInterface> succEdge = succIterPtr->currentDUGEdge();
	if (succEdge->getType() != CFLOW_EDGE) continue;
  
	if (succEdge->getProc() == proc) return true;
    }

    return false;
}




/*!
  'true' if there is a path from 'use' to 'def' through other procedures 
  than 'proc'. We traverse the VDG backward from 'def'.
*/
bool ManagerDUGStandard::isPathThruOtherProcs(
    IRSymHandle use, IRSymHandle def, ProcHandle proc )
{
    OA_ptr<NodeInterface> defNode = mDUG->getNode(def, proc);
    OA_ptr<NodeInterface> useNode = mDUG->getNode(use, proc);

    std::set<OA_ptr<NodeInterface> > visited;
    visited.insert(defNode);
    return defNode->isPathFrom(useNode, visited);
}




/*!
  For two global variables 'use' and 'def', there can be a value flow
  through other procedures. The dependence matrix entry for 'use' and
  'def' is set to 'true' if there is a path from 'use' to 'def' in the 
  VDG.
*/
void ManagerDUGStandard::setDepMatrix4Globals(
    IRSymHandle use, IRSymHandle def, ProcHandle proc )
{
    std::map<IRSymHandle, std::map<IRSymHandle, bool> >&
	depMat = mProcToMatrix[proc];

    // Do nothing if 'use' is not used in other procs, 
    if (!isOutgoingToOtherProcs(use, proc)) return;

    // Do nothing if 'def' is not defined from other procs, 
    if (!isIncomingFromOtherProcs(def, proc)) return;

#ifdef DEBUG_DUAA
    std::cout << "setDepMatrix4Globals, checking Paths(" << mIR->toString(proc) <<
	"): " << mIR->toString(use.second) << " -> " << mIR->toString(def.second) << std::endl;
#endif
    // Do nothing if there is no path from 'use' to 'def',
    if (!isPathThruOtherProcs(use, def, proc)) return;

    depMat[use][def] = true;
#ifdef DEBUG_DUAA
    std::cout << "Value passing globals(" << mIR->toString(proc) << "): " 
	      << mIR->toString(use.second) << " -> " << mIR->toString(def.second) 
	      << std::endl;
#endif
}




/*!
  Warshall's transitive closure algorithm on the dependence matrix
*/
void ManagerDUGStandard::transitiveClosureDepMatrix(
    OA_ptr<OA::CallGraph::CallGraphInterface> cgraph
    )
{
    // use 'DEdgeRev' to get PostDFSIterator
    // 'getPostDFSIterator' is not implemented
    OA_ptr<OA::CallGraph::NodesIteratorInterface> iter;
    iter = cgraph->getCallGraphReversePostDFSIterator(DGraph::DEdgeRev);

    for ( ; iter->isValid(); ++(*iter)) {
	OA_ptr<CallGraph::NodeInterface> node; 
	node = iter->currentCallGraphNode();
	ProcHandle proc = node->getProc();
	transitiveClosure(proc);
	edgesBetweenFormals(proc);
    }
}



/*!
  Warshall's transitive closure algorithm on the dependence matrix 
  of one procedure. This makes edges between formal parameters too.
*/
void ManagerDUGStandard::transitiveClosure(ProcHandle proc)
{
    std::map<IRSymHandle, std::map<IRSymHandle, bool> >& depMat = mProcToMatrix[proc];

#ifdef DEBUG_DUAA
    std::cout << "*** transitiveClosure(" << mIR->toString(proc) 
	      << ") ***" << std::endl;
#endif
    std::set<IRSymHandle>& irSymSet = mProcToIRSymSet[proc];
    std::set<IRSymHandle>::iterator i, j, k;
    IRSymHandle use, def, var;
    for (k=irSymSet.begin(); k!=irSymSet.end(); k++){
	var = *k;
	for (i=irSymSet.begin(); i!=irSymSet.end(); i++){
	    use = *i;
	    if (!depMat[use][var]) continue;
	    for (j=irSymSet.begin(); j!=irSymSet.end(); j++){
		def = *j;
		if (!depMat[var][def]) continue;
		depMat[use][def] = true;
	    }
	}
    }
#ifdef DEBUG_DUAA
    std::cout << "*** END:transitiveClosure(" << mIR->toString(proc) 
	      << ") ***" << std::endl;
#endif
}




/*!
  This makes edges between formal parameters.
*/
void ManagerDUGStandard::edgesBetweenFormals(ProcHandle proc)
{
    std::map<IRSymHandle, std::map<IRSymHandle, bool> >& depMat = mProcToMatrix[proc];

    std::set<CallHandle>& callsites = mProcToCallsiteSet[proc];
    std::set<CallHandle>::iterator callIter;

    std::set<SymHandle>& formals = mProcToFormalSet[proc];
    std::set<SymHandle>::iterator i, j;
  
    for (i=formals.begin(); i!=formals.end(); i++){
	SymHandle formal1 = *i;
	for (j=formals.begin(); j!=formals.end(); j++){
	    SymHandle formal2 = *j;

	    // IRHandle(1) for a node incoming into 'foo' for 'formal1'
	    // IRHandle(2) for a node outgoing from 'foo' for 'formal2'
	    IRSymHandle formalNodeIn(IRHandle(1), formal1);
	    IRSymHandle formalNodeOut(IRHandle(2), formal2);
	    //--------------------------------------------------
	    // - Value flow between formals through two global variables
	    //   not in this procedure
	    // - This is necessary for PARAM_EDGES
	    //--------------------------------------------------
	    if (!depMat[formalNodeIn][formalNodeOut]){
		if (!hasEdgesToOtherProc(formalNodeIn, proc)) continue;
		if (!hasEdgesFromOtherProc(formalNodeOut, proc)) continue;
		if (isPathThruOtherProcs(formalNodeIn, formalNodeOut, proc)) {
#ifdef DEBUG_DUAA
		    std::cout << "foundPath(" << mIR->toString(proc) << "): " << mIR->toString(formal1)
			      << " -> " << mIR->toString(formal2) << std::endl;
#endif
		    depMat[formalNodeIn][formalNodeOut] = true;
		}
	    }

	    if (depMat[formalNodeIn][formalNodeOut]){
		insertEdge(formalNodeIn, formalNodeOut, PARAM_EDGE, CallHandle(0), proc, 
			   proc, proc);

		for (callIter = callsites.begin(); callIter!=callsites.end(); callIter++){
		    CallHandle call = *callIter;
		    // get calling procedure
		    ProcHandle caller = mCallsiteToProc[call];
		    assert(caller != ProcHandle(0));
		    std::set<IRSymHandle>& set1 = mFormalToActualMap[call][formalNodeIn];
		    std::set<IRSymHandle>& set2 = mFormalToActualMap[call][formalNodeOut];
		    std::set<IRSymHandle>::iterator i1;
		    std::set<IRSymHandle>::iterator i2;
		    for (i1=set1.begin(); i1 != set1.end(); i1++){
			for (i2=set2.begin(); i2 != set2.end(); i2++){
			    setDepMatrix(caller, *i1, *i2);
			}
		    }
		}
	    }
	}
    }
}



void IndepLocVisitor::visitNamedLoc(NamedLoc& loc)
{
    mDUG->insertIndepSymList(loc.getSymHandle(), mProc);
    mProcsOfInterest.insert(mProc);

}

void IndepLocVisitor::visitUnnamedLoc(UnnamedLoc& loc)
{
    //assert(0);
}

void IndepLocVisitor::visitInvisibleLoc(InvisibleLoc& loc)
{
    mDUG->insertIndepSymList(loc.getBaseSym(), mProc);
    mProcsOfInterest.insert(mProc);
}

void IndepLocVisitor::visitUnknownLoc(UnknownLoc& loc)
{
    assert(0);
}

void IndepLocVisitor::visitLocSubSet(LocSubSet& loc)
{
    OA_ptr<Location> ll = loc.getBaseLoc();
    if (!ll.ptrEqual(0)) { ll->acceptVisitor(*this); }
}

void depLocVisitor::visitNamedLoc(NamedLoc& loc)
{
    mDUG->insertDepSymList(loc.getSymHandle(), mProc);
    mProcsOfInterest.insert(mProc);

}

void depLocVisitor::visitUnnamedLoc(UnnamedLoc& loc)
{
    //assert(0);
}

void depLocVisitor::visitInvisibleLoc(InvisibleLoc& loc)
{
    
    mDUG->insertDepSymList(loc.getBaseSym(), mProc);
    mProcsOfInterest.insert(mProc);

}

void depLocVisitor::visitUnknownLoc(UnknownLoc& loc)
{
    assert(0);
}

void depLocVisitor::visitLocSubSet(LocSubSet& loc)
{
    OA_ptr<Location> ll = loc.getBaseLoc();
    if (!ll.ptrEqual(0)) { ll->acceptVisitor(*this); }
}




    } // end of namespace MPDUG
} // end of namespace OA

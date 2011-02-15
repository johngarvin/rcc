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

/*
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
*/


ManagerDUGStandard::ManagerDUGStandard(OA_ptr<DUGIRInterface> _ir, 
                                       OA_ptr<Activity::ActivityIRInterface> _air) 
    : mIR(_ir), mActIR(_air)
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

/*!
  Creates a DUG for the program
*/
void ManagerDUGStandard::insertEdge( 
    SymHandle        from,
    SymHandle        to,
    EdgeType         etype,
    CallHandle       expr,
    ProcHandle       fromProc,
    ProcHandle       toProc,
    ProcHandle       proc)
{
    if (from == to) {
	OA_ptr<Node> fromNode = mDUG->getNode(from, fromProc);
	fromNode->setSelfDependent();

	return;
    }
    // duplicate edges
    if (etype == CFLOW_EDGE && mMatrix[etype][from][to]) return;
    mMatrix[etype][from][to] = true;

#ifdef DEBUG_DUAA
    static const char *sEdgeTypeToString[] = {
	"CFLOW",
	"CALL",
	"RETURN",
	"PARAM"
    };
    std::cout << "insertEdge(" << sEdgeTypeToString[etype] << "): " 
	      << mIR->toString(from) << "@" << mIR->toString(fromProc)
	      << " -> " << mIR->toString(to) << "@" << mIR->toString(toProc) << std::endl;
#endif
    OA_ptr<Node> fromNode = mDUG->getNode(from, fromProc);
    OA_ptr<Node> toNode   = mDUG->getNode(to, toProc);

    // edge between 'from' and 'to' node
    OA_ptr<Edge> dugEdge;
    dugEdge = new Edge(mDUG,fromNode, toNode, etype, expr, proc);
    mDUG->addEdge(dugEdge);
#ifdef SAC07
    sac07_numGlobalEdges++;
#endif
}



/*!
  Creates an DUG for the program
*/
void ManagerDUGStandard::labelCallRetEdges(
    StmtHandle stmt, ProcHandle proc)
{

#ifdef DEBUG_DUAA
    std::cout << "labelCallRetEdges:" << std::endl;
#endif
    
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

	// to process only reachable procedures
	mProcsOfInterest.insert(callee);

	mProcToCallsiteSet[callee].insert(call); 
	mCallsiteToProc[call] = proc;

<<<<<<< .working
	// iterate over formal parameters for callee procedure
	// iterate over actual parameters at the same time
	OA_ptr<IRCallsiteParamIterator> actualIter;
	actualIter = mIR->getCallsiteParams(call);
	for (int formalCnt=0; actualIter->isValid(); formalCnt++, ++(*actualIter)) 
	{
	    // formal
	    SymHandle formalSym = mIR->getFormalSym(callee, formalCnt);
	    mProcToFormalSet[callee].insert(formalSym); 
=======
    OA_ptr<DataFlow::ParamBindIterator> pIter;
    pIter = mParamBind->getParamBindIterator(call);
>>>>>>> .merge-right.r888

<<<<<<< .working
	    // actual
	    ExprHandle param = actualIter->current();
	    OA_ptr<ExprTree> eTreePtr; eTreePtr = mIR->getExprTree(param);
	    ExprTree::NodesIterator nodes_iter(*eTreePtr);
=======
>>>>>>> .merge-right.r888

<<<<<<< .working
	    for ( ; nodes_iter.isValid(); ++nodes_iter) {
=======
    for( ; pIter->isValid(); ++(*pIter) ) {
        
        OA_ptr<MemRefExpr> formalMRE = pIter->currentFormal();
        OA_ptr<ExprTree> actualETree = pIter->currentActual();
>>>>>>> .merge-right.r888

<<<<<<< .working
		OA_ptr<ExprTree::Node> exprTreeNode; 
		exprTreeNode = nodes_iter.current();
=======
        CollectMREVisitor evalVisitor;
        actualETree->acceptVisitor(evalVisitor);
>>>>>>> .merge-right.r888

<<<<<<< .working
		if ( exprTreeNode->isaMemRefNode() ) {
=======
        //! ======= Collect all use MREs from ExprTree =========
        OA::set<OA_ptr<MemRefExpr> > mSet;
        mSet = evalVisitor.getMemRef();       
>>>>>>> .merge-right.r888

<<<<<<< .working
		    OA_ptr<ExprTree::MemRefNode> memRefNode;
		    memRefNode = exprTreeNode.convert<ExprTree::MemRefNode>();
		    MemRefHandle memref = memRefNode->getHandle();
=======
        OA::OA_ptr<std::set<OA_ptr<MemRefExpr> > > mSetPtr;
        mSetPtr = new std::set<OA_ptr<MemRefExpr> >(mSet);
>>>>>>> .merge-right.r888

<<<<<<< .working
		    // get the memory reference expressions for this handle
		    OA_ptr<MemRefExprIterator> mreIter;
		    mreIter = mIR->getMemRefExprIterator(memref);
      
		    // for each mem-ref-expr associated with this memref
          
		    for (; mreIter->isValid(); (*mreIter)++) {
              
			OA_ptr<OA::MemRefExpr> mre; mre = mreIter->current();
			SymHandle actualSym;
			if(mre->isaRefOp())
			{
			    OA_ptr<RefOp> refOp = mre.convert<RefOp>();
			    if( refOp->isaAddressOf() ) { mre = refOp->getMemRefExpr(); }
			    if(mre->isaNamed()) {
				OA_ptr<NamedRef> namedRef = mre.convert<NamedRef>();
				actualSym = namedRef->getSymHandle();         
			    } else if(mre->isaRefOp()) {
				OA_ptr<RefOp> refrefop = mre.convert<RefOp>();
				actualSym = refrefop->getBaseSym();
			    } else {
				continue;
			    }
=======
        OA_ptr<MemRefExprIterator> mreIter;
        mreIter = new OA::MemRefExprIterator(mSetPtr);

        for( ; mreIter->isValid(); ++(*mreIter) ) {

           OA::OA_ptr<OA::MemRefExpr> actualMRE;
           actualMRE = mreIter->current();
>>>>>>> .merge-right.r888

<<<<<<< .working
			}  else if(mre->isaNamed()) {
			    OA_ptr<NamedRef> namedRef = mre.convert<NamedRef>();
			    actualSym = namedRef->getSymHandle();
=======
           //! ===== Visitor to extract SymHandle from MRE =====
           SymFromMREVisitor sVisitor(mIR);
>>>>>>> .merge-right.r888

<<<<<<< .working
			}
			mFormalToActualMap[call][formalSym].insert(actualSym);
			insertEdge(actualSym, formalSym, CALL_EDGE, call, proc, callee, proc);
			OA_ptr<Location> baseSymLoc = mIR->getLocation(callee, formalSym);
			insertEdge(formalSym, actualSym, RETURN_EDGE, call, callee, proc, proc);
=======
           //! ===== Process Actual MRE =====
           actualMRE->acceptVisitor(sVisitor);
           OA_ptr<std::set<SymHandle> > actualSymSet;
           actualSymSet = sVisitor.getSymSet();
>>>>>>> .merge-right.r888

<<<<<<< .working
			mDUG->mapSymToMemRefSet(actualSym, memref);
			mDUG->mapSymToStmtSet(actualSym, stmt);
=======
           //! ===== Process Formal MRE =====
           formalMRE->acceptVisitor(sVisitor);
           OA_ptr<std::set<SymHandle> > formalSymSet;
           formalSymSet = sVisitor.getSymSet();
 
>>>>>>> .merge-right.r888

<<<<<<< .working
		    }
		}
	    }
	}
    }
=======
           //! ===== Create Map and DUG Graph Between Formal and Actual =====
           std::set<SymHandle>::iterator aSymIter, fSymIter;
           for( aSymIter = actualSymSet->begin(); 
                aSymIter != actualSymSet->end(); ++aSymIter )            
           {  
             SymHandle actualSym;
             actualSym = *(aSymIter);
 
             for( fSymIter = formalSymSet->begin(); 
                  fSymIter != formalSymSet->end(); ++fSymIter )  
             {
               SymHandle formalSym;
               formalSym = *(fSymIter);

                 
               mFormalToActualMap[call][formalSym].insert(actualSym);
               insertEdge(actualSym, formalSym, CALL_EDGE, call, proc, callee);
               insertEdge(formalSym, actualSym, RETURN_EDGE, call, callee, proc);
               mDUG->mapSymToStmtSet(actualSym, stmt);

             } //! end for fSymIter

           } //! end for aSymIter

        } // end for mreIter

    } // end for pIter

  } // end for callsiteIter

>>>>>>> .merge-right.r888
}




/*!
  collect independent variables
*/
void
ManagerDUGStandard::collectIndependentSyms( ProcHandle proc)
{
#ifdef DEBUG_DUAA
    std::cout << "collectIndependentSyms: ---" << std::endl;
#endif  
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
#ifdef DEBUG_DUAA
	    std::cout << "collectIndependentSyms: " 
		      << mIR->toString(sym) << std::endl;
#endif  
	}
    }
}



/*!
  collect dependent variables
*/
void
ManagerDUGStandard::collectDependentSyms( ProcHandle proc)
{
#ifdef DEBUG_DUAA
    std::cout << "collectDependentSyms: ---" << std::endl;
#endif  
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
#ifdef DEBUG_DUAA
	    std::cout << "collectDependentSyms: " 
		      << mIR->toString(sym) << std::endl;
#endif  
	}
    }
}



// for each (use, def) pair of a stmt
void ManagerDUGStandard::labelUseDefEdges(
    StmtHandle stmt, ProcHandle proc)
{

#ifdef DEBUG_DUAA
    std::cout << "labelUseDefEdges: ---" << std::endl;
#endif
    std::set<SymHandle> useSet, defSet, allSyms;

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

	    if(mre->isaNamed())
	    {
		OA_ptr<NamedRef> namedRef = mre.convert<NamedRef>();
		SymHandle sym = namedRef->getSymHandle();
		allSyms.insert(sym);

	    }
	    if(mre->isaRefOp())
	    {
		OA_ptr<RefOp> refOp = mre.convert<RefOp>();
            
		if(refOp->isaAddressOf()) {
		    OA_ptr<MemRefExpr> subMemRef = refOp->getMemRefExpr();
               
		    if(subMemRef->isaNamed())
		    {
			OA_ptr<NamedRef> nRef = subMemRef.convert<NamedRef>();
			SymHandle sym = nRef->getSymHandle();
			allSyms.insert(sym);
		    } 
		} else {    
		    SymHandle sym = refOp->getBaseSym();
		    allSyms.insert(sym);
		}   
	    }
	}
    }


    // Want to add a visitor PLM 12/15/06
    // collect uses
    mrIterPtr = mIR->getUseMemRefs(stmt);
    for (; mrIterPtr->isValid(); (*mrIterPtr)++ ) {
	MemRefHandle mref = mrIterPtr->current();

	// get the memory reference expressions for this handle
	OA_ptr<MemRefExprIterator> mreIter; 
	mreIter = mIR->getMemRefExprIterator(mref);
      
	// for each mem-ref-expr associated with this memref
	for (; mreIter->isValid(); (*mreIter)++) {
	    OA_ptr<OA::MemRefExpr> mre; mre = mreIter->current();

	    if(mre->isaNamed())
	    {
		OA_ptr<NamedRef> namedRef = mre.convert<NamedRef>();
		SymHandle use = namedRef->getSymHandle();
<<<<<<< .working

		mDUG->mapSymToMemRefSet(use, mref);
		mDUG->mapSymToStmtSet(use, stmt);
=======
            mDUG->mapSymToStmtSet(use, stmt);
>>>>>>> .merge-right.r888
		useSet.insert(use);
<<<<<<< .working
	    }
	    if(mre->isaRefOp())
	    {
=======
          }

          if(mre->isaRefOp())
          {
>>>>>>> .merge-right.r888
		OA_ptr<RefOp> refOp = mre.convert<RefOp>();
		if(refOp->isaAddressOf()) {
		    OA_ptr<MemRefExpr> subMemRef = refOp->getMemRefExpr();
		    if(subMemRef->isaNamed())
		    {
			OA_ptr<NamedRef> namedRef = mre.convert<NamedRef>();
			SymHandle use = namedRef->getSymHandle();
<<<<<<< .working

			mDUG->mapSymToMemRefSet(use, mref);
			mDUG->mapSymToStmtSet(use, stmt);
=======
                  mDUG->mapSymToStmtSet(use, stmt);
>>>>>>> .merge-right.r888
			useSet.insert(use);
		    } 

		} else {
		    SymHandle use = refOp->getBaseSym();
<<<<<<< .working

		    mDUG->mapSymToMemRefSet(use, mref);
		    mDUG->mapSymToStmtSet(use, stmt);
=======
               mDUG->mapSymToStmtSet(use, stmt);
>>>>>>> .merge-right.r888
		    useSet.insert(use);
		}
	    }
	}
    }

    // collect defs

    // Want to add a visitor PLM 12/15/06
    mrIterPtr = mIR->getDefMemRefs(stmt);
    for (; mrIterPtr->isValid(); (*mrIterPtr)++ ) {
	MemRefHandle mref = mrIterPtr->current();

	// get the memory reference expressions for this handle
	OA_ptr<MemRefExprIterator> mreIter; 
	mreIter = mIR->getMemRefExprIterator(mref);
      
	// for each mem-ref-expr associated with this memref
	for (; mreIter->isValid(); (*mreIter)++) {
	    OA_ptr<OA::MemRefExpr> mre; mre = mreIter->current();


	    if(mre->isaNamed())
	    {
		OA_ptr<NamedRef> namedRef = mre.convert<NamedRef>();
		SymHandle def = namedRef->getSymHandle();
		if (allSyms.find(def) != allSyms.end())
		{
<<<<<<< .working
            
		    mDUG->mapSymToMemRefSet(def, mref);
		    mDUG->mapSymToStmtSet(def, stmt);
=======
             mDUG->mapSymToStmtSet(def, stmt);
>>>>>>> .merge-right.r888
		    defSet.insert(def);
		}
	    }
	    if(mre->isaRefOp())
	    {
              
		OA_ptr<RefOp> refOp = mre.convert<RefOp>();

		if(refOp->isaAddressOf()) {
		    OA_ptr<MemRefExpr> subMemRef = refOp->getMemRefExpr();
		    if(subMemRef->isaNamed())
		    {
			OA_ptr<NamedRef> namedRef = subMemRef.convert<NamedRef>();
			SymHandle def = namedRef->getSymHandle();
			if (allSyms.find(def) != allSyms.end())
			{
<<<<<<< .working

			    mDUG->mapSymToMemRefSet(def, mref);
			    mDUG->mapSymToStmtSet(def, stmt);
=======
                      mDUG->mapSymToStmtSet(def, stmt);
>>>>>>> .merge-right.r888
			    defSet.insert(def);
			}

		    } 

		} else {
		    SymHandle def = refOp->getBaseSym();
            
		    if (allSyms.find(def) != allSyms.end()) {

<<<<<<< .working
			mDUG->mapSymToMemRefSet(def, mref);
			mDUG->mapSymToStmtSet(def, stmt);
=======
                  mDUG->mapSymToStmtSet(def, stmt);
>>>>>>> .merge-right.r888
			defSet.insert(def);
		    }
		}
	    }
	}
    }

    // map all uses to may defs and vice versa for this statement
    // have to do this after removing implicit deps in case it will be
    // explicitly put back in
  
    std::set<SymHandle>::iterator useIter, defIter;
    for (useIter=useSet.begin(); useIter!=useSet.end(); useIter++) {
	SymHandle use = *useIter;
	for (defIter=defSet.begin(); defIter!=defSet.end(); defIter++) {
	    SymHandle def = *defIter;
	    insertEdge(use, def, CFLOW_EDGE, CallHandle(0), proc, proc, proc);

	    // build dependence matrix
	    setDepMatrix(proc, use, def);
	}
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
	collectIndependentSyms(procIter->current());
	collectDependentSyms  (procIter->current());
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
    return dug;
}




/*!
  Set the dependence matrix entry to 'true'
*/
void ManagerDUGStandard::setDepMatrix( 
    ProcHandle proc, 
    SymHandle  use, 
    SymHandle  def)
{
    mDUG->mapSymToProc(use, proc);
    mDUG->mapSymToProc(def, proc);
    if (use == def) return;
    mProcToSymSet[proc].insert(use);
    mProcToSymSet[proc].insert(def);
    mProcToMatrix[proc][use][def] = true;

#ifdef DEBUG_DUAA
    std::cout << "setDepMatrix: " << mIR->toString(proc) << ": " << mIR->toString(use);
    std::cout << " ---> ";
    std::cout << mIR->toString(def) << std::endl;
#endif
}




bool ManagerDUGStandard::hasEdgesToOtherProc(SymHandle sym, ProcHandle proc)
{
    // to filter out a fake parameter '.len' representing the length of 
    // a string parameter.
    if (!mDUG->isNode(sym, proc)) return false;
    OA_ptr<Node> node = mDUG->getNode(sym, proc);

    std::set<SymHandle> visited;
    return node->hasEdgesToOtherProc(proc, visited);
}




bool ManagerDUGStandard::hasEdgesFromOtherProc(SymHandle sym, ProcHandle proc)
{
    // to filter out a fake parameter '.len' representing the length of 
    // a string parameter.
    if (!mDUG->isNode(sym, proc)) return false;
    OA_ptr<Node> node = mDUG->getNode(sym, proc);

    std::set<SymHandle> visited;
    return node->hasEdgesFromOtherProc(proc, visited);
}




/*!
  'true' if 'sym' has an outgoing edge to other procedure
  'true' if 'sym' is defined inside and used outside.
*/
bool ManagerDUGStandard::isOutgoingToOtherProcs(
    SymHandle sym, ProcHandle proc )
{
    assert(mDUG->isNode(sym, proc));
    OA_ptr<Node> node = mDUG->getNode(sym, proc);

    bool definedInside = false;
    OA_ptr<EdgesIteratorInterface> predIterPtr
	= node->getDUGIncomingEdgesIterator();
    for (; predIterPtr->isValid() && !definedInside; ++(*predIterPtr)) {
	OA_ptr<EdgeInterface> predEdge = predIterPtr->currentDUGEdge();
	if (predEdge->getType() != CFLOW_EDGE &&
	    predEdge->getType() != RETURN_EDGE) continue;
  
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
  'true' if 'sym' has an incoming edge for other procedures and an
  outgoing edge for this procedure: 
  I.e., 'true' if sym is defined outside and used inside
*/
bool ManagerDUGStandard::isIncomingFromOtherProcs(
    SymHandle sym, ProcHandle proc )
{
    assert(mDUG->isNode(sym, proc));
    OA_ptr<Node> node = mDUG->getNode(sym, proc);

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
	if (succEdge->getType() != CFLOW_EDGE &&
	    succEdge->getType() != CALL_EDGE ) continue;
  
	if (succEdge->getProc() == proc) return true;
    }

    return false;
}




/*!
  'true' if there is a path from 'use' to 'def' through other procedures 
  than 'proc'. We traverse the VDG backward from 'def'.
*/
bool ManagerDUGStandard::isPathThruOtherProcs(
    SymHandle use, SymHandle def, ProcHandle proc )
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
    SymHandle use, SymHandle def, ProcHandle proc )
{
    std::map<SymHandle, std::map<SymHandle, bool> >&
	depMat = mProcToMatrix[proc];

    // Do nothing if 'use' is not used in other procs, 
    if (!isOutgoingToOtherProcs(use, proc)) return;

    // Do nothing if 'def' is not defined from other procs, 
    if (!isIncomingFromOtherProcs(def, proc)) return;

#ifdef DEBUG_DUAA
    std::cout << "setDepMatrix4Globals, checking Paths(" << mIR->toString(proc) <<
	"): " << mIR->toString(use) << " -> " << mIR->toString(def) << std::endl;
#endif
    // Do nothing if there is no path from 'use' to 'def',
    if (!isPathThruOtherProcs(use, def, proc)) return;

    depMat[use][def] = true;
#ifdef DEBUG_DUAA
    std::cout << "Value passing globals(" << mIR->toString(proc) <<
	"): " << mIR->toString(use) << " -> " << mIR->toString(def) << std::endl;
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
	edgesBetweenActuals(proc);
    }
}




bool ManagerDUGStandard::isLocal(SymHandle sym, ProcHandle proc)
{
    OA_ptr<Location> loc = mIR->getLocation(proc, sym);
    if (loc.ptrEqual(0)) return false;
    return loc->isLocal();
}



/*!
  Warshall's transitive closure algorithm on the dependence matrix 
  of one procedure. This makes edges between formal parameters too.
*/
void ManagerDUGStandard::transitiveClosure(ProcHandle proc)
{
    std::map<SymHandle, std::map<SymHandle, bool> >& depMat = mProcToMatrix[proc];

#ifdef DEBUG_DUAA
    std::cout << "*** transitiveClosure(" << mIR->toString(proc) 
	      << ") ***" << std::endl;
#endif
    std::set<SymHandle>& symSet = mProcToSymSet[proc];
    std::set<SymHandle>::iterator i, j, k;
    SymHandle use, def, var;

#ifdef THIS_IS_TOO_EXPENSIVE
    // set the dep matrix if both symbols are globals and
    // there is a path from 'use' to 'def' through other
    // procedures.
    for (k=symSet.begin(); k!=symSet.end(); k++){
	use = *k;
	if (isLocal(use, proc)) continue;
	for (i=symSet.begin(); i!=symSet.end(); i++){
	    def = *i;
	    if (isLocal(def, proc)) continue;
	    if (use != def && !depMat[use][def])
		setDepMatrix4Globals(use, def, proc);
	}
    }
#endif
    // transitive closure
    for (k=symSet.begin(); k!=symSet.end(); k++){
	var = *k;
	for (i=symSet.begin(); i!=symSet.end(); i++){
	    use = *i;
	    if (!depMat[use][var]) continue;
	    for (j=symSet.begin(); j!=symSet.end(); j++){
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
  This makes edges between actual parameters.
*/
void ManagerDUGStandard::edgesBetweenActuals(ProcHandle proc)
{
    std::set<CallHandle>& callsites = mProcToCallsiteSet[proc];
    std::set<CallHandle>::iterator callIter;

    std::set<SymHandle>& formals = mProcToFormalSet[proc];
    std::set<SymHandle>::iterator i, j;
  
    for (i=formals.begin(); i!=formals.end(); i++){
	SymHandle formal1 = *i;
	for (j=formals.begin(); j!=formals.end(); j++){
	    SymHandle formal2 = *j;
	    if (formal1 == formal2) continue;

	    //--------------------------------------------------
	    // - Value flow between formals through two global variables
	    //   not in this procedure
	    // - This is necessary for PARAM_EDGES
	    //--------------------------------------------------
	    if (!mProcToMatrix[proc][formal1][formal2]){
		if (!hasEdgesToOtherProc(formal1, proc)) continue;
		if (!hasEdgesFromOtherProc(formal2, proc)) continue;
		if (isPathThruOtherProcs(formal1, formal2, proc)) {
#ifdef DEBUG_DUAA
		    std::cout << "foundPath(" << mIR->toString(proc) << "): " << mIR->toString(formal1)
			      << " -> " << mIR->toString(formal2) << std::endl;
#endif
		    mProcToMatrix[proc][formal1][formal2] = true;
		}
	    }

	    if (mProcToMatrix[proc][formal1][formal2]){
		insertEdge(formal1, formal2, PARAM_EDGE, CallHandle(0), proc, proc, proc);

		for (callIter = callsites.begin(); callIter!=callsites.end(); callIter++){
		    CallHandle call = *callIter;
		    // get calling procedure
		    ProcHandle caller = mCallsiteToProc[call];
		    assert(caller != ProcHandle(0));
		    std::set<SymHandle>& set1 = mFormalToActualMap[call][formal1];
		    std::set<SymHandle>& set2 = mFormalToActualMap[call][formal2];
		    std::set<SymHandle>::iterator i1;
		    std::set<SymHandle>::iterator i2;
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



<<<<<<< .working
void IndepLocVisitor::visitNamedLoc(NamedLoc& loc)
{
    mDUG->insertIndepSymList(loc.getSymHandle(), mProc);
    mProcsOfInterest.insert(mProc);
=======
>>>>>>> .merge-right.r888

<<<<<<< .working
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




=======
>>>>>>> .merge-right.r888
  } // end of namespace MPDUG
} // end of namespace OA

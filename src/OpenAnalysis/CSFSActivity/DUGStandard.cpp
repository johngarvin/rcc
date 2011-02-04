/*! \file
  
\brief Implementation of DUGStandard.

\authors Michelle Strout
\version $Id: DUGStandard.cpp,v 1.3 2005/10/17 21:21:41 garvin Exp $

Copyright (c) 2002-2004, Rice University <br>
Copyright (c) 2004, University of Chicago <br>  
All rights reserved. <br>
See ../../../Copyright.txt for details. <br>
*/

//--------------------------------------------------------------------
//--------------------------------------------------------------------

// standard headers

#include <string.h>
#include <stdlib.h>
#include <iostream>
using std::ostream;
using std::endl;
using std::cout;
using std::cerr;

#include "DUGStandard.hpp"

namespace OA {
    namespace DUG {

//--------------------------------------------------------------------

//--------------------------------------------------------------------
// must be updated any time DUG::Interface::EdgeType changes
static const char *sEdgeTypeToString[] = { 
    "F",
    "C",
    "R",
    "P"
//   "CFLOW",
//   "CALL",
//   "RETURN",
//   "PARAM"
};
static const char *sNodeTypeToString[] = { 
    "FORMALPARAM_NODE",
    "NONEFORMAL_NODE",
};

//--------------------------------------------------------------------
/*! called by all Node constructors
 */
void Node::Ctor() { 

    mDGNode = new DGraph::NodeImplement;

    mVaried = false;
    mUseful = false;
    mSelfDependent = false;
#ifdef DEBUG_DUAA
    static unsigned nodeCnt=0;
    nodeCnt++;
    std::cout << "getId(" << getId() << ")" << std::endl;
    std::cout << "nodeCnt(" << nodeCnt << ")" << std::endl;
#endif
}


OA_ptr<EdgeInterface> EdgesIterator::currentDUGEdge() const
{
    return current().convert<Edge>();
}



OA_ptr<NodeInterface> NodesIterator::currentDUGNode() const
{
    return current().convert<Node>();
}




//--------------------------------------------------------------------
void
Node::dump (ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    os << sNodeTypeToString[mType] << std::endl;
}

bool Node::operator==(DGraph::NodeInterface& other) 
{
    
    return mDGNode->operator==(other);
}

bool Node::operator<(DGraph::NodeInterface& other) 
{
    return mDGNode->operator<(other);
}



void
Node::longdump (ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    // print the node ID
    os << "DUGStandard Node: ";
    dump(os,ir);
  
    if (isAnEntry()) {
	os << " (ENTRY)";
    } else if (isAnExit()) {
	os << " (EXIT)"; 
    }
    os << endl;
  
    // print the source(s)
    unsigned int count = 0;
    OA_ptr<NodesIteratorInterface> srcIter = getDUGSourceNodesIterator();
    for ( ; srcIter->isValid(); ++(*srcIter), ++count) {
	OA_ptr<NodeInterface> node = srcIter->currentDUGNode();
	if (count == 0) { os << " <-- ("; }
	else            { os << ", "; }

	node->dump(os,ir);
    }
    if (count > 0) { os << ")" << endl; }
  
    // print the sink(s)
    count = 0;
    OA_ptr<NodesIteratorInterface> outIter = getDUGSinkNodesIterator();
    for ( ; outIter->isValid(); ++(*outIter), ++count) {
	OA_ptr<NodeInterface> node = outIter->currentDUGNode();
	if (count == 0) { os << " --> ("; } 
	else            { os << ", "; }
    
	node->dump(os,ir);
    }
    if (count > 0) { os << ")" << endl; }
}

void
Node::dumpdot (ostream& os, OA_ptr<DUGIRInterface> ir)
{
    SymHandle sym   = getSym();
    IRHandle  ih    = getDef(); 
    std::string str = ir->toString(sym);

    char buf1[20], buf2[20];
    sprintf(buf1, "(%u)\\n", getId());
    sprintf(buf2, "@%u", ih.hval());

    std::string::size_type loc = str.find( "::", 0);
    if( loc != std::string::npos ) str.erase(loc, 2);

    str.insert(loc, std::string(buf1));
    str.append(std::string(buf2));

    OA_ptr<Location> memLoc = mDUG->mIR->getLocation(mProc, sym);
    if (memLoc.ptrEqual(0)){
#ifdef DEBUG_DUAA
	std::cout << "^^^^^ NULL LOCATION ^^^^^" << std::endl;
	std::cout << "\tmProc(" << mDUG->mIR->toString(mProc);
	std::cout << "), sym(" << mDUG->mIR->toString(sym) 
		  << ")" << std::endl;
	if (mDef == IRHandle(0) || mDef == IRHandle(1) || mDef == IRHandle(2))
	    std::cout << "stmt(012)" << std::endl;
	else
	    std::cout << "stmt(" << mDUG->mIR->toString((StmtHandle)mDef)
		      << ")" << std::endl;
#endif
    }
    else if (!memLoc->isLocal()) {
	str.erase(0, loc);
	str.insert(0, std::string("JWGLOBAL"));
    }

    os << "\"" << str << "\"";
}

//--------------------------------------------------------------------

OA_ptr<DGraph::EdgesIteratorInterface> 
Node::getIncomingEdgesIterator() const
{
    return mDGNode->getIncomingEdgesIterator();
}

OA_ptr<DGraph::EdgesIteratorInterface> 
Node::getOutgoingEdgesIterator() const
{
    return mDGNode->getOutgoingEdgesIterator();
}

OA_ptr<DGraph::NodesIteratorInterface> 
Node::getSourceNodesIterator() const
{
    return mDGNode->getSourceNodesIterator();
}

OA_ptr<DGraph::NodesIteratorInterface> 
Node::getSinkNodesIterator() const
{
    return mDGNode->getSourceNodesIterator();
}

OA_ptr<EdgesIteratorInterface>
Node::getDUGIncomingEdgesIterator() const
{
    OA_ptr<EdgesIterator> retval;
    retval = new EdgesIterator(getIncomingEdgesIterator());
    return retval;

}

OA_ptr<EdgesIteratorInterface>
Node::getDUGOutgoingEdgesIterator() const
{
    OA_ptr<EdgesIterator> retval;
    retval = new EdgesIterator(getOutgoingEdgesIterator());
    return retval;

}

OA_ptr<NodesIteratorInterface>
Node::getDUGSourceNodesIterator() const
{
    OA_ptr<NodesIterator> retval;
    retval = new NodesIterator(getSourceNodesIterator());
    return retval;

}

OA_ptr<NodesIteratorInterface>
Node::getDUGSinkNodesIterator() const
{
    OA_ptr<NodesIterator> retval;
    retval = new NodesIterator(getSinkNodesIterator());
    return retval;

}



//--------------------------------------------------------------------
//--------------------------------------------------------------------
Edge::Edge (OA_ptr<DUGStandard> pDUG,
	    OA_ptr<Node> pNode1, 
	    OA_ptr<Node> pNode2, 
	    EdgeType pType, CallHandle call,
	    ProcHandle proc)
    : mDUG(pDUG), mNode1(pNode1), mNode2(pNode2), mType(pType), 
      mCall(call), mProc(proc)
{

    // create DGraphEdge for associated DGraph
    mDGEdge = new DGraph::EdgeImplement(pNode1->mDGNode,
					pNode2->mDGNode);

}


//--------------------------------------------------------------------
bool Edge::operator==(DGraph::EdgeInterface& other) 
{
    
    return mDGEdge->operator==(other);
}

bool Edge::operator<(DGraph::EdgeInterface& other) 
{
    return mDGEdge->operator<(other);
}



void Edge::dump(ostream& os)
{
    os << sEdgeTypeToString[mType];
}

void filterStr(std::string& s)
{
    std::string::size_type loc = s.find( "::", 0);
    if( loc != std::string::npos ) s.erase(0, loc+2);
}


void Edge::dumpdot(ostream& os, OA_ptr<DUGIRInterface> ir)
{
    OA_ptr<NodeInterface> srcNode  = getDUGSource();
    OA_ptr<NodeInterface> sinkNode = getDUGSink();

    srcNode->dumpdot(os, ir);
    os << "->";
    sinkNode->dumpdot(os, ir);
    dumpdot_label(os, ir);
}



void Edge::dumpdot_label(ostream& os, OA_ptr<DUGIRInterface> ir)
{
    CallHandle call = getCall();
    EdgeType etype = getType();
    os << "[label=\"" << sEdgeTypeToString[etype];
    if (call != CallHandle(0)) os << "(" << call.hval() << ")";
    os << "\\n";

    ProcHandle proc = getProc();
    std::string procStr = ir->toString(proc);
    filterStr(procStr);
    os << procStr;

    if (call != CallHandle(0)){
	if (etype == CALL_EDGE)
	    os << "\\n-> ";
	else{
	    assert(etype == RETURN_EDGE);
	    os << "\\n<- ";
	}
	SymHandle calleeSym = ir->getSymHandle(call);
	procStr = ir->toString(calleeSym);
	filterStr(procStr);
	os << procStr;
	os << "\", style=dashed];" << endl;
    }
    else
	os << "\"];" << endl;
}


//--------------------------------------------------------------------
//--------------------------------------------------------------------
DUGStandard::DUGStandard(
    OA_ptr<DUGIRInterface> pIR,
    OA_ptr<DataFlow::ParamBindings> pParamBind)
    : mIR(pIR), mParamBind(pParamBind)
{
    mEntry       = mExit = NULL;
    mDGraph      = new DGraph::DGraphImplement;
    mCallNodes   = new std::list<OA_ptr<Node> >;
    mReturnNodes = new std::list<OA_ptr<Node> >;
    mActiveSymSet    = new std::set<SymHandle>;
    mActiveMemRefSet = new std::set<MemRefHandle>;
    mActiveStmtSet   = new std::set<StmtHandle>;
    mUnknownLocActive = false;
}


DUGStandard::~DUGStandard()
{
    mEntry = NULL;
    mExit = NULL;
}

//--------------------------------------------------------------------
// DUGStandard Methods
//--------------------------------------------------------------------

OA_ptr<Edge> 
DUGStandard::getDUGEdge(
    const OA_ptr<DGraph::EdgeInterface> dgEdge) const
{

    
    assert (0); 
    OA_ptr<Edge> retval;
    return retval;
}

OA_ptr<Node> 
DUGStandard::getDUGNode(
    const OA_ptr<DGraph::NodeInterface> dgNode) const
{
    assert(0);   
    OA_ptr<Node> retval;
    return retval; 
}

void DUGStandard::addEdge(OA_ptr<DGraph::EdgeInterface> pEdge)
{
    mDGraph->addEdge(pEdge);
}

void DUGStandard::addNode(OA_ptr<DGraph::NodeInterface> pNode)
{
    
    mDGraph->addNode(pNode);
}


OA_ptr<DGraph::NodesIteratorInterface> DUGStandard::getNodesIterator() const
{
    return mDGraph->getNodesIterator();
}

OA_ptr<DGraph::NodesIteratorInterface>
DUGStandard::getEntryNodesIterator( ) const
{
    
    return mDGraph->getEntryNodesIterator();
}

OA_ptr<DGraph::NodesIteratorInterface>
DUGStandard::getExitNodesIterator( ) const
{
    return mDGraph->getExitNodesIterator();
}


OA_ptr<DGraph::EdgesIteratorInterface> DUGStandard::getEdgesIterator() const
{
    
    return mDGraph->getEdgesIterator();
}

OA_ptr<DGraph::NodesIteratorInterface> DUGStandard::getDFSIterator(OA_ptr<DGraph::NodeInterface> n) 
{
    
    
    return mDGraph->getDFSIterator(n);
}

OA_ptr<DGraph::NodesIteratorInterface> DUGStandard::getBFSIterator() 
{ 
    assert(0); 
    OA_ptr<DGraph::NodesIteratorInterface> retval; 
    return retval;
}


OA_ptr<DGraph::NodesIteratorInterface>
DUGStandard::getReversePostDFSIterator( DGraph::DGraphEdgeDirection pOrient)
{
    return mDGraph->getReversePostDFSIterator(pOrient);
}

OA_ptr<DGraph::NodesIteratorInterface>
DUGStandard::getReversePostDFSIterator(OA_ptr<DGraph::NodeInterface> root, 
				       DGraph::DGraphEdgeDirection pOrient)
{
    assert(0);
}

/*! FIXME: for now just returning a ReversePostDFSIterator that
  starts at exit.  For data-flow analysis this is actually better
  but DGraphDFProblem has to get the PostDFSIterator because
  not all graphs (eg. callgraph) have an exit node.
*/
OA_ptr<DGraph::NodesIteratorInterface> 
DUGStandard::getPostDFSIterator(DGraph::DGraphEdgeDirection pOrient)
{
    assert(0);
}

OA_ptr<DGraph::NodesIteratorInterface>
DUGStandard::getPostDFSIterator(OA_ptr<DGraph::NodeInterface> root, 
				DGraph::DGraphEdgeDirection pOrient)
{
    assert(0);
}


OA_ptr<NodesIteratorInterface> DUGStandard::getDUGNodesIterator() const
{
    OA_ptr<NodesIterator> retval;
    retval = new NodesIterator(getNodesIterator());
    return retval;

}


OA_ptr<NodesIteratorInterface>
DUGStandard::getDUGEntryNodesIterator( ) const
{
    OA_ptr<NodesIterator> retval;
    retval = new NodesIterator(getEntryNodesIterator());
    return retval;


}

OA_ptr<NodesIteratorInterface>
DUGStandard::getDUGExitNodesIterator( ) const
{
    OA_ptr<NodesIterator> retval;
    retval = new NodesIterator(getExitNodesIterator());
    return retval;


}

OA_ptr<EdgesIteratorInterface> DUGStandard::getDUGEdgesIterator() const
{
    OA_ptr<EdgesIterator> retval;
    retval = new EdgesIterator(getEdgesIterator());
    return retval;


}


OA_ptr<NodesIteratorInterface>
DUGStandard::getDUGReversePostDFSIterator( DGraph::DGraphEdgeDirection pOrient)
{
    OA_ptr<NodesIterator> retval;
    retval = new NodesIterator(getReversePostDFSIterator(pOrient));
    return retval;
}

OA_ptr<NodesIteratorInterface>
DUGStandard::getDUGDFSIterator(OA_ptr<NodeInterface> n)
{
    OA_ptr<NodesIterator> retval;
    retval = new NodesIterator(getDFSIterator(n));
    return retval;


}

void
DUGStandard::dump (ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    os << "===== DUGStandard: =====\n"
       << endl;
  
    // print the contents of all the nodes
    OA_ptr<NodesIteratorInterface> nodeIter = getDUGNodesIterator();
    for ( ; nodeIter->isValid(); ++(*nodeIter)) {
	OA_ptr<NodeInterface> node = nodeIter->currentDUGNode();
	node->longdump(os, ir);
	os << endl;
    }
  
    os << "====================" << endl;

}



void
DUGStandard::dumpdot(ostream& os, OA_ptr<DUGIRInterface> ir)
{

    os << "digraph OA_DUG {" << endl;
    os << "node [shape=rectangle];" << endl;

    // then output nodes and edges by procedure
    OA_ptr<EdgesIteratorInterface> iter;
    iter = getDUGEdgesIterator();
    for (; iter->isValid(); (*iter)++ ) {
	OA_ptr<EdgeInterface> edge = iter->currentDUGEdge();
	edge->dumpdot(os, ir);
    }

    os << "}" << endl;
    os.flush();
}

OA_ptr<Node> 
DUGStandard::getNode(IRSymHandle ish, ProcHandle proc){
    
    IRHandle ih = ish.first;
    SymHandle sym = ish.second;
    assert(sym);
#ifdef DEBUG_DUAA
    std::cout << "getNode: " << mIR->toString(sym) << "("
	      << sym.hval() << ")@" << mIR->toString(proc) << "("
	      << proc.hval() << ") --- ";
#endif
    IRSymHandle key(ih, sym);

    OA_ptr<Node> retval = mSymToNode[key];
    if (!retval.ptrEqual(0)) {
#ifdef DEBUG_DUAA
	std::cout << "found itself" << std::endl;
#endif
	return retval;
    }
    OA_ptr<Location> loc = mIR->getLocation(proc, sym);
    assert (!loc.ptrEqual(0));
    OA_ptr<NamedLoc> nloc = loc.convert<NamedLoc>();
    OA_ptr<SymHandleIterator> symIter = nloc->getFullOverlapIter();
    for ( ; symIter->isValid(); (*symIter)++) {
	retval = mSymToNode[IRSymHandle(ih,symIter->current())];
	if (!retval.ptrEqual(0)) {
#ifdef DEBUG_DUAA
	    std::cout << "found FullOverlap" << std::endl;
#endif
	    return retval;
	}
    }
#if 0
    symIter = nloc->getPartOverlapIter();
    for ( ; symIter->isValid(); (*symIter)++) {
	retval = mSymToNode[IRSymHandle(ih,symIter->current())];
	if (!retval.ptrEqual(0)) {
#ifdef DEBUG_DUAA
	    std::cout << "found PartOverlap" << std::endl;
#endif
	    return retval;
	}
    }
#endif

    assert (retval.ptrEqual(0));
    OA_ptr<DUGStandard> temp; temp = this;
    retval = new Node(temp, proc, ih, sym);
    assert(!retval.ptrEqual(0));
    mSymToNode[key] = retval;
    addNode(retval);
#ifdef DEBUG_DUAA
    std::cout << "added" << std::endl;
#endif
    if (isIndependent(proc, sym) || isDependent(proc, sym))
	mInDepSymToNodes[sym].insert(retval);

    return retval;
}

// true if a node exists for 'loc'
bool
DUGStandard::isNode(IRSymHandle ish, ProcHandle proc){
    IRHandle ih = ish.first;
    SymHandle sym = ish.second;
    assert(sym);
#ifdef DEBUG_DUAA
    std::cout << "isNode: " << mIR->toString(sym) << "("
	      << sym.hval() << ")@" << mIR->toString(proc) << "("
	      << proc.hval() << ") --- ";
#endif
    std::pair<IRHandle, SymHandle> key(ih, sym);
    OA_ptr<Node> retval = mSymToNode[key];

    if (!retval.ptrEqual(0)) {
#ifdef DEBUG_DUAA
	std::cout << "found itself" << std::endl;
#endif
	return true;
    }

    OA_ptr<Location> loc = mIR->getLocation(proc, sym);
    if (loc.ptrEqual(0)) {
#ifdef DEBUG_DUAA
	std::cout << "NULL location" << std::endl;
#endif
	return false;
    }
    OA_ptr<NamedLoc> nloc = loc.convert<NamedLoc>();
    OA_ptr<SymHandleIterator> symIter = nloc->getFullOverlapIter();
    for ( ; symIter->isValid(); (*symIter)++) {
	retval = mSymToNode[IRSymHandle(ih,symIter->current())];
	if (!retval.ptrEqual(0)) {
#ifdef DEBUG_DUAA
	    std::cout << "found FullOverlap" << std::endl;
#endif
	    return true;
	}
    }
#if 0
    symIter = nloc->getPartOverlapIter();
    for ( ; symIter->isValid(); (*symIter)++) {
	retval = mSymToNode[IRSymHandle(ih,symIter->current())];
	if (!retval.ptrEqual(0)) {
#ifdef DEBUG_DUAA
	    std::cout << "found PartOverlap" << std::endl;
#endif
	    return true;
	}
    }
#endif
#ifdef DEBUG_DUAA
    std::cout << "not found" << std::endl;
#endif
    return false;
}

void 
DUGStandard::insertActiveSymSet(OA_ptr<Location> loc)
{
    // Unknown
    if (loc->isaUnknown()) {

	mUnknownLocActive = true;

	// Named
    } else if (loc->isaNamed()) {
      
	OA_ptr<NamedLoc> nloc = loc.convert<NamedLoc>();
#ifdef DEBUG_DUAA
	std::cout << "DUGStandard::insertActiveSymSet:(loc)"; 
	loc->dump(std::cout, mIR);
	std::cout << std::endl;
#endif  
	insertActiveSymSet( nloc->getSymHandle() );

	OA_ptr<SymHandleIterator> symIter = nloc->getFullOverlapIter();
	for ( ; symIter->isValid(); (*symIter)++) {
#ifdef DEBUG_DUAA
	    std::cout << "\tFullOverlap: " << mIR->toString(symIter->current()); 
	    std::cout << std::endl;
#endif  
	    insertActiveSymSet( symIter->current());
	}
	symIter = nloc->getPartOverlapIter();
	for ( ; symIter->isValid(); (*symIter)++) {
#ifdef DEBUG_DUAA
	    std::cout << "\tPartialOverlap: "; 
	    std::cout << std::endl;
#endif  
	    insertActiveSymSet(symIter->current());
	}
 
	// Unnamed
    } else if (loc->isaUnnamed()) {
    
	//assert(0);
	// not handling this yet

	// Invisible
    } else if (loc->isaInvisible()) {

	OA_ptr<InvisibleLoc> invLoc 
	    = loc.convert<InvisibleLoc>();
	OA_ptr<MemRefExpr> mre = invLoc->getMemRefExpr();

	// get symbol from memory reference expression if no derefs
	// FIXME: here need another visitor for MemRefExpr, for now assuming
	// only named ones
	if (mre->isaNamed()) {
	    OA_ptr<NamedRef> namedRef 
		= namedRef.convert<NamedRef>();
	    insertActiveSymSet( namedRef->getSymHandle() );
	} else {
	    assert(0);
	}

	// LocSubSet
    } else if (loc->isaSubSet()) {

	OA_ptr<Location> baseLoc = loc->getBaseLoc();
	// FIXME: now really want to call visitor on this guy but instead will just
	// call this function recursively
	insertActiveSymSet(baseLoc);

    } else {

	assert(0);
    }
}


void 
Node::setActive(SymHandle sym)
{
    assert(mProc);
    // will be storing activity results for stmt and memrefs in
    // ActiveStandard for current procedure
    if (mDUG->mActiveMap[mProc].ptrEqual(0)) {
	mDUG->mActiveMap[mProc] = new OA::Activity::ActiveStandard(mProc);
    }

    mDUG->insertActiveSymSet(sym);

    std::set<MemRefHandle> mrefSet;
    mrefSet = mDUG->getMemRefSet(sym);
    if (!mrefSet.empty()){
	std::set<MemRefHandle>::iterator i = mrefSet.begin();
	for(; i != mrefSet.end(); i++) {
	    mDUG->insertActiveMemRefSet(*i, mProc);
	}
    }


    std::set<StmtHandle> stmtSet;
    stmtSet = mDUG->getStmtSet(sym);
    if (!stmtSet.empty()){
	std::set<StmtHandle>::iterator i = stmtSet.begin();
	for(; i != stmtSet.end(); i++)
	    mDUG->insertActiveStmtSet(*i, mProc);
    }

}


void 
Node::setActive()
{
    setActive(mSym);

    OA_ptr<Location> loc = mDUG->mIR->getLocation(getProc(), mSym);
    OA_ptr<NamedLoc> nmloc = loc.convert<NamedLoc>();
    assert(!nmloc.ptrEqual(0));
    OA_ptr<SymHandleIterator> overlapIter = nmloc->getFullOverlapIter();
    for ( ; overlapIter->isValid(); (*overlapIter)++ ) {
	SymHandle sym = overlapIter->current();
	setActive(sym);
    }

    overlapIter = nmloc->getPartOverlapIter();
    for ( ; overlapIter->isValid(); (*overlapIter)++ ) {
	SymHandle sym = overlapIter->current();
	setActive(sym);
    }
}


void 
Node::markVaried(std::list<CallHandle>& callStack,
		 OA_ptr<Activity::ActivityIRInterface> ir,
		 std::set<OA_ptr<EdgeInterface> >& visited,
		 std::set<std::pair<unsigned,unsigned> >& onPath,
		 ProcHandle proc,
		 SymHandle prevSym, 
		 OA_ptr<EdgeInterface> parenEdge)
{ 
    SymHandle currSym = getSym();

    EdgeType parenEType = CFLOW_EDGE;
    CallHandle parenCall = CallHandle(0);
    if (!parenEdge.ptrEqual(0)) {
	parenEType = parenEdge->getType();
	parenCall = parenEdge->getCall();
    }


#ifdef DEBUG_DUAA
    std::cout << "-v->"; 
    if (parenEdge.ptrEqual(0)) {
	std::cout << "NULL edge";
    }else
	parenEdge->dumpdot(std::cout, mDUG->mIR);
    std::cout << std::endl;
#endif  
    bool isVariedBefore = isVaried();
    int nonParentSuccessors = 0;
    setVaried();

    bool valueThroughGlobals = false;
    if (callStack.front() == CallHandle(1)) valueThroughGlobals = true;

    // set up iterator for successor edges
    OA_ptr<EdgesIteratorInterface> succIterPtr;
    succIterPtr = getDUGOutgoingEdgesIterator();
    // iterate over the successors
    for (; succIterPtr->isValid(); ++(*succIterPtr)) {

	OA_ptr<EdgeInterface> succEdge = succIterPtr->currentDUGEdge();
	OA_ptr<NodeInterface> succNode = succEdge->getDUGSink();

	SymHandle succSym = succNode->getSym();
	unsigned succId = succNode->getId();
	EdgeType etype = succEdge->getType();

	std::pair<unsigned, unsigned> pathNode;
	switch(etype) {
	    case CALL_EDGE: 
		pathNode = std::pair<unsigned,unsigned>
		    (succEdge->getCall().hval(),succId); break;
	    case PARAM_EDGE:
		if (callStack.empty())
		    pathNode = std::pair<unsigned,unsigned>(1,succId); 
		else
		    pathNode = std::pair<unsigned,unsigned>
			(callStack.front().hval(),succId); break;
	    case RETURN_EDGE:
	    case CFLOW_EDGE:
		pathNode = std::pair<unsigned,unsigned>(1,succId); break;
	    default: assert(0);
	}

	if (succSym != prevSym || parenCall != succEdge->getCall()) nonParentSuccessors++;

	if (visited.find(succEdge) != visited.end() ||
	    onPath.find(pathNode)  != onPath.end()  ) 
	{
	    continue;
	}

	onPath.insert(pathNode);

	switch(etype) {
	    case (CALL_EDGE):
	    {
		// to deal with value passing through global variables between procedures
		ProcHandle callerProc = succEdge->getSourceProc();
		if (proc != callerProc){ 
#ifdef DEBUG_DUAA
		    std::cout << "valthruglobals: begin" << std::endl;
#endif  
		    callStack.push_front(CallHandle(1));
		}
          
		callStack.push_front(succEdge->getCall());
		visited.insert(succEdge);
        
		succNode->markVaried(callStack, ir, visited, onPath, 
				     succEdge->getSinkProc(), currSym, 
				     succEdge);
		callStack.pop_front();
		if (proc != callerProc){ 
#ifdef DEBUG_DUAA
		    std::cout << "valthruglobals: end" << std::endl;
#endif  
		    callStack.pop_front();
		}
		break;
	    }
	    case (RETURN_EDGE):
		if (callStack.front() == succEdge->getCall() || valueThroughGlobals){
		    if (!valueThroughGlobals) callStack.pop_front();
		    visited.insert(succEdge);

		    succNode->markVaried(callStack, ir, visited, onPath, 
					 succEdge->getProc(), currSym, 
					 succEdge);
		    if (!valueThroughGlobals) callStack.push_front(succEdge->getCall());
		}
#ifdef DEBUG_DUAA
		else{
		    std::cout << "markVaried: stackTop(" << (unsigned)callStack.front().hval() 
			      << ") <-> edgeCallExp("
			      << (unsigned)succEdge->getCall().hval() << ")" << std::endl;
		}
#endif  
		break;
	    default: // for both CFLOW_EDGE and PARAM_EDGE
		if (succEdge->getType() != PARAM_EDGE) 
		    visited.insert(succEdge);
		// value passing through global variables between procedures
		ProcHandle succProc = succEdge->getProc();
		if (proc != succProc) {
		    callStack.push_front(CallHandle(1));
          
		    succNode->markVaried(callStack, ir, visited, onPath, 
					 succProc, currSym, succEdge);
		    callStack.pop_front();
		}
		else{
		    succNode->markVaried(callStack, ir, visited, onPath, 
					 proc, currSym, succEdge);
		}
		break;
	}

	onPath.erase(pathNode);
    }
    // Actual or formal parameters without any outgoing edges shouldn't be
    // activated.
    if (nonParentSuccessors == 0 && !isVariedBefore && !isSelfDependent() && 
	( parenEType == CALL_EDGE || parenEType == RETURN_EDGE) && 
	!mDUG->isDependent(getProc(), getSym()) ){
	unsetVaried();
#ifdef DEBUG_DUAA
	std::cout << "unsetVaried("; dumpdot(std::cout, mDUG->mIR); 
	std::cout << ")" << std::endl;
#endif  
    }
#ifdef DEBUG_DUAA
    std::cout << "<-" << std::endl;
#endif  
}

void
Node::markUseful(std::list<CallHandle>& callStack,
		 OA_ptr<Activity::ActivityIRInterface> ir,
		 std::set<OA_ptr<EdgeInterface> >& visited,
		 std::set<std::pair<unsigned,unsigned> >& onPath,
		 ProcHandle proc,
		 SymHandle prevSym, 
		 OA_ptr<EdgeInterface> parenEdge)
{
    if (!isVaried()) {
	return;
    }
    SymHandle currSym = getSym();

    EdgeType parenEType = CFLOW_EDGE;
    CallHandle parenCall = CallHandle(0);
    if (!parenEdge.ptrEqual(0)) {
	parenEType = parenEdge->getType();
	parenCall = parenEdge->getCall();
    }

#ifdef DEBUG_DUAA
    std::cout << "-u->"; 
    if (parenEdge.ptrEqual(0)) {
	std::cout << "NULL edge";
    }else
	parenEdge->dumpdot(std::cout, mDUG->mIR);
    std::cout << "(call:" << parenCall.hval() << ")" << std::endl; 
#endif  
    bool isUsefulBefore = isUseful();
    setUseful();
    int nonChildAncestors = 0;

    bool valueThroughGlobals = false;
    if (callStack.front() == CallHandle(1)) valueThroughGlobals = true;

    // set up iterator for predecessor edges
    OA_ptr<EdgesIteratorInterface> predIterPtr;
    predIterPtr = getDUGIncomingEdgesIterator();
    // iterate over the predecessors unless 'this' node is for indenepdent variable
    // we assume no independent variables are dependent on another independent variables
    for (; predIterPtr->isValid(); ++(*predIterPtr)) {
      
	OA_ptr<EdgeInterface> predEdge = predIterPtr->currentDUGEdge();
	OA_ptr<NodeInterface> predNode = predEdge->getDUGSource();

	SymHandle predSym = predNode->getSym();
	unsigned predId = predNode->getId();
	EdgeType etype = predEdge->getType();

	std::pair<unsigned, unsigned> pathNode;
	switch(etype) {
	    case RETURN_EDGE:
		pathNode = std::pair<unsigned,unsigned>
		    (predEdge->getCall().hval(),predId); break;
	    case PARAM_EDGE:
		if (callStack.empty())
		    pathNode = std::pair<unsigned,unsigned>(1,predId); 
		else
		    pathNode = std::pair<unsigned,unsigned>
			(callStack.front().hval(),predId); break;
	    case CALL_EDGE: 
	    case CFLOW_EDGE:
		pathNode = std::pair<unsigned,unsigned>
		    (1,predId); break;
	    default: assert(0);
	}
	if (predSym != prevSym || parenCall != predEdge->getCall()) nonChildAncestors++;
	if (visited.find(predEdge) != visited.end() ||
	    onPath.find(pathNode)  != onPath.end()  ) continue;
	onPath.insert(pathNode);

	switch(etype) {
	    case (CALL_EDGE):
		if (callStack.front() == predEdge->getCall() || valueThroughGlobals ){
		    if (!valueThroughGlobals) callStack.pop_front();
		    visited.insert(predEdge);
		    predNode->markUseful(callStack, ir, visited, onPath, 
					 predEdge->getProc(), currSym, predEdge);
		    if (!valueThroughGlobals) callStack.push_front(predEdge->getCall());
		}
		break;
	    case (RETURN_EDGE):
	    {
		ProcHandle callerProc = predEdge->getSinkProc();
		if (proc != callerProc){ 
		    callStack.push_front(CallHandle(1));
#ifdef DEBUG_DUAA
		    std::cout << "valthruglobals: begin" << std::endl;
#endif  
		}

		callStack.push_front(predEdge->getCall());
		visited.insert(predEdge);
		predNode->markUseful(callStack, ir, visited, onPath, 
				     predEdge->getSourceProc(), 
				     currSym, predEdge);
		callStack.pop_front();
		if (proc != callerProc){ 
		    callStack.pop_front();
#ifdef DEBUG_DUAA
		    std::cout << "valthruglobals: end" << std::endl;
#endif  
		}
		break;
	    }
	    default: // for both CFLOW_EDGE and PARAM_EDGE
		if (predEdge->getType() != PARAM_EDGE) visited.insert(predEdge);
		ProcHandle predProc = predEdge->getProc();
		if (proc != predProc) {
		    callStack.push_front(CallHandle(1));
#ifdef DEBUG_DUAA
		    std::cout << "valthruglobals: begin" << std::endl;
#endif  
		    predNode->markUseful(callStack, ir, visited, onPath, 
					 predProc, currSym, predEdge);
		    callStack.pop_front();
#ifdef DEBUG_DUAA
		    std::cout << "valthruglobals: end" << std::endl;
#endif  
		}
		else
		{
		    predNode->markUseful(callStack, ir, visited, onPath, 
					 proc, currSym, predEdge);
		}
		break;
	}

	onPath.erase(pathNode);
    }

    unsigned currId = getId();
    // Formal parameters without any outgoing edges shouldn't be
    // activated.
    if (nonChildAncestors == 0 && !isUsefulBefore && !isSelfDependent() && 
	( parenEType == RETURN_EDGE || parenEType == CALL_EDGE) &&
	!mDUG->isIndependent(getProc(), getSym()) ){
#ifdef DEBUG_DUAA
	std::cout << "unsetUseful(" << currId << ")" << std::endl;
#endif  
	unsetUseful();
    }else{
#ifdef DEBUG_DUAA
	std::cout << "setActive(" << currId << ")" << std::endl;
#endif  
	setActive();
    }

#ifdef DEBUG_DUAA
    std::cout << "<-" << std::endl;
#endif  
}

//! Indicate whether the given sym is active or not
bool DUGStandard::isActive(SymHandle sym)
{
    // an unknown location is active, therefore all symbols are active
    if (mUnknownLocActive) {
	return true;
    } else if (mActiveSymSet->find(sym) != mActiveSymSet->end()) {
	return true;
    } else {
	return false;
    }  
}



//! Indicate whether the given memref is active or not
bool DUGStandard::isActive(MemRefHandle memref)
{
    if (mActiveMemRefSet->find(memref) != mActiveMemRefSet->end()) {
	return true;
    } else {
	return false;
    }
}



// 'true' if there is a path from 'useNode' to 'this'
bool
Node::isPathFrom(OA_ptr<NodeInterface> useNode,
		 std::set<OA_ptr<NodeInterface> >& visited)
{ 
    if (useNode.ptrEqual(this)) return true;

    // traverse backward from this nodes
    OA_ptr<EdgesIteratorInterface> predIterPtr
	= getDUGIncomingEdgesIterator();
    for (; predIterPtr->isValid(); ++(*predIterPtr)) {
	OA_ptr<EdgeInterface> predEdge = predIterPtr->currentDUGEdge();
	OA_ptr<NodeInterface> predNode = predEdge->getDUGSource();
  
	if (visited.find(predNode) != visited.end()) continue;
	visited.insert(predNode);
	if (predNode->isPathFrom(useNode, visited)) return true;
    }

    return false;
}



// true if this has an outgoing edge to other procedures
bool
Node::hasOutgoingEdgesToOtherProcs(ProcHandle proc)
{ 
    OA_ptr<EdgesIteratorInterface> succIterPtr
	= getDUGOutgoingEdgesIterator();
    for (; succIterPtr->isValid(); ++(*succIterPtr)) {
	OA_ptr<EdgeInterface> succEdge = succIterPtr->currentDUGEdge();
	if (succEdge->getType() != CFLOW_EDGE) continue;
	if (succEdge->getProc() != proc) return true;
    }

    return false;
}



// returns a set of outgoing nodes of this proc
void
Node::findOutgoingNodes(ProcHandle proc, 
			std::set<IRSymHandle>& OutGoingNodeSet,
			std::set<IRSymHandle>& visited)
{ 
    IRSymHandle nodeKey = getIRSymHandle();
    if (visited.find(nodeKey) != visited.end()) return;
    visited.insert(nodeKey);

    if (hasOutgoingEdgesToOtherProcs(proc)) 
	OutGoingNodeSet.insert(nodeKey);

    // traverse foreward from this nodes
    OA_ptr<EdgesIteratorInterface> succIterPtr
	= getDUGOutgoingEdgesIterator();
    for (; succIterPtr->isValid(); ++(*succIterPtr)) {
	OA_ptr<EdgeInterface> succEdge = succIterPtr->currentDUGEdge();
	OA_ptr<NodeInterface> succNode = succEdge->getDUGSink();

	if (succEdge->getType() != CFLOW_EDGE) continue;
	if (succEdge->getProc() != proc) continue;
  
	succNode->findOutgoingNodes(proc, OutGoingNodeSet, visited);
    }
}



// true if this has an incoming edge to other procedures
bool
Node::hasIncomingEdgesFromOtherProcs(ProcHandle proc)
{ 
    OA_ptr<EdgesIteratorInterface> predIterPtr
	= getDUGIncomingEdgesIterator();
    for (; predIterPtr->isValid(); ++(*predIterPtr)) {
	OA_ptr<EdgeInterface> predEdge = predIterPtr->currentDUGEdge();
	if (predEdge->getType() != CFLOW_EDGE) continue;
	if (predEdge->getProc() != proc) return true;
    }
    return false;
}



// returns a set of incoming nodes of this proc
void
Node::findIncomingNodes(ProcHandle proc, 
			std::set<IRSymHandle>& InComingNodeSet,
			std::set<IRSymHandle>& visited)
{ 
    IRSymHandle nodeKey = getIRSymHandle();
    if (visited.find(nodeKey) != visited.end()) return;
    visited.insert(nodeKey);

    if (hasIncomingEdgesFromOtherProcs(proc)) 
	InComingNodeSet.insert(nodeKey);

    // traverse backward from this nodes
    OA_ptr<EdgesIteratorInterface> predIterPtr = getDUGIncomingEdgesIterator();
    for (; predIterPtr->isValid(); ++(*predIterPtr)) {
	OA_ptr<EdgeInterface> predEdge = predIterPtr->currentDUGEdge();
	OA_ptr<NodeInterface> predNode = predEdge->getDUGSource();
  
	if (predEdge->getType() != CFLOW_EDGE) continue;
	if (predEdge->getProc() != proc) continue;

	predNode->findIncomingNodes(proc, InComingNodeSet, visited);
    }
}



bool
Node::hasEdgesToOtherProc(ProcHandle proc, std::set<IRSymHandle>& visited)
{ 
    IRSymHandle ish = getIRSymHandle();
    if (visited.find(ish) != visited.end()) return false;
    visited.insert(ish);

    // traverse foreward from this nodes
    OA_ptr<EdgesIteratorInterface> succIterPtr = getDUGOutgoingEdgesIterator();
    for (; succIterPtr->isValid(); ++(*succIterPtr)) {
	OA_ptr<EdgeInterface> succEdge = succIterPtr->currentDUGEdge();
	OA_ptr<NodeInterface> succNode = succEdge->getDUGSink();
	
	EdgeType etype = succEdge->getType();
	if (etype != CFLOW_EDGE && etype != CALL_EDGE) continue;
	if (succEdge->getProc() != proc) return true;
	if (etype == CFLOW_EDGE && succNode->hasEdgesToOtherProc(proc, visited)) 
	    return true;
    }
    return false;
}



// true if this has an incoming edge to other procedures
bool
Node::hasEdgesFromOtherProc(ProcHandle proc, std::set<IRSymHandle>& visited)
{ 
    IRSymHandle ish = getIRSymHandle();
    if (visited.find(ish) != visited.end()) return false;
    visited.insert(ish);

    // traverse backward from this nodes
    OA_ptr<EdgesIteratorInterface> predIterPtr = getDUGIncomingEdgesIterator();
    for (; predIterPtr->isValid(); ++(*predIterPtr)) {
	OA_ptr<EdgeInterface> predEdge = predIterPtr->currentDUGEdge();
	OA_ptr<NodeInterface> predNode = predEdge->getDUGSource();
  
	EdgeType etype = predEdge->getType();
	if (etype != CFLOW_EDGE) continue;
	if (predEdge->getProc() != proc) return true;
	if (predNode->hasEdgesFromOtherProc(proc, visited)) 
	    return true;
    }
    return false;
}



    } // end namespace DUGStandard
} // end namespace OA

/*! \file
  
  \brief Declaration for standard DUG (interprocedural control flow graph).

  \authors Michelle Strout
  \version $Id: DUGStandard.hpp,v 1.3 2008/02/06 19:58:47 utke Exp $

  Copyright (c) 2002-2004, Rice University <br>
  Copyright (c) 2004, University of Chicago <br>  
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>

*/

//--------------------------------------------------------------------
//--------------------------------------------------------------------

#ifndef DUGStandard_H
#define DUGStandard_H

//#define DEBUG_DUAA

//--------------------------------------------------------------------
// STL headers
#include <list>
#include <map>

// OpenAnalysis headers
#include <OpenAnalysis/CallGraph/ManagerCallGraph.hpp>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphInterface.hpp>
#include <OpenAnalysis/CFG/CFG.hpp>
#include <OpenAnalysis/IRInterface/ActivityIRInterface.hpp>
#include <OpenAnalysis/IRInterface/DUGIRInterface.hpp>
#include <OpenAnalysis/CSFIActivity/DUGInterface.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphImplement.hpp>
#include <OpenAnalysis/Activity/ActiveStandard.hpp>
#include <OpenAnalysis/DataFlow/ParamBindings.hpp>
//--------------------------------------------------------------------


namespace OA {
  namespace DUG {
      
class Node;
class Edge;
class NodesIterator;
class EdgesIterator;
class DUGStandard;
 
//--------------------------------------------------------
class Node : public virtual NodeInterface {
public:
    Node (OA_ptr<DUGStandard> pDUG, ProcHandle proc, NodeType pType)
        : mDUG(pDUG), mProc(proc), mType(pType) { Ctor(); }
    Node (OA_ptr<DUGStandard> pDUG, ProcHandle proc, NodeType pType,
          OA_ptr<CFG::NodeInterface> cNode)
        : mDUG(pDUG), mProc(proc), mType(pType) { Ctor(); }
    Node (OA_ptr<DUGStandard> pDUG, ProcHandle pProc, SymHandle pSym)
        : mDUG(pDUG), mProc(pProc), mType(NONEFORMAL_NODE), mSym(pSym) {
	Ctor();
    }

    ~Node () { }
 
    //========================================================
    // Info specific to DUG
    //========================================================
    
    NodeType getType() const { return mType; }

    ProcHandle getProc() const { return mProc; }
    OA_ptr<Alias::AliasTag> getTag() const { return aTag; }
    SymHandle getSym() const { return mSym; }
    
    void markVaried(std::list<CallHandle>&, 
                    OA_ptr<Activity::ActivityIRInterface>,
                    std::set<OA_ptr<EdgeInterface> >&,
                    std::set<std::pair<unsigned,unsigned> >&,
                    ProcHandle, unsigned,
                    OA_ptr<EdgeInterface>,
		    bool activeWithVariedOnly);
    void markUseful(std::list<CallHandle>&, 
                    OA_ptr<Activity::ActivityIRInterface>,
                    std::set<OA_ptr<EdgeInterface> >&,
                    std::set<std::pair<unsigned,unsigned> >&,
                    ProcHandle, unsigned,
                    OA_ptr<EdgeInterface>);

    bool isVaried(){ return mVaried;}
    void setVaried()
	{
	    mVaried = true;
	}
    void unsetVaried()
	{ 
	    mVaried = false;
	}

    bool isUseful(){ return mUseful;}
    void setUseful(){ mUseful = true;};
    void unsetUseful(){ mUseful = false;};

    void setActive();
    void setActive(SymHandle);

    bool isSelfDependent(){ return mSelfDependent;}
    void setSelfDependent(){ mSelfDependent = true;};

    bool isPathFrom(OA_ptr<NodeInterface>,
                    std::set<OA_ptr<NodeInterface> >&);
    bool hasEdgesToOtherProc(ProcHandle, std::set<SymHandle>&);
    bool hasEdgesFromOtherProc(ProcHandle, std::set<SymHandle>&);
    void findOutgoingNodes(ProcHandle, std::set<SymHandle>&, std::set<SymHandle>&);
//     void findIncomingNodes(ProcHandle, std::set<SymHandle>&, std::set<SymHandle>&);


    unsigned int size () const {
	assert(0);  //yet to be implemented PLM 09/19/06
	return 0; 
    }


    //========================================================
    // DGraph::Interface::Node interface
    //========================================================
    
    //! getId: An id unique within instances of DUG::Node
    unsigned int getId() const 
	{ 
	    return mDGNode->getId();
	    //return mId; 
	}
    
    //! number of incoming edges
    int num_incoming () const { return mDGNode->num_incoming(); }
    
    //! number of outgoing edges
    int num_outgoing () const { return mDGNode->num_outgoing(); }

    //! returns true if node is an entry node, IOW  has no incoming edges
    bool isAnEntry() const { return mDGNode->isAnEntry(); }

    //! returns true if node is an exit node, IOW  has no outgoing edges
    bool isAnExit() const { return mDGNode->isAnExit(); }

    bool operator==(DGraph::NodeInterface& other);
    bool operator<(DGraph::NodeInterface& other);
   
    //========================================================
    // Output
    //========================================================
    void dump(std::ostream& os) { }  // for full override
    void dumpbase(std::ostream& os) {}
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
    void dumpdot(std::ostream& os, OA_ptr<DUGIRInterface> ir);
    void longdump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
    void output(OA::IRHandlesIRInterface& ir) const { 
        
        mDGNode->output(ir);

    }
    
public:
    //========================================================
    // These methods shadow the same named methods in
    // the DGraph::Interface class and allow us to return
    // the more specific subclass
    //========================================================
    OA_ptr<DGraph::EdgesIteratorInterface> getIncomingEdgesIterator() const;

    OA_ptr<DGraph::EdgesIteratorInterface> getOutgoingEdgesIterator() const;

    OA_ptr<DGraph::NodesIteratorInterface> getSourceNodesIterator() const;

    OA_ptr<DGraph::NodesIteratorInterface> getSinkNodesIterator() const;

    OA_ptr<EdgesIteratorInterface> getDUGIncomingEdgesIterator() const;

    OA_ptr<EdgesIteratorInterface> getDUGOutgoingEdgesIterator() const;

    OA_ptr<NodesIteratorInterface> getDUGSourceNodesIterator() const;

    OA_ptr<NodesIteratorInterface> getDUGSinkNodesIterator() const;

    //========================================================
    // Construction
    //========================================================
    void addIncomingEdge(OA_ptr<DGraph::EdgeInterface> e)
	{
	    return mDGNode->addIncomingEdge(e);  
	}

    void addOutgoingEdge(OA_ptr<DGraph::EdgeInterface> e)
	{
	    return mDGNode->addOutgoingEdge(e); 
	}

    void removeIncomingEdge(OA_ptr<DGraph::EdgeInterface> e)
	{

	    return mDGNode->removeIncomingEdge(e); 
	}

    void removeOutgoingEdge(OA_ptr<DGraph::EdgeInterface> e)
	{
	    return mDGNode->removeOutgoingEdge(e);
	}  

private:
    void Ctor();
   

    unsigned int mId;               // 0 is reserved; first instance is 1
    OA_ptr<DUGStandard> mDUG;       // enclosing DUG
    ProcHandle mProc;               // enclosing procedure
    NodeType mType;                 // node type

    OA_ptr<DGraph::NodeImplement> mDGNode;  // corresponding DGraph node
    // in DGraph that keeps 
    // structure

    OA_ptr<Alias::AliasTag> aTag;
    SymHandle mSym;                  // node key
    bool mVaried;                    // 'true' if varied
    bool mUseful;                    // 'true' if useful
    bool mSelfDependent;             // 'true' if the variable has a self dependence.
    friend class DUGStandard;
    friend class Edge;
};
  
 
//--------------------------------------------------------
class Edge : public virtual EdgeInterface {
public:
    Edge (OA_ptr<DUGStandard> pDUG,
          OA_ptr<Node> pNode1, OA_ptr<Node> pNode2, EdgeType pType,
          CallHandle call, ProcHandle p); 
    ~Edge () {}
    
    //========================================================
    // Info specific to DUG
    //========================================================
    
    EdgeType getType() const { return mType; }

    ProcHandle getSourceProc() const { return mNode1->getProc(); }
    ProcHandle getSinkProc() const { return mNode2->getProc(); }
    CallHandle getCall() const { return mCall; }
    ProcHandle getProc() const { return mProc; }

    //========================================================
    // DGraph::Interface::Edge interface
    //========================================================
    
    //! getId: An id unique within instances of CallGraph::Edge
    unsigned int getId() const 
	{
	    return mDGEdge->getId();
	}

    //! Returns source node for this edge
    OA_ptr<DGraph::NodeInterface> getSource () const { return mNode1; }
    // FIXME: tail and head should be done like source and sink
    // in DGraph::Interface and here
    OA_ptr<DGraph::NodeInterface> tail () const { return mNode1; }
    
    //! Returns sink node for this edge
    OA_ptr<DGraph::NodeInterface> getSink () const { return mNode2; }
    OA_ptr<DGraph::NodeInterface> head () const { return mNode2; }

    OA_ptr<NodeInterface> getDUGSource() const
	{
	    return getSource().convert<Node>();
	}

    OA_ptr<NodeInterface> getDUGSink() const
	{
	    return getSink().convert<Node>();
	}


    bool operator==(DGraph::EdgeInterface& other);
    bool operator<(DGraph::EdgeInterface& other);

    void dump (std::ostream& os);
    void dumpdot (std::ostream& os, OA_ptr<DUGIRInterface> ir);
    void dumpdot_label (std::ostream& os, OA_ptr<DUGIRInterface> ir);
    void dumpbase (std::ostream& os) {}
    void output(OA::IRHandlesIRInterface& ir) const { mDGEdge->output(ir); }

private:

    OA_ptr<DUGStandard> mDUG;
    OA_ptr<Node> mNode1, mNode2;
    OA_ptr<DGraph::EdgeImplement> mDGEdge;

    EdgeType mType;
    CallHandle mCall;
    unsigned int mId; // 0 is reserved; first instance is 1
    ProcHandle mProc; // proc where this edge is used

    friend class DUGStandard;
    friend class Node;

}; 
  
//------------------------------------------------------------------
/*! An iterator over DUG::Nodes that satisfies the interface
  for all different iterator types.
*/
class NodesIterator : public DGraph::NodesIteratorImplement,
		      public virtual NodesIteratorInterface
{
public:
    NodesIterator(OA_ptr<DGraph::NodesIteratorInterface> ni) 
	: DGraph::NodesIteratorImplement(ni) { }

    ~NodesIterator () {}

    OA_ptr<NodeInterface> currentDUGNode() const ;

};
  
//------------------------------------------------------------------
/*! An iterator over DUG::Edges that satisfies the interface
  for all different iterator types.
*/
class EdgesIterator : public DGraph::EdgesIteratorImplement, 
		      public virtual EdgesIteratorInterface
{
public:
    EdgesIterator(OA_ptr<DGraph::EdgesIteratorInterface> ni)
	: DGraph::EdgesIteratorImplement(ni) { }

    ~EdgesIterator () {}

    OA_ptr<EdgeInterface> currentDUGEdge() const;

};
  

//------------------------------------------------------------------
class DUGStandard : public virtual DUGInterface  
{
public:

    friend class Node;  
    
    DUGStandard (
	OA_ptr<DUGIRInterface>,
	OA_ptr<DataFlow::ParamBindings>);
    virtual ~DUGStandard ();
  
    //-------------------------------------
    // DUG information access
    //-------------------------------------

    //! get DUGEdge that is associated with given CFG edge in
    //! one of the CFGs we contain
    OA_ptr<Edge> getDUGEdge(OA_ptr<DGraph::EdgeInterface> dgEdge) const;

    //! get DUGNode that is associated with given CFG node in
    //! one of the CFGs we contain
    OA_ptr<Node> getDUGNode(OA_ptr<DGraph::NodeInterface> dgNode) const;
  
    std::list<std::pair<SymHandle, ProcHandle> >& getIndepSyms(){
	return mIndepSymList;
    }
    std::list<std::pair<SymHandle, ProcHandle> >& getDepSyms(){
	return mDepSymList;
    }

    void insertIndepSymList(SymHandle sym, ProcHandle proc){
	mIndepSymList.push_back(std::pair<SymHandle, ProcHandle>(sym, proc));
	mIndepSymSet.insert(sym);
    }

    void insertDepSymList(SymHandle sym, ProcHandle proc){
	mDepSymList.push_back(std::pair<SymHandle, ProcHandle>(sym, proc));
	mDepSymSet.insert(sym);
    }

    void mapSymToProc(SymHandle sym, ProcHandle proc){
      
	mSymToProc[sym] = proc;
    }

    void insertActiveSymSet(SymHandle sym){
#ifdef DEBUG_DUAA
	std::cout << "DUGStandard::insertActiveSymSet:(sym: "; 
	std::cout << sym << " )" << std::endl;
#endif 
	mActiveSymSet->insert(sym);
    }

    std::map<ProcHandle,OA_ptr<OA::Activity::ActiveStandard> >& getActiveMap(){
	return mActiveMap;
    }

<<<<<<< .working
    OA_ptr<std::set<SymHandle> > getActiveSymSet(){
	return mActiveSymSet;
    }
    void insertActiveSymSet(OA_ptr<Location>);
=======
  OA_ptr<std::set<SymHandle> > getActiveSymSet(){
    return mActiveSymSet;
  }
>>>>>>> .merge-right.r888

<<<<<<< .working
    //! 'true' if the given symbol is independent variable or not
    bool isIndependent(ProcHandle proc, SymHandle sym){
	if (mIndepSymSet.find(sym) != mIndepSymSet.end()) return true;
=======
  // void insertActiveSymSet(OA_ptr<Location>);

 //! 'true' if the given symbol is independent variable or not
  bool isIndependent(SymHandle sym){
    return mIndepSymSet.find(sym) != mIndepSymSet.end();
  }
>>>>>>> .merge-right.r888

	OA_ptr<Location> loc = mIR->getLocation(proc, sym);
	if (loc.ptrEqual(0)) return false;
	OA_ptr<NamedLoc> nloc = loc.convert<NamedLoc>();
	OA_ptr<SymHandleIterator> symIter = nloc->getFullOverlapIter();
	for ( ; symIter->isValid(); (*symIter)++) {
	    SymHandle temp = symIter->current();
	    if (mIndepSymSet.find(temp) != mIndepSymSet.end()) return true;
	}
	return false;
    }

    //! 'true' if the given symbol is dependent variable or not
    bool isDependent(ProcHandle proc, SymHandle sym){
	if (mDepSymSet.find(sym) != mDepSymSet.end()) return true;

	OA_ptr<Location> loc = mIR->getLocation(proc, sym);
	if (loc.ptrEqual(0)) return false;
	OA_ptr<NamedLoc> nloc = loc.convert<NamedLoc>();
	OA_ptr<SymHandleIterator> symIter = nloc->getFullOverlapIter();
	for ( ; symIter->isValid(); (*symIter)++) {
	    SymHandle temp = symIter->current();
	    if (mDepSymSet.find(temp) != mDepSymSet.end()) return true;
	}
	return false;
    }

    //! Indicate whether the given symbol is active or not
    bool isActive(SymHandle sym);

    //! Indicate whether the given stmt is active or not
    bool isActive(StmtHandle stmt);

    //! Indicate whether the given memref is active or not
    bool isActive(MemRefHandle memref);

    void insertActiveStmtSet(StmtHandle stmt, ProcHandle proc){
	mActiveStmtSet->insert(stmt);
	mActiveMap[proc]->insertStmt(stmt);
    }


    void insertActiveMemRefSet(MemRefHandle mref, ProcHandle proc){
	mActiveMemRefSet->insert(mref);
	mActiveMap[proc]->insertMemRef(mref);
    }

    void assignActiveStandard(ProcHandle proc) {
    
	if (mActiveMap[proc].ptrEqual(0)) {
	    mActiveMap[proc] = new OA::Activity::ActiveStandard(proc);
	}
    }
    
    //========================================================
    // Construction
    //========================================================
  
  
    void addNode(OA_ptr<DGraph::NodeInterface> pNode);
    void addEdge(OA_ptr<DGraph::EdgeInterface> pEdge);
  

    //========================================================
    // DGraph::Interface::Edge interface
    //========================================================
 
    int getNumNodes () { return mDGraph->getNumNodes(); }
    int getNumEdges () { return mDGraph->getNumEdges(); }

  
<<<<<<< .working
    // Output Functionalities 
    void dump (std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
    void dumpdot (std::ostream& os, OA_ptr<DUGIRInterface> ir);
    void dumpbase(std::ostream& os) {}
    void output(OA::IRHandlesIRInterface& ir) { mDGraph->output(ir); }
=======
  // Output Functionalities 
  void dump (std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
  void dumpdot (std::ostream& os, OA_ptr<DUGIRInterface> ir);
  void dumpbase(std::ostream& os) {}
  void output(OA::IRHandlesIRInterface& ir) const { mDGraph->output(ir); }
>>>>>>> .merge-right.r888

  
    //------------------------------------------------------------------
    // Using trick of imitating covariance when getting all iterators
    // so that get an iterator specific to actual subclass
    // This is why have these getBlahIterator methods that shadow those
    // in DGraph::Interface and also have the protected ones
    // that must be defined here which override implementation in 
    // DGraph::Interface
    //------------------------------------------------------------------
  
    OA_ptr<DGraph::NodesIteratorInterface> getNodesIterator() const;

    OA_ptr<DGraph::NodesIteratorInterface> getEntryNodesIterator() const;

    OA_ptr<DGraph::NodesIteratorInterface> getExitNodesIterator() const;

    OA_ptr<DGraph::EdgesIteratorInterface> getEdgesIterator() const;

    OA_ptr<DGraph::NodesIteratorInterface> getDFSIterator(OA_ptr<DGraph::NodeInterface> n);

    OA_ptr<DGraph::NodesIteratorInterface> getBFSIterator();

    OA_ptr<DGraph::NodesIteratorInterface> 
    getReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient);

    OA_ptr<DGraph::NodesIteratorInterface>
    getReversePostDFSIterator(OA_ptr<DGraph::NodeInterface> root, 
                              DGraph::DGraphEdgeDirection pOrient);

    OA_ptr<DGraph::NodesIteratorInterface> getPostDFSIterator(
	DGraph::DGraphEdgeDirection pOrient);

    OA_ptr<DGraph::NodesIteratorInterface>
    getPostDFSIterator(OA_ptr<DGraph::NodeInterface> root, 
                       DGraph::DGraphEdgeDirection pOrient);


    //==================================================================
  
    OA_ptr<NodesIteratorInterface>
    getDUGNodesIterator() const;

    OA_ptr<EdgesIteratorInterface>
    getDUGEdgesIterator() const;

    OA_ptr<NodesIteratorInterface>
    getDUGEntryNodesIterator() const;

    OA_ptr<NodesIteratorInterface>
    getDUGExitNodesIterator() const;

    OA_ptr<NodesIteratorInterface>
    getDUGReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient);

    OA_ptr<NodesIteratorInterface>
    getDUGDFSIterator(OA_ptr<NodeInterface> n);

<<<<<<< .working
    //! map Location to MemRefHandle
    void mapSymToMemRefSet(SymHandle sym, MemRefHandle mref) {
	mSymToMemRefSet[sym].insert(mref);
    }
=======
/*
  //! map Location to MemRefHandle
  void mapSymToMemRefSet(SymHandle sym, MemRefHandle mref) {
    mSymToMemRefSet[sym].insert(mref);
  }
>>>>>>> .merge-right.r888
*/


    //! map Location to StmtHandle
    void mapSymToStmtSet(SymHandle sym, StmtHandle stmt) {
	mSymToStmtSet[sym].insert(stmt);
    }

    //! Return an iterator over all MemRefs that are mapped from
    //! the given Location
    std::set<MemRefHandle> getMemRefSet(SymHandle sym) {
	return mSymToMemRefSet[sym];
    }

    //! Return an iterator over all StmtHandles that are mapped from
    //! the given Location
    std::set<StmtHandle> getStmtSet(SymHandle sym) {
	return mSymToStmtSet[sym];
    }
 
    //==================================================================

    OA_ptr<Node> getNode(SymHandle, ProcHandle);
    bool isNode(SymHandle, ProcHandle);

private:
    OA_ptr<Node> mEntry; 
    OA_ptr<Node> mExit;

    // dgraph that will store underlying graph structure
    //  OA_ptr<DGraph::DGraphImplement> mDGraph;
  
    OA_ptr<DGraph::DGraphImplement> mDGraph;

    // using lists instead of sets because some of the iterators
    // need an ordered list of things and only want to have one
    // NodeIterator and EdgesIterator implementation
    OA_ptr<std::list<OA_ptr<Node> > > mCallNodes;
    OA_ptr<std::list<OA_ptr<Node> > > mReturnNodes;

    // map from SymHandle to Node
    std::map<SymHandle, OA_ptr<Node> > mSymToNode;

<<<<<<< .working
    // map from SymHandle to Location
    std::map<SymHandle, OA_ptr<Location> > mSymToLoc;
    std::map<SymHandle, ProcHandle> mSymToProc;
=======
  // map from SymHandle to Location
  std::map<SymHandle, OA_ptr<Alias::AliasTag> > mSymToTag;
  std::map<SymHandle, ProcHandle> mSymToProc;
>>>>>>> .merge-right.r888

    // DUAA specific
    OA_ptr<DUGIRInterface>            mIR;
    //OA_ptr<IRHandlesIRInterface>      mIR;
    OA_ptr<DataFlow::ParamBindings>   mParamBind;


    std::list<std::pair<SymHandle, ProcHandle> > mIndepSymList;
    std::list<std::pair<SymHandle, ProcHandle> > mDepSymList;
    std::set<SymHandle>             mIndepSymSet;
    std::set<SymHandle>             mDepSymSet;

    OA_ptr<std::set<StmtHandle> > mActiveStmtSet;
    OA_ptr<std::set<MemRefHandle> > mActiveMemRefSet;
    OA_ptr<std::set<SymHandle> >    mActiveSymSet;
    std::map<ProcHandle,OA_ptr<OA::Activity::ActiveStandard> > mActiveMap;

    bool mUnknownLocActive;



    std::map<SymHandle, std::set<MemRefHandle> > mSymToMemRefSet;

    std::map<SymHandle, std::set<StmtHandle> > mSymToStmtSet;

<<<<<<< .working
    std::map<OA_ptr<Location>, OA_ptr<std::set<MemRefHandle> > > mLocToMemRefSet;
=======
  std::map<OA_ptr<Alias::AliasTag>, OA_ptr<std::set<MemRefHandle> > > mTagToMemRefSet;
>>>>>>> .merge-right.r888

<<<<<<< .working
    std::map<OA_ptr<Location>, OA_ptr<std::set<StmtHandle> > > mLocToStmtSet;
=======
  std::map<OA_ptr<Alias::AliasTag>, OA_ptr<std::set<StmtHandle> > > mTagToStmtSet;
>>>>>>> .merge-right.r888

};
//--------------------------------------------------------------------

  } // end of DUG namespace
} // end of OA namespace

#endif

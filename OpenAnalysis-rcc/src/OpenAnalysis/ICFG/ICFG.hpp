/*! \file
  
  \brief Declaration for standard ICFG (interprocedural control flow graph).

  \authors Michelle Strout, Barbara Kreaseck, 
    Priyadarshini Malusare

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

//--------------------------------------------------------------------
//--------------------------------------------------------------------

#ifndef ICFG_H
#define ICFG_H

//--------------------------------------------------------------------
// STL headers
#include <list>
#include <map>

// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphImplement.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphInterface.hpp>
#include <OpenAnalysis/CFG/CFG.hpp>
#include "ICFGInterface.hpp"

//=================================================================
// ICFG is the special case of CFG and it can access CFG directly
// without using CFGInterface.
//=================================================================

//--------------------------------------------------------------------

namespace OA {
  namespace ICFG {
      
// Changes here must be also reflected in edgeTypeToString 
// and nodeTypeToString.
 
//--------------------------------------------------------------------
/*! ICFG implements the DGraph::Interface (directed graph) but has
    enhanced nodes and edges.  Each node corresponds to a basic block in one of
    the procedures and contains a list of statments and each edge is either a
    control flow edge, call edge, or return edge.
*/
  class Node;
  class Edge;

  class NodesIterator;

  class EdgesIterator;
  class ICFG;

 
  //--------------------------------------------------------
  class Node : public virtual NodeInterface,
               public virtual DGraph::NodeImplement
  {
  public:
    Node (OA_ptr<ICFG> pICFG, ProcHandle proc, NodeType pType);
    Node (OA_ptr<ICFG> pICFG, ProcHandle proc, NodeType pType, CallHandle call);
    Node (OA_ptr<ICFG> pICFG, ProcHandle proc, NodeType pType, 
          OA_ptr<CFG::NodeInterface> cNode); 
    // from MustMayActive branch:
    Node (Node& other) ;

    ~Node ();
    
    //========================================================
    // Info specific to ICFG
    //========================================================
    
    NodeType getType() const;

    ProcHandle getProc() const;

    CallHandle getCall() const;
    
    //! number of statements in node
    unsigned int size () const;

    //! create a forward order iterator for the statements in the node
    OA_ptr<CFG::NodeStatementsIteratorInterface> 
        getNodeStatementsIterator() const;

    //! create a reverse order iterator for the statements in the node
    OA_ptr<CFG::NodeStatementsRevIteratorInterface> 
        getNodeStatementsRevIterator() const;

    //========================================================
    // Construction Methods
    //========================================================

    void addCallEdge(OA_ptr<Edge> e);
    void addReturnEdge(OA_ptr<Edge> e);

    // from MustMayActive branch:
    void removeCallEdge( OA_ptr<Edge> e);
    void removeReturnEdge( OA_ptr<Edge> e);

    //=====================================================================

    OA_ptr<EdgesIteratorInterface> getICFGIncomingEdgesIterator() const;

    OA_ptr<EdgesIteratorInterface> getICFGOutgoingEdgesIterator() const;

    OA_ptr<NodesIteratorInterface> getICFGSourceNodesIterator() const;

    OA_ptr<NodesIteratorInterface> getICFGSinkNodesIterator() const;

    //========================================================
    // Output
    //========================================================
    void dump(std::ostream& os) {} // for full override
    void dumpbase(std::ostream& os) {}
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
    void dumpdot(ICFG&,
                 std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

    void longdump(ICFG& icfg, std::ostream& os,
                  OA_ptr<IRHandlesIRInterface> ir);

    virtual void output(OA::IRHandlesIRInterface& ir);


    //------------------------------------------------------------------
  

  private:
     void Ctor();

    // ========================================================== 
    // if we are a ENTRY_Node and have CallEdges then we
    // are the entry to a CFG and the CallEdges are incoming
    // if we are a EXIT_Node and have ReturnEdges then 
    // we are the exit to a CFG and the return edges are outgoing
    // if we are a Call node then the call edges are outgoing
    // if we are a return node then the call edges are incoming
    // ==========================================================
    // 
    OA_ptr<std::list<OA_ptr<Edge> > > mCallEdges;
    OA_ptr<std::list<OA_ptr<Edge> > > mReturnEdges;


    OA_ptr<ICFG> mICFG;             // enclosing ICFG
    ProcHandle mProc;               // enclosing procedure
    CallHandle mCall;
    NodeType mType;                 // node type
    OA_ptr<CFG::NodeInterface> mCFGNode;        // corresponding CFG node
    //DGraph::NodeImplement mDGNode;  // corresponding DGraph node
                                                   // in DGraph that keeps 
                                                   // structure

    friend class ICFG;
    friend class Edge;
  };
  
  // lt_Node: function object that compares by id.  Useful for sorting.
  class lt_Node : public DGraph::lt_NodeInterface {
  public:
    // return true if n1 < n2; false otherwise
    bool operator()(const OA_ptr<DGraph::NodeInterface> n1, 
                    const OA_ptr<DGraph::NodeInterface> n2) const ;
  };
 
  //--------------------------------------------------------
  class Edge : public virtual EdgeInterface,
               public virtual DGraph::EdgeImplement
  {
  public:


    Edge (OA_ptr<ICFG> pICFG, OA_ptr<DGraph::NodeInterface> pNode1, 
          OA_ptr<DGraph::NodeInterface> pNode2, EdgeType pType,
          CallHandle call, CFG::EdgeType cfgEdgeType, ExprHandle cfgExpr);


    Edge (OA_ptr<ICFG> pICFG, OA_ptr<DGraph::NodeInterface> pNode1, 
          OA_ptr<DGraph::NodeInterface> pNode2, EdgeType pType,
          CFG::EdgeType cfgEdgeType, ExprHandle cfgExpr);

    
    ~Edge ();
    
    //========================================================
    // Info specific to ICFG
    //========================================================
    
    EdgeType getType() const;

    ProcHandle getSourceProc() const;
    ProcHandle getSinkProc() const;
    
    CallHandle getCall() const;

    //===========
    // Info passed along from CFG edges
    //===========

    CFG::EdgeType getCFGType() const;
    ExprHandle    getCFGExpr() const;

    //========================================================
    // DGraph::Interface::Edge interface
    //========================================================
    
    OA_ptr<NodeInterface> getICFGSource() const;

    OA_ptr<NodeInterface> getICFGSink() const;
    
    void dump (std::ostream& os);
    void dumpdot (std::ostream& os);
    void dumpbase (std::ostream& os) {}
    virtual void output(OA::IRHandlesIRInterface& ir);  


  private:

    OA_ptr<ICFG> mICFG;
    OA_ptr<Node> mNode1, mNode2;

    EdgeType mType;
    CallHandle mCall;

    CFG::EdgeType mCFGType;
    ExprHandle mCFGExpr;

    friend class ICFG;
    friend class Node;

  }; 
  
  // lt_Edge: function object that compares by source and sink node
  // ids.  Useful for sorting.  Not used to put edges in sets or other
  // STL containers.
  
  class lt_Edge : public DGraph::lt_EdgeInterface {
  public:
      bool operator()(const OA_ptr<DGraph::EdgeInterface> e1, 
                      const OA_ptr<DGraph::EdgeInterface> e2) const;
  };

  //------------------------------------------------------------------
  /*! An iterator over ICFG::Nodes that satisfies the interface
      for all different iterator types.
  */
  class NodesIterator : public DGraph::NodesIteratorImplement, 
                        public virtual NodesIteratorInterface
  {
  public:
    NodesIterator(OA_ptr<DGraph::NodesIteratorInterface> ni);

    OA_ptr<NodeInterface> currentICFGNode() const ;
    
  };
  
  //------------------------------------------------------------------
  /*! An iterator over ICFG::Edges that satisfies the interface
      for all different iterator types.
  */
  class EdgesIterator : public DGraph::EdgesIteratorImplement,
                        public virtual EdgesIteratorInterface
  {
  public:
      
     EdgesIterator(OA_ptr<DGraph::EdgesIteratorInterface> ni);

    OA_ptr<EdgeInterface> currentICFGEdge() const;

  };
  

//------------------------------------------------------------------
class ICFG : public virtual ICFGInterface,
             public virtual DGraph::DGraphImplement
{
  
public:
  ICFG (); 
  virtual ~ICFG ();
 
  //========================================================
  // Construction
  //========================================================
  //void addNode(OA_ptr<Node> pNode);
  void addEdge(OA_ptr<Edge> pEdge);

  //=======================================================
  // From MustMayActive
  // =====================================================
  // 
  void removeEdge(OA_ptr<Edge> e);

  //void removeNode(OA_ptr<DGraph::NodeInterface> n);

  // can't do connect by creating a new edge in a member function
  // because edges have to have an OA_ptr to containing ICFG
  //void connect(OA_ptr<Node> pNode1, OA_ptr<Node> pNode2,
  //             EdgeType pType);

  //========================================================
  // Output 
  //========================================================
  void dump (std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
  void dumpdot (std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
  void dumpbase(std::ostream& os) {}
  virtual void output(OA::IRHandlesIRInterface& ir);  


  // =================================================================
  // Using trick of imitating covariance when getting all iterators
  // so that get an iterator specific to actual subclass
  // This is why have these getBlahIterator methods that shadow those
  // in DGraph::Interface and also have the protected ones
  // that must be defined here which override implementation in 
  // DGraph::Interface
  // FIXME: still true?
  //==================================================================
  
  OA_ptr<NodesIteratorInterface> 
      getICFGNodesIterator() const;

  OA_ptr<EdgesIteratorInterface> 
      getICFGEdgesIterator() const;

  OA_ptr<NodesIteratorInterface> 
      getICFGEntryNodesIterator() const;

  OA_ptr<NodesIteratorInterface> 
      getICFGExitNodesIterator() const;

  OA_ptr<NodesIteratorInterface>
    getICFGReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient);

  OA_ptr<NodesIteratorInterface> 
      getICFGDFSIterator(OA_ptr<NodeInterface> n);

 private:
  OA_ptr<Node> mEntry; 
  OA_ptr<Node> mExit;




  // using lists instead of sets because some of the iterators
  // need an ordered list of things and only want to have one
  // NodeIterator and EdgesIterator implementation
  OA_ptr<std::list<OA_ptr<Node> > > mCallNodes;
  OA_ptr<std::list<OA_ptr<Edge> > > mCallEdges;
  OA_ptr<std::list<OA_ptr<Node> > > mReturnNodes;
  OA_ptr<std::list<OA_ptr<Edge> > > mReturnEdges;


};
//--------------------------------------------------------------------

  } // end of ICFG namespace
} // end of OA namespace

#endif

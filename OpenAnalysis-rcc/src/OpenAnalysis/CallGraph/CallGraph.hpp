/*! \file
  
  \brief Declaration for standard CallGraph.

  \authors Nathan Tallent?, Michelle Strout, Andy Stone, Barbara Kreaseck, Priyadarshini Malusare
  \version $Id: CallGraph.hpp,v 1.21 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

//--------------------------------------------------------------------
//--------------------------------------------------------------------

#ifndef CallGraph_H
#define CallGraph_H

//--------------------------------------------------------------------
// standard headers
#ifdef NO_STD_CHEADERS
# include <string.h>
#else
# include <cstring>
#endif

// STL headers
#include <list>
#include <map>

// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphImplement.hpp>
#include <OpenAnalysis/IRInterface/CallGraphIRInterface.hpp>

#include <OpenAnalysis/Utils/Iterator.hpp>
#include "CallGraphInterface.hpp"
#include <OpenAnalysis/Utils/DGraph/DGraphInterface.hpp>
//--------------------------------------------------------------------

namespace OA {
  namespace CallGraph {

      class Node;
      class Edge;
      class NodesIterator;
      class EdgesIterator;
      class CallGraph;
      class CallGraphCalleeProcIter;
      class NodeCallsIterator;

      //=======================================================================
      //! CallGraph is a DGraph (directed graph) with enhanced nodes and
      //! edges.  Each node corresponds to a function definition or
      //! reference (or both).  Each node contains a list of places where it
      //! is defined or referenced.
      //=========================================================================


      //=======================================================================
      //! general iterator for Procs that are called from a common CallHandle 
      class CallGraphCalleeProcIter : public virtual ProcHandleIterator,
                           public IRHandleSetIterator<ProcHandle>
      {
        public:
            
            CallGraphCalleeProcIter(OA_ptr<std::set<ProcHandle> > pSet);
    
           ~CallGraphCalleeProcIter();

            void operator++();
    
            bool isValid() const;
    
            ProcHandle current() const;
    
            void reset();
    
       };

  // ============================================================================= 
  //! The NodesIterator is just an extension of DGraphStandard::NodesIterator 
  //!    and Interface::NodesIterator
  //!    to provide access to CallGraph::CallGraph nodes. 
  // =============================================================================

  class NodesIterator : public DGraph::NodesIteratorImplement,
                        public virtual NodesIteratorInterface
  {
  public:

    NodesIterator(OA_ptr<DGraph::NodesIteratorInterface> ni);

    OA_ptr<NodeInterface> currentCallGraphNode() const;

  };

  //------------------------------------------------------------------
  class EdgesIterator : public DGraph::EdgesIteratorImplement, 
                        public virtual  EdgesIteratorInterface
  {
  public:

    EdgesIterator(OA_ptr<DGraph::EdgesIteratorInterface> ni);

    OA_ptr<EdgeInterface> currentCallGraphEdge() const;

  };

 
  //===========================================================  

  class Node : public virtual NodeInterface, 
               public virtual DGraph::NodeImplement
  {
  public:
    Node();
    Node(SymHandle s);
    ~Node ();
    
    //! getId: An id unique within instances of CallGraph::Node
    //virtual unsigned int getId() const;

    bool isDefined() const;
    bool isCalled() const;
    
    ProcHandle getProc() const;
    SymHandle getProcSym() const;

    /*
    int num_incoming () const ;
    int num_outgoing () const ;
    */

    /*
    //! returns true if node is an entry node, IOW  has no incoming edges
    bool isAnEntry() const;
    bool isAnExit() const;
    */

    /*
    //========================================================
    // Comparison operators
    //========================================================
    bool operator== (DGraph::NodeInterface& other);

    bool operator< (DGraph::NodeInterface& other);
    */

    /*
    //========================================================
    // Construction
    //========================================================
    void addIncomingEdge(OA_ptr<DGraph::EdgeInterface> e);

    void addOutgoingEdge(OA_ptr<DGraph::EdgeInterface> e);

    void removeIncomingEdge(OA_ptr<DGraph::EdgeInterface> e);
    
    void removeOutgoingEdge(OA_ptr<DGraph::EdgeInterface> e);
    */

    virtual void output(OA::IRHandlesIRInterface& ir) const;

    void dump(std::ostream& os) {} // for full override
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
    void longdump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
    
    friend class NodeCallsIterator;

    //==================================================== 
    // Be Careful
    //===================================================

    void add_def(ProcHandle h);

    void add_call(CallHandle h);

  public:
    /*
    // Other Iterators
    OA_ptr<DGraph::EdgesIteratorInterface> 
           getIncomingEdgesIterator() const;

    OA_ptr<DGraph::EdgesIteratorInterface> 
           getOutgoingEdgesIterator() const;

    OA_ptr<DGraph::NodesIteratorInterface> 
           getSourceNodesIterator() const;

    OA_ptr<DGraph::NodesIteratorInterface> 
           getSinkNodesIterator() const;
           */

    OA_ptr<EdgesIteratorInterface> 
           getCallGraphIncomingEdgesIterator() const;

    OA_ptr<EdgesIteratorInterface> 
           getCallGraphOutgoingEdgesIterator() const;

    OA_ptr<NodesIteratorInterface> 
           getCallGraphSourceNodesIterator() const;

    OA_ptr<NodesIteratorInterface> 
           getCallGraphSinkNodesIterator() const;

  private:
    

    SymHandle mSym;      // should be identical across def and uses
    ProcHandle mProc;             // where this function is defined (if any)
    std::list<CallHandle> mCalls; // places this function is called (if any)
    //DGraph::NodeImplement mDGNode;
  };
  
  //--------------------------------------------------------
  class Edge : public virtual EdgeInterface,
               public virtual DGraph::EdgeImplement
{
  public:

   Edge(OA_ptr<DGraph::NodeInterface> source,
             OA_ptr<DGraph::NodeInterface> sink, EdgeType _type,
             CallHandle call);

    ~Edge ();
    
    //! getId: An id unique within instances of CallGraph::Edge
    //virtual unsigned int getId() const;
    
    EdgeType getType() const;

    //! Returns the handle for the callsite that induces this edge
    CallHandle getCallHandle() const;
   // void setCallHandle(CallHandle call) { mCallsiteExpr = call; }


    /* 
    // =====================================================================
    //! DGraph Nodes
    // ===================================================================
    
    //! Returns source node for this edge
    OA_ptr<DGraph::NodeInterface> getSource () const;
    
    //! Returns sink node for this edge
    OA_ptr<DGraph::NodeInterface> getSink () const;
    */

    // ============================================================= 
    //! CallGraph Specific Iterators
    // =============================================================

    OA_ptr<NodeInterface> getCallGraphSource() const;

    OA_ptr<NodeInterface> getCallGraphSink() const;


    /*
    //=========================================================
    //! Comparision Operators
    // =======================================================

    bool operator== (DGraph::EdgeInterface& other);

    bool Edge::operator< (DGraph::EdgeInterface& other);
    */

    virtual void output(OA::IRHandlesIRInterface& ir) const;
    
    void dump (std::ostream& os);

  private:

    EdgeType mType;
    CallHandle mCallsiteExpr;
    //DGraph::EdgeImplement mDGEdge;
  };  

  // =============================================================

  class NodeCallsIterator : public virtual NodeCallsIteratorInterface {
  public:
    NodeCallsIterator (OA_ptr<NodeInterface> node_) 
        { mNode = node_.convert<Node>(); 
          mIter = mNode->mCalls.begin(); }
    ~NodeCallsIterator () {}
    void operator ++ () { if (mIter != mNode->mCalls.end()) { ++mIter; } }
    bool isValid() const { return (mIter != mNode->mCalls.end()); }
    CallHandle current() const { return *mIter; }
  private:
    OA_ptr<Node> mNode;
    std::list<CallHandle>::iterator mIter;
  };

  // ===========================================================
  
class CallGraph: public virtual CallGraphInterface,
                 public virtual DGraph::DGraphImplement
{
  
public:  // CallGraph class

  CallGraph (const SymHandle name);
  
  CallGraph();
  
  virtual ~CallGraph ();

  class NodeLabelListIterator;
  
  friend class NodeLabelListIterator;

  void connect (OA_ptr<NodeInterface> src, OA_ptr<NodeInterface> dst, 
                EdgeType type, CallHandle call);
  
  void disconnect (OA_ptr<EdgeInterface> e);

  /*
  int getNumNodes();
   
  int getNumEdges();
  */
        
  OA_ptr<Node> findOrAddNode(SymHandle sym);
  
  void addToCallProcSetMap(CallHandle call, ProcHandle proc);

  typedef std::map<SymHandle, OA_ptr<Node> > SymToNodeMapType;

  void dump (std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
  
  virtual void output(OA::IRHandlesIRInterface& ir) const;

 const OA::SymHandle subprog_name () { return mName; }

  //-------------------------------------
  // Iterators
  // Have same code here as in DGraphStandard.hpp because
  // these methods are meant to shadow those in DGraphStandard.hpp
  // so that we get subclass specific return values
  //-------------------------------------

 /*
  OA_ptr<DGraph::NodesIteratorInterface> getNodesIterator() const;

  OA_ptr<DGraph::EdgesIteratorInterface> getEdgesIterator() const;

  OA_ptr<DGraph::NodesIteratorInterface> 
      getReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient);

  OA_ptr<DGraph::NodesIteratorInterface>
      getDFSIterator(OA_ptr<DGraph::NodeInterface> n);

  OA_ptr<DGraph::NodesIteratorInterface> 
      getEntryNodesIterator() const;

  OA_ptr<DGraph::NodesIteratorInterface> 
      getExitNodesIterator() const;
  */    

  OA_ptr<NodesIteratorInterface> 
      getCallGraphNodesIterator() const;

  OA_ptr<EdgesIteratorInterface> 
      getCallGraphEdgesIterator() const;

  OA_ptr<NodesIteratorInterface>
      getCallGraphReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient);

  OA_ptr<NodesIteratorInterface>
      getCallGraphDFSIterator(OA_ptr<NodeInterface> n);


  OA_ptr<NodesIteratorInterface> 
      getCallGraphEntryNodesIterator() const;

  OA_ptr<NodesIteratorInterface> 
      getCallGraphExitNodesIterator() const;

   /*
   //========================================================
   // Construction
   //========================================================
   void addNode(OA_ptr<DGraph::NodeInterface> n);

   void addEdge(OA_ptr<DGraph::EdgeInterface> e);

   void removeEdge(OA_ptr<DGraph::EdgeInterface> e);

   void removeNode(OA_ptr<DGraph::NodeInterface> n);
   */
   
  /*! Returns an iterator over ProcHandles, of Procs that are may called from
      the given CallHandle
  */
  OA_ptr<ProcHandleIterator> getCalleeProcIter(CallHandle call);

private:
  const SymHandle mName;
  
  SymToNodeMapType mSymToNodeMap;

  std::map<CallHandle, OA_ptr<std::set<ProcHandle> > > mCallToCalleeProcSetMap;

  //DGraph::DGraphImplement mDGraph;
};
//--------------------------------------------------------------------

  } // end of CallGraph namespace
} // end of OA namespace

#endif

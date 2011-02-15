/*! \file
  
  \brief Abstract interface that all ICFG analysis results must satisfy.

  \authors Michelle Strout, Priyadarshini Malusare
  \version $Id: ICFGInterface.hpp,v 1.1 2005/06/21 15:20:55 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


FIXME: Not actually being used yet and needs fixed
*/

//--------------------------------------------------------------------
//--------------------------------------------------------------------

#ifndef ICFGInterface_H
#define ICFGInterface_H

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphInterface.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>

//--------------------------------------------------------------------

namespace OA {
  namespace ICFG {
      
// Changes here must be also reflected in edgeTypeToString 
// and nodeTypeToString.
enum EdgeType { CFLOW_EDGE, CALL_EDGE, RETURN_EDGE, CALL_RETURN_EDGE };
enum NodeType { CFLOW_NODE, CALL_NODE, RETURN_NODE, ENTRY_NODE, EXIT_NODE };
 
//--------------------------------------------------------------------
/*! ICFG implements the DGraph::Interface (directed graph) but has
    enhanced nodes and edges.  Each node corresponds to a basic block in one of
    the procedures and contains a list of statments and each edge is either a
    control flow edge, call edge, or return edge.
*/
  class NodeInterface;
  class EdgeInterface;
  class ICFGInterface;
  class NodesIteratorInterface;
  class EdgesIteratorInterface;
  
 
  //--------------------------------------------------------
  class NodeInterface : public virtual DGraph::NodeInterface {
  public:
    NodeInterface () {}
    ~NodeInterface () { }
    
    //========================================================
    // Info specific to ICFG
    //========================================================
    
    virtual NodeType getType() const = 0;

    virtual ProcHandle getProc() const = 0;
   
    virtual CallHandle getCall() const = 0;
 
    //! number of statements in node
    virtual unsigned int size () const = 0;

    //! create a forward order iterator for the statements in the node
    virtual OA_ptr<CFG::NodeStatementsIteratorInterface> 
        getNodeStatementsIterator() const = 0;

    //! create a reverse order iterator for the statements in the node
    virtual OA_ptr<CFG::NodeStatementsRevIteratorInterface> 
        getNodeStatementsRevIterator() const = 0;

    //========================================================
    // Output
    //========================================================
//    virtual void dump(std::ostream& os) =  0;
//    virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir) = 0;
//    virtual void dumpdot(ICFGStandard&, 
//                 std::ostream& os, OA_ptr<IRHandlesIRInterface> ir) = 0;
//    void longdump(ICFGStandard& icfg, std::ostream& os, 
//                  OA_ptr<IRHandlesIRInterface> ir) = 0;
    
  public:
    //========================================================
    // These methods shadow the same named methods in
    // the DGraph::Interface class and allow us to return
    // the more specific subclass
    //========================================================
    // Other Iterators
    /*
    OA_ptr<DGraph::EdgesIteratorInterface> getIncomingEdgesIterator() const { 
    }

    OA_ptr<DGraph::EdgesIteratorInterface> getOutgoingEdgesIterator() const { 
    }

    OA_ptr<DGraph::NodesIteratorInterface> getSourceNodesIterator() const {
    }


    OA_ptr<DGraph::NodesIteratorInterface> getSinkNodesIterator() const { 
    }
    */


    virtual OA_ptr<EdgesIteratorInterface> getICFGIncomingEdgesIterator() const = 0;

    virtual OA_ptr<EdgesIteratorInterface> getICFGOutgoingEdgesIterator() const = 0;

    virtual OA_ptr<NodesIteratorInterface> getICFGSourceNodesIterator() const = 0;

    virtual OA_ptr<NodesIteratorInterface> getICFGSinkNodesIterator() const = 0;

    
    //------------------------------------------------------------------
  };
  
  //--------------------------------------------------------
  class EdgeInterface : public virtual DGraph::EdgeInterface {
  public:
    EdgeInterface () {}
    ~EdgeInterface () {}
    
    //========================================================
    // Info specific to ICFG
    //========================================================
    
    virtual EdgeType getType() const = 0;

    virtual ProcHandle getSourceProc() const = 0;
    virtual ProcHandle getSinkProc() const = 0;
    virtual CallHandle getCall() const = 0;

    virtual OA_ptr<NodeInterface> getICFGSource() const = 0;

    virtual OA_ptr<NodeInterface> getICFGSink() const = 0;


  }; 
  
 
  //------------------------------------------------------------------
  /*! An iterator over ICFG::Nodes that satisfies the interface
      for all different iterator types.
  */
  class NodesIteratorInterface : public virtual DGraph::NodesIteratorInterface 
  {
  public:
    virtual OA_ptr<NodeInterface> currentICFGNode() const = 0;
  };
  
  //------------------------------------------------------------------
  /*! An iterator over ICFG::Edges that satisfies the interface
      for all different iterator types.
  */
  class EdgesIteratorInterface : public virtual DGraph::EdgesIteratorInterface
  {
  public:

    virtual OA_ptr<EdgeInterface> currentICFGEdge() const = 0;
  };
  

//------------------------------------------------------------------

class ICFGInterface : public virtual DGraph::DGraphInterface {

public:
  
  //void dump (std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
  //void dumpdot (std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
  //void dumpbase(std::ostream& os) {}
  
  //------------------------------------------------------------------
  // Using trick of imitating covariance when getting all iterators
  // so that get an iterator specific to actual subclass
  // This is why have these getBlahIterator methods that shadow those
  // in DGraph::Interface and also have the protected ones
  // that must be defined here which override implementation in 
  // DGraph::Interface
  //------------------------------------------------------------------
  /*
  OA_ptr<DGraph::NodesIteratorInterface> getNodesIterator() const { 
  }
 

  virtual OA_ptr<DGraph::NodesIteratorInterface> getExitNodesIterator() const = 0;

  OA_ptr<DGraph::EdgesIteratorInterface> getEdgesIterator() const { 
  }
  
  OA_ptr<DGraph::NodesIteratorInterface> 
    getReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient) { 
  }

  
  OA_ptr<DGraph::NodesIteratorInterface> getDFSIterator(OA_ptr<NodeInterface> n) { }
  */
  
  virtual OA_ptr<NodesIteratorInterface> getICFGNodesIterator() const = 0;

  virtual OA_ptr<EdgesIteratorInterface> getICFGEdgesIterator() const = 0;

  virtual OA_ptr<NodesIteratorInterface> getICFGEntryNodesIterator() const = 0;

  virtual OA_ptr<NodesIteratorInterface> getICFGExitNodesIterator() const = 0;

  virtual OA_ptr<NodesIteratorInterface>
    getICFGReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient) = 0;

 
  virtual OA_ptr<NodesIteratorInterface> getICFGDFSIterator(OA_ptr<NodeInterface> n) = 0;


  
};
//--------------------------------------------------------------------

  } // end of ICFG namespace
} // end of OA namespace

#endif

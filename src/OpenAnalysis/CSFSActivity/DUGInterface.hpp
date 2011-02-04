/*! \file
  
  \brief Abstract interface that all DUG analysis results must satisfy.

  \authors Michelle Strout
  \version $Id: DUGInterface.hpp,v 1.1 2005/06/21 15:20:55 mstrout Exp $

  Copyright (c) 2002-2004, Rice University <br>
  Copyright (c) 2004, University of Chicago <br>  
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>

FIXME: Not actually being used yet and needs fixed

*/

//--------------------------------------------------------------------
//--------------------------------------------------------------------

#ifndef DUGInterface_H
#define DUGInterface_H

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphInterface.hpp>

//--------------------------------------------------------------------

namespace OA {
  namespace DUG {
      
// Changes here must be also reflected in edgeTypeToString 
// and nodeTypeToString.
enum EdgeType { CFLOW_EDGE, CALL_EDGE, RETURN_EDGE, PARAM_EDGE };
//enum NodeType { CFLOW_NODE, CALL_NODE, RETURN_NODE, ENTRY_NODE, EXIT_NODE };
enum NodeType { FORMALPARAM_NODE, NONEFORMAL_NODE };
 
//--------------------------------------------------------------------
/*! DUG implements the DGraph::Interface (directed graph) but has
    enhanced nodes and edges.  Each node corresponds to a basic block in one of
    the procedures and contains a list of statments and each edge is either a
    control flow edge, call edge, or return edge.
*/
  class NodeInterface;
  class EdgeInterface;
  class NodesIteratorInterface;
  class EdgesIteratorInterface;
  class DUGInterface;
 
  typedef std::pair<IRHandle,SymHandle> IRSymHandle;

  //--------------------------------------------------------
  class NodeInterface : public virtual DGraph::NodeInterface {
  public:
    NodeInterface () {}
    ~NodeInterface () { }
    
    //========================================================
    // Info specific to DUG
    //========================================================
    
    virtual NodeType getType() const = 0;

    virtual ProcHandle getProc() const = 0;

    virtual OA_ptr<Location> getLoc() const = 0;
   
    virtual SymHandle getSym() const = 0;
    virtual IRHandle getDef() const = 0;
  
    //! number of statements in node
    virtual unsigned int size () const = 0;

    /*
    //! create a forward order iterator for the statements in the node
    virtual OA_ptr<CFG::NodeStatementsIteratorInterface> 
        getNodeStatementsIterator() const = 0;

    //! create a reverse order iterator for the statements in the node
    virtual OA_ptr<CFG::NodeStatementsRevIteratorInterface> 
        getNodeStatementsRevIterator() const = 0;
        */

    virtual OA_ptr<EdgesIteratorInterface> getDUGIncomingEdgesIterator() const = 0;

    virtual OA_ptr<EdgesIteratorInterface> getDUGOutgoingEdgesIterator() const = 0;

    virtual OA_ptr<NodesIteratorInterface> getDUGSourceNodesIterator() const = 0;

    virtual OA_ptr<NodesIteratorInterface> getDUGSinkNodesIterator() const = 0;

    virtual void markVaried(std::list<CallHandle>&,
                    OA_ptr<Activity::ActivityIRInterface>,
                    std::set<OA_ptr<EdgeInterface> >&,
                    std::set<std::pair<unsigned,unsigned> >&,
                    ProcHandle, SymHandle,
                    OA_ptr<EdgeInterface>) = 0;

    virtual void markUseful(std::list<CallHandle>&,
                    OA_ptr<Activity::ActivityIRInterface>,
                    std::set<OA_ptr<EdgeInterface> >&,
                    std::set<std::pair<unsigned,unsigned> >&,
                    ProcHandle, SymHandle,
                    OA_ptr<EdgeInterface>) = 0;

    virtual bool isPathFrom(OA_ptr<NodeInterface>,
                    std::set<OA_ptr<NodeInterface> >&) = 0;

    virtual void dump(std::ostream& os) = 0;  // for full override
    virtual void dumpbase(std::ostream& os) = 0;
    virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir) = 0;
    virtual void dumpdot(std::ostream& os, OA_ptr<DUGIRInterface> ir) = 0;
    virtual void longdump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir) = 0;
    virtual void output(OA::IRHandlesIRInterface& ir) = 0;

    virtual void findOutgoingNodes(ProcHandle, std::set<IRSymHandle>&, 
                   std::set<IRSymHandle>&) = 0;
    virtual void findIncomingNodes(ProcHandle, std::set<IRSymHandle>&, 
                   std::set<IRSymHandle>&) = 0;
    virtual bool hasEdgesToOtherProc(ProcHandle, std::set<IRSymHandle>&) = 0;
    virtual bool hasEdgesFromOtherProc(ProcHandle, std::set<IRSymHandle>&) = 0;


  };
  
  //--------------------------------------------------------
  class EdgeInterface : public virtual DGraph::EdgeInterface {
  public:
    EdgeInterface () {}
    ~EdgeInterface () {}
    
    //========================================================
    // Info specific to DUG
    //========================================================
    
    virtual EdgeType getType() const = 0;

    virtual ProcHandle getSourceProc() const = 0;
    virtual ProcHandle getSinkProc() const = 0;
    virtual CallHandle getCall() const = 0;
    virtual ProcHandle getProc() const = 0;

    virtual OA_ptr<NodeInterface> getDUGSource() const = 0;

    virtual OA_ptr<NodeInterface> getDUGSink() const = 0;
    virtual void dumpdot(std::ostream& os, OA_ptr<DUGIRInterface> ir) = 0;


  }; 
  
 
  //------------------------------------------------------------------
  /*! An iterator over DUG::Nodes that satisfies the interface
      for all different iterator types.
  */
  class NodesIteratorInterface :  public virtual DGraph::NodesIteratorInterface
  {
  public:
    NodesIteratorInterface () {}
    ~NodesIteratorInterface () {}
    virtual OA_ptr<NodeInterface> currentDUGNode() const = 0;
  };
  
  //------------------------------------------------------------------
  /*! An iterator over DUG::Edges that satisfies the interface
      for all different iterator types.
  */
  class EdgesIteratorInterface :  public virtual DGraph::EdgesIteratorInterface
  {
  public:
    EdgesIteratorInterface () {}
    ~EdgesIteratorInterface () {}
    virtual OA_ptr<EdgeInterface> currentDUGEdge() const = 0;
  };
  

class DUGInterface : public virtual DGraph::DGraphInterface {

//------------------------------------------------------------------
public:
  DUGInterface () { } 
  virtual ~DUGInterface () { }
  
  virtual OA_ptr<NodesIteratorInterface>
      getDUGNodesIterator() const = 0;

  virtual OA_ptr<EdgesIteratorInterface>
      getDUGEdgesIterator() const = 0;

  virtual OA_ptr<NodesIteratorInterface>
      getDUGEntryNodesIterator() const = 0;


  virtual OA_ptr<NodesIteratorInterface>
      getDUGExitNodesIterator() const = 0;

  virtual OA_ptr<NodesIteratorInterface>
    getDUGReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient) = 0;

  virtual OA_ptr<NodesIteratorInterface>
      getDUGDFSIterator(OA_ptr<NodeInterface> n) = 0;

  
};
//--------------------------------------------------------------------

  } // end of DUG namespace
} // end of OA namespace

#endif

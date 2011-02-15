/*! \file
  
  \brief Abstract interface that all CFG analysis results must satisfy.

  \authors Arun Chauhan (2001 as part of Mint), Nathan Tallent, Michelle Strout,
           Andy Stone, Priyadarshini Malusare
  \version $Id: Interface.hpp,v 1.28 2005/06/21 15:20:54 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef CFGINTERFACE_H
#define CFGINTERFACE_H

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphInterface.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphImplement.hpp>

namespace OA {
  namespace CFG {


//======================================================================
/*! This abstract Interface to any CFG assumes a directed graph with
    nodes and edges.  The nodes contain an ordered list of statements.
    The entire program would be represented by a set of CFGs, one for
    each subroutine, and one for the main program.
*/
//=====================================================================
  enum EdgeType { TRUE_EDGE = 0, FALLTHROUGH_EDGE, FALSE_EDGE,
		  BACK_EDGE, MULTIWAY_EDGE, BREAK_EDGE,
		  CONTINUE_EDGE, RETURN_EDGE, NO_EDGE };
    // BK 08/14/08 added NO_EDGE to CFG::EdgeType.  Now that we are
    // decorating ICFG and MPI_ICFG edges with their corresponding
    // CFG::EdgeTypes, we needed a CFG::EdgeType that would signify
    // that there was no corresponding CFG edge.  There should never
    // be any edges within a CFG with the EdgeType of NO_EDGE.

  //------------------------------------------------------------------
  typedef StmtHandleIterator NodeStatementsIteratorInterface;
  typedef StmtHandleIterator NodeStatementsRevIteratorInterface;
  class NodeInterface;
  class EdgeInterface;
  class NodesIteratorInterface;
  class EdgesIteratorInterface;
  class CFGInterface;

  //------------------------------------------------------------------
  /*! Node in the CFG must satisfy the following interface.
  */
  //------------------------------------------------------------------
  class NodeInterface : public virtual DGraph::NodeInterface 
  {
  public:
    NodeInterface () { };
    
    //! create a node with a statement
    NodeInterface (OA::StmtHandle n) { };
    
    //virtual ~Node () = 0;
    virtual ~NodeInterface () { };
    
    //! get an id unique within instances of CFG::Node
    //virtual unsigned int getId() const = 0;
    
    //! number of statements in node
    virtual unsigned int size () const = 0;

    //! create a forward order iterator for the statements in the node
    virtual OA_ptr<NodeStatementsIteratorInterface> getNodeStatementsIterator() const = 0;

    //! create a reverse order iterator for the statements in the node
    virtual OA_ptr<NodeStatementsRevIteratorInterface> 
        getNodeStatementsRevIterator() const = 0;

    virtual OA_ptr<EdgesIteratorInterface> 
        getCFGIncomingEdgesIterator() const = 0;

    virtual OA_ptr<EdgesIteratorInterface> 
        getCFGOutgoingEdgesIterator() const = 0;

    virtual OA_ptr<NodesIteratorInterface> 
        getCFGSourceNodesIterator() const = 0;

    virtual OA_ptr<NodesIteratorInterface> 
        getCFGPredNodesIterator() const = 0;

    virtual OA_ptr<NodesIteratorInterface> 
        getCFGSinkNodesIterator() const = 0;

    virtual OA_ptr<NodesIteratorInterface> 
        getCFGSuccNodesIterator() const = 0;

  
  };
  
  //------------------------------------------------------------------
  /*! Edge in the CFG must satisfy the following interface.
  */
  //------------------------------------------------------------------
  class EdgeInterface : public virtual DGraph::EdgeInterface
  {
  public:
    EdgeInterface () { };
    
    virtual ~EdgeInterface () { };
    
    virtual OA_ptr<NodeInterface> 
        getCFGSource() const = 0;
    
    virtual OA_ptr<NodeInterface> 
        getCFGSink() const = 0;
    
  public:
    // get an id unique within instances of Interface::Edge
    //virtual unsigned int getId() const = 0;

    //! get type associated with edge: TRUE_EDGE, FALLTHROUGH_EDGE, etc.
    virtual EdgeType getType() const = 0;
    
    //! get expression associated with edge, specifies branching val
    virtual ExprHandle getExpr() const = 0;

  };  


class CFGInterface : public virtual DGraph::DGraphInterface
{ 
public:
  CFGInterface() {}
  virtual ~CFGInterface () {}

  //-------------------------------------
  // CFG information access
  //-------------------------------------
  
  //! pointer to entry node
  virtual OA_ptr<NodeInterface> getEntry() const = 0;
  
  //! pointer to entry node
  virtual OA_ptr<NodeInterface> getExit() const = 0;

  //! get SymHandle for ProcHandle (i.e., function name)
  virtual SymHandle getName () const = 0; 
    
  virtual OA_ptr<NodesIteratorInterface> 
      getCFGNodesIterator() const = 0;

  // method to get a more specific iterator
  virtual OA_ptr<EdgesIteratorInterface> 
      getCFGEdgesIterator() const = 0;


  virtual OA_ptr<NodesIteratorInterface> 
      getCFGEntryNodesIterator() const = 0;

  virtual OA_ptr<NodesIteratorInterface> 
      getCFGExitNodesIterator() const = 0;


  virtual OA_ptr<NodesIteratorInterface>
      getCFGReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient) = 0;

  virtual OA_ptr<NodesIteratorInterface>  
      getCFGDFSIterator(OA_ptr<NodeInterface> n) = 0;
};


  //------------------------------------------------------------------
  /*! The NodesIterator is just and extension of DGraph::NodesIterator 
      to provide access to CFG::Interface nodes. 
  */
  class NodesIteratorInterface : 
      public virtual DGraph::NodesIteratorInterface {
  public:

    virtual OA_ptr<NodeInterface> 
        currentCFGNode() const = 0;

  };
  
//------------------------------------------------------------------
  class EdgesIteratorInterface :
      public virtual DGraph::EdgesIteratorInterface {
  public:

     virtual OA_ptr<EdgeInterface> 
         currentCFGEdge() const = 0;
    
  };

  } // end of CFG namespace
} // end of OA namespace

#endif


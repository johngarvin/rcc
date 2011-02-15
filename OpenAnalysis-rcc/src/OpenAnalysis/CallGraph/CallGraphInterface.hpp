/*! \file
  
  \brief Abstract interface that all CFG analysis results must satisfy.

  \authors Nathan Tallent?, Michelle Strout, Priyadarshini Malusare
  \version $Id: Interface.hpp,v 1.13 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef CallGraphInterface_H
#define CallGraphInterface_H

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
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphInterface.hpp>

#include <OpenAnalysis/Utils/Iterator.hpp>
//--------------------------------------------------------------------

namespace OA {
  namespace CallGraph {

//--------------------------------------------------------------------
/*! This abstract Interface to any CallGraph assumes a directed graph
    interface with nodes and edges.
    Each node corresponds to a function definition or
    reference (or both).  
 */
  class CallGraphInterface;    
  class NodesIteratorInterface;
  class EdgesIteratorInterface;
  class NodeInterface;
  class EdgeInterface;
  
  
  // Changes here must be also reflected in CallGraph.C:edgeTypeToString?
  enum EdgeType { NORMAL_EDGE = 0 };
 

class CallGraphInterface : public virtual DGraph::DGraphInterface {
  
public:
  ~CallGraphInterface() { }  
  //-------------------------------------
  // CallGraph information access
  //-------------------------------------
  
  //! iterator over ProcHandles of procs that are may called from the 
  //! given CallHandle
  virtual OA_ptr<ProcHandleIterator> getCalleeProcIter(CallHandle) = 0;

  //---------------------------------------------------------
  // CallGraph iterators
  //---------------------------------------------------------

  virtual OA_ptr<NodesIteratorInterface> getCallGraphNodesIterator() const = 0;

  virtual OA_ptr<EdgesIteratorInterface> getCallGraphEdgesIterator() const = 0;

  virtual OA_ptr<NodesIteratorInterface> getCallGraphEntryNodesIterator() const = 0;

  virtual OA_ptr<NodesIteratorInterface> getCallGraphExitNodesIterator() const = 0;

  virtual OA_ptr<NodesIteratorInterface>
      getCallGraphReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient) = 0;

  virtual OA_ptr<NodesIteratorInterface> getCallGraphDFSIterator(OA_ptr<NodeInterface> n) = 0;

  virtual const SymHandle subprog_name ()  = 0;

  virtual void dump (std::ostream& os, OA_ptr<IRHandlesIRInterface>) = 0;

};




  //------------------------------------------------------------------
  /*! The NodesIterator is just an extension of DGraph::NodesIterator 
      to provide access to CFG::Interface nodes. 
  */
  class NodesIteratorInterface : public virtual DGraph::NodesIteratorInterface {
  public:
  
    ~NodesIteratorInterface() { }
    
    virtual OA_ptr<NodeInterface> currentCallGraphNode() const = 0;
    
  };


  
  //------------------------------------------------------------------
  class EdgesIteratorInterface : public virtual DGraph::EdgesIteratorInterface {
  public:

    ~EdgesIteratorInterface() { }  
    
    virtual OA_ptr<EdgeInterface> currentCallGraphEdge() const = 0;
    
  };
  

  class NodeCallsIterator;
  //--------------------------------------------------------
  class NodeInterface : public virtual DGraph::NodeInterface {
  public:
      
    ~NodeInterface() { }  
      
    //! getId: An id unique within instances of CallGraph::Node
    //virtual unsigned int getId() const = 0;
    
    virtual bool isDefined() const = 0;
    
    virtual bool isCalled() const  = 0;
    
    virtual ProcHandle getProc() const = 0;
    
    virtual SymHandle getProcSym() const = 0;
    
    virtual OA_ptr<EdgesIteratorInterface> getCallGraphIncomingEdgesIterator() const = 0;

    virtual OA_ptr<EdgesIteratorInterface> getCallGraphOutgoingEdgesIterator() const = 0;

    virtual OA_ptr<NodesIteratorInterface> getCallGraphSourceNodesIterator() const = 0;

    virtual OA_ptr<NodesIteratorInterface> getCallGraphSinkNodesIterator() const = 0;
    
  };
  


  //--------------------------------------------------------
  class EdgeInterface : public virtual DGraph::EdgeInterface {
  public:
   
    ~EdgeInterface() { }  
    
    //! getId: An id unique within instances of CallGraph::Edge
    //virtual unsigned int getId() const = 0;
    
    //! Returns the handle for the callsite that induces this edge
    virtual CallHandle getCallHandle() const = 0;
    
    virtual EdgeType getType() const = 0;

    virtual OA_ptr<NodeInterface> getCallGraphSource() const = 0;

    virtual OA_ptr<NodeInterface> getCallGraphSink() const = 0;

    
  };  


  //--------------------------------------------------------
  
  class NodeCallsIteratorInterface : public Iterator 
  {
  public:
      
    ~NodeCallsIteratorInterface() { }  
      
    virtual void operator ++ () = 0;
    
    virtual bool isValid() = 0;
    
    virtual CallHandle current() = 0;
    
  };
  
//--------------------------------------------------------------------

  } // end of CallGraph namespace
} // end of OA namespace

#endif

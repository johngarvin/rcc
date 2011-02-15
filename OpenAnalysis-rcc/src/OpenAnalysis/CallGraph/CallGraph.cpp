/*! \file
  
  \brief Implementation of CallGraph.

  Implements the CallGraph::Interface for viewing the contents and structure
  of a CallGraph.

  Also implements methods useful for constructing a CallGraph.

  \authors Arun Chauhan (2001 as part of Mint), Nathan Tallent, Michelle Strout, Barbara Kreaseck
  \version $Id: CallGraph.cpp,v 1.14 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

//--------------------------------------------------------------------
//--------------------------------------------------------------------

// standard headers

#ifdef NO_STD_CHEADERS
# include <stdlib.h>
# include <string.h>
# include <assert.h>
#else
# include <cstdlib>
# include <cstring>
# include <cassert>
using namespace std; // For compatibility with non-std C headers
#endif

#include <iostream>
using std::ostream;
using std::endl;
using std::cout;
using std::cerr;

// Mint headers
#include "CallGraph.hpp"

namespace OA {
  namespace CallGraph {

static bool debug = false;

//--------------------------------------------------------------------

//--------------------------------------------------------------------
// must be updated any time CallGraph::Interface::EdgeType changes
static const char *sEdgeTypeToString[] = { 
  "NORMAL_EDGE" // FIXME (make CFG's version static)
};


//--------------------------------------------------------------------

CallGraphCalleeProcIter::CallGraphCalleeProcIter(OA_ptr<std::set<ProcHandle> > pSet)
        : IRHandleSetIterator<ProcHandle>(pSet)  {}

CallGraphCalleeProcIter::~CallGraphCalleeProcIter() {}

void CallGraphCalleeProcIter::operator++() { IRHandleSetIterator<ProcHandle>::operator++(); }


bool CallGraphCalleeProcIter::isValid() const
      { return IRHandleSetIterator<ProcHandle>::isValid(); }

ProcHandle CallGraphCalleeProcIter::current() const
      { return IRHandleSetIterator<ProcHandle>::current(); }


void CallGraphCalleeProcIter::reset() { IRHandleSetIterator<ProcHandle>::reset(); }


NodesIterator::NodesIterator(OA_ptr<DGraph::NodesIteratorInterface> ni)
        : DGraph::NodesIteratorImplement(ni) {}

EdgesIterator::EdgesIterator(OA_ptr<DGraph::EdgesIteratorInterface> ni)
    : DGraph::EdgesIteratorImplement(ni) {}

Node::Node() : mSym(0), mProc(0) {  }

Node::Node(SymHandle s) : mSym(s), mProc(0)
        {  }

Node::~Node () { }

bool Node::isDefined() const { return (mProc.hval() != 0); }

bool Node::isCalled() const { return (mCalls.size() != 0); }

ProcHandle Node::getProc() const { return mProc; }

SymHandle Node::getProcSym() const { return mSym; }

void Node::add_def(ProcHandle h) { mProc = h; }

void Node::add_call(CallHandle h) { mCalls.push_back(h); }

Edge::Edge(OA_ptr<DGraph::NodeInterface> source,
             OA_ptr<DGraph::NodeInterface> sink, EdgeType _type,
             OA::CallHandle call) : DGraph::EdgeImplement (source, sink), mType(_type), mCallsiteExpr(call)
{
}

/*
Edge::Edge(OA_ptr<DGraph::NodeInterface> source,
             OA_ptr<DGraph::NodeInterface> sink, EdgeType _type,
             CallHandle call) : DGraph::EdgeImplement (source, sink), mType(_type), mCallsiteExpr(call)         {
            }
*/

Edge::~Edge () {}

/*
unsigned int Edge::getId() const { return mDGEdge.getId(); }
*/

EdgeType Edge::getType() const { return mType; }

CallHandle Edge::getCallHandle() const { return mCallsiteExpr; }

/*
OA_ptr<DGraph::NodeInterface> Edge::getSource () const
        {

            return mDGEdge.getSource();
        }

OA_ptr<DGraph::NodeInterface> Edge::getSink () const
        {



            return mDGEdge.getSink();
        }
*/

OA_ptr<NodeInterface> Edge::getCallGraphSource() const
{
           return getSource().convert<NodeInterface>();

}

OA_ptr<NodeInterface> Edge::getCallGraphSink() const
{
           return getSink().convert<NodeInterface>();
}

/*
bool Edge::operator== (DGraph::EdgeInterface& other)
{
    return mDGEdge.operator==(other);
}

bool Edge::operator< (DGraph::EdgeInterface& other)
{
    return mDGEdge.operator<(other);
}
*/

/*
int CallGraph::getNumNodes() { return mDGraph.getNumNodes(); }

int CallGraph::getNumEdges() { return mDGraph.getNumEdges(); }
*/

CallGraph::CallGraph(const SymHandle name) 
  : mName(name)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_CallGraph:ALL", debug);
}

CallGraph::CallGraph() 
  : mName(SymHandle(0))
{
}


CallGraph::~CallGraph()
{
}

/*
void CallGraph::addNode(OA_ptr<DGraph::NodeInterface> n)
{
  mDGraph.addNode(n);
}

void CallGraph::addEdge(OA_ptr<DGraph::EdgeInterface> e)
{

   mDGraph.addEdge(e);
}


void CallGraph::removeEdge(OA_ptr<DGraph::EdgeInterface> e)
    {
        mDGraph.removeEdge(e);
    }

void CallGraph::removeNode(OA_ptr<DGraph::NodeInterface> n)
    {
        mDGraph.removeNode(n);
    }
*/


OA_ptr<ProcHandleIterator> CallGraph::getCalleeProcIter(CallHandle call)
    {
      OA_ptr<ProcHandleIterator> retIter;
      OA_ptr<std::set<ProcHandle> > procSet;
      procSet = mCallToCalleeProcSetMap[call];
      if (procSet.ptrEqual(NULL)) {
        procSet = new std::set<ProcHandle>;
      }
      retIter = new CallGraphCalleeProcIter(procSet);
      return retIter;
    }



//--------------------------------------------------------------------
OA_ptr<NodeInterface> NodesIterator::currentCallGraphNode() const {
   return current().convert<Node>();
}

OA_ptr<EdgeInterface> EdgesIterator::currentCallGraphEdge() const {
   return current().convert<Edge>();
}

/*
unsigned int Node::getId() const { return mDGNode.getId(); }

int Node::num_incoming () const
{
    return mDGNode.num_incoming();
}

int Node::num_outgoing () const
{
    return mDGNode.num_outgoing();
}

bool Node::isAnEntry() const
{
    return mDGNode.isAnEntry();
}

bool Node::isAnExit() const
{
    return mDGNode.isAnExit();
}


bool Node::operator== (DGraph::NodeInterface& other)
{
    return mDGNode.operator==(other);
}

bool Node::operator< (DGraph::NodeInterface& other)
{
    return mDGNode.operator<(other);
}

void Node::addIncomingEdge(OA_ptr<DGraph::EdgeInterface> e)
{
    mDGNode.addIncomingEdge(e);
}

void Node::addOutgoingEdge(OA_ptr<DGraph::EdgeInterface> e)
{
     mDGNode.addOutgoingEdge(e);
}

void Node::removeIncomingEdge(OA_ptr<DGraph::EdgeInterface> e)
{
    mDGNode.removeIncomingEdge(e);
}

void Node::removeOutgoingEdge(OA_ptr<DGraph::EdgeInterface> e)
{
     mDGNode.removeOutgoingEdge(e);
}


OA_ptr<DGraph::EdgesIteratorInterface> Node::getIncomingEdgesIterator() const
      {
         return mDGNode.getIncomingEdgesIterator();
      }


OA_ptr<DGraph::EdgesIteratorInterface> Node::getOutgoingEdgesIterator() const
      {
         return mDGNode.getOutgoingEdgesIterator();
      }

OA_ptr<DGraph::NodesIteratorInterface> Node::getSourceNodesIterator() const
      {
         return mDGNode.getSourceNodesIterator();
      }

OA_ptr<DGraph::NodesIteratorInterface> Node::getSinkNodesIterator() const
      {
         return mDGNode.getSinkNodesIterator();
      }
*/

OA_ptr<EdgesIteratorInterface> Node::getCallGraphIncomingEdgesIterator() const
      {
         OA_ptr<EdgesIterator> retval;
         retval = new EdgesIterator(getIncomingEdgesIterator());
         return retval;

      }

OA_ptr<EdgesIteratorInterface> Node::getCallGraphOutgoingEdgesIterator() const
      {
         OA_ptr<EdgesIterator> retval;
         retval = new EdgesIterator(getOutgoingEdgesIterator());
         return retval;

      }

OA_ptr<NodesIteratorInterface> Node::getCallGraphSourceNodesIterator() const
      {
         OA_ptr<NodesIterator> retval;
         retval = new NodesIterator(getSourceNodesIterator());
         return retval;
      }


OA_ptr<NodesIteratorInterface> Node::getCallGraphSinkNodesIterator() const
      {
         OA_ptr<NodesIterator> retval;
         retval = new NodesIterator(getSinkNodesIterator());
         return retval;

      }


//--------------------------------------------------------------------


void Edge::dump(ostream& os)
{
  os << sEdgeTypeToString[mType];
}

void Edge::output(IRHandlesIRInterface& ir) const
{
    // always get NORMAL_EDGE so it just clutters things
    //sOutBuild->outputString(  sEdgeTypeToString[mType] );
  
  sOutBuild->outputString( ir.toString(mCallsiteExpr) );
}

//--------------------------------------------------------------------

void CallGraph::connect (OA_ptr<NodeInterface> src, 
              OA_ptr<NodeInterface> dst, 
              EdgeType type, CallHandle call) 
{
    OA_ptr<Edge> e; e = new Edge (src, dst, type, call);

    addEdge (e);
}


void CallGraph::disconnect (OA_ptr<EdgeInterface> e)
{
    removeEdge(e);
}


OA_ptr<Node> 
CallGraph::findOrAddNode(SymHandle sym)
{
  OA_ptr<Node> node; node = NULL;

  SymToNodeMapType::iterator it = mSymToNodeMap.find(sym);
  if (it == mSymToNodeMap.end()) {
    node = new Node(sym);
    addNode(node);
    mSymToNodeMap.insert(SymToNodeMapType::value_type(sym, node));
  } else {
    node = (*it).second;
  }
  assert(!node.ptrEqual(NULL));
  
  return node;
}

//--------------------------------------------------------------------

void CallGraph::addToCallProcSetMap(CallHandle call, ProcHandle proc) 
{
  OA_ptr<std::set<ProcHandle> > procSet;  procSet = NULL;

  std::map<CallHandle, OA_ptr<std::set<ProcHandle> > >::iterator it;
  it = mCallToCalleeProcSetMap.find(call);
  if (it == mCallToCalleeProcSetMap.end()) {
    procSet = new std::set<ProcHandle>;
    mCallToCalleeProcSetMap.insert(std::map<CallHandle, 
                  OA_ptr<std::set<ProcHandle> > >::value_type(call,procSet));
  }
  mCallToCalleeProcSetMap[call]->insert(proc);

  // checking (keeping this here to test on UseOA-Rose -- BK
  if (debug) {
    it = mCallToCalleeProcSetMap.find(call);
    if (it == mCallToCalleeProcSetMap.end()) {
      std::cout << "++++++ map insert failed (cannot even find call)\n\n";
    } else {
      procSet = (*it).second;
      std::set<ProcHandle>::iterator psit;
      bool found = false;
      for (psit = procSet->begin(); psit != procSet->end(); (psit++) ) {
        if ((*psit)==proc) {
          found = true;
        }
      }
      if (found) {
        std::cout << "++++++ map insert succeeded\n\n";
      } else {
        std::cout << "++++++ map insert failed (found call, but not proc)\n\n";
      }
    }
  }

}




/*
OA_ptr<DGraph::NodesIteratorInterface> CallGraph::getNodesIterator() const
  {
      return mDGraph.getNodesIterator();
  }


OA_ptr<DGraph::EdgesIteratorInterface> CallGraph::getEdgesIterator() const
  {
      return mDGraph.getEdgesIterator();
  }


OA_ptr<DGraph::NodesIteratorInterface>
      CallGraph::getReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient)
    {
        return mDGraph.getReversePostDFSIterator(pOrient);
    }


OA_ptr<DGraph::NodesIteratorInterface>
      CallGraph::getDFSIterator(OA_ptr<DGraph::NodeInterface> n){

          return mDGraph.getDFSIterator(n);
      }



OA_ptr<DGraph::NodesIteratorInterface> CallGraph::getEntryNodesIterator() const
{
   return mDGraph.getEntryNodesIterator();
}


OA_ptr<DGraph::NodesIteratorInterface> CallGraph::getExitNodesIterator() const
{
 return mDGraph.getExitNodesIterator();
}
*/


OA_ptr<NodesIteratorInterface> CallGraph::getCallGraphNodesIterator() const
  {
       OA_ptr<NodesIterator> retval;
       retval = new NodesIterator(getNodesIterator());
       return retval;

  }


OA_ptr<EdgesIteratorInterface> CallGraph::getCallGraphEdgesIterator() const
  {
       OA_ptr<EdgesIterator> retval;
       retval = new EdgesIterator(getEdgesIterator());
       return retval;

  }


OA_ptr<NodesIteratorInterface>
      CallGraph::getCallGraphReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient)
    {
         OA_ptr<NodesIterator> retval;
         retval = new NodesIterator(getReversePostDFSIterator(pOrient));
         return retval;

    }


OA_ptr<NodesIteratorInterface>
     CallGraph::getCallGraphDFSIterator(OA_ptr<NodeInterface> n)
{
     OA_ptr<NodesIterator> retval;
     retval = new NodesIterator(getDFSIterator(n));
     return retval;

}


OA_ptr<NodesIteratorInterface> CallGraph::getCallGraphEntryNodesIterator() const
{
     OA_ptr<NodesIterator> retval;
     retval = new NodesIterator(getEntryNodesIterator());
     return retval;

}


OA_ptr<NodesIteratorInterface> CallGraph::getCallGraphExitNodesIterator() const
{
     OA_ptr<NodesIterator> retval;
     retval = new NodesIterator(getExitNodesIterator());
     return retval;

}



//--------------------------------------------------------------------
void
CallGraph::dump (ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    /*! COMMENTED OUT BY plm 08/17/06
  os << "===== CallGraph: " << ir->toString(mName) << " =====\n"
     << endl;
  
  // print the contents of all the nodes
  NodesIterator nodes_iter(*this);
  for ( ; nodes_iter.isValid(); ++nodes_iter) {
    OA_ptr<CallGraph::Node> node = nodes_iter.current();
    node->longdump(os, ir);
    os << endl;
  }
  
  os << "====================" << endl;
*/
}
//--------------------------------------------------------------------

void CallGraph::output(OA::IRHandlesIRInterface& ir) const {

  // perform output from superclass
  //mDGraph.output(ir);
  
  DGraph::DGraphImplement::output(ir);

  // now output map info:  CallHandle to ProcHandle set
  sOutBuild->mapStart("mCallToCalleeProcSetMap", "CallHandle", 
                      "OA_ptr<std::set<ProcHandle> > ");
  std::map<CallHandle, OA_ptr<std::set<ProcHandle> > >::const_iterator mapIter;

  mapIter = mCallToCalleeProcSetMap.begin();

  for (; 
       mapIter != mCallToCalleeProcSetMap.end(); mapIter++) {

    CallHandle call = mapIter->first;
    if (call==CallHandle(0)) continue;

    sOutBuild->mapEntryStart();
    sOutBuild->mapKey(ir.toString(call));
    sOutBuild->mapValueStart();

    OA_ptr<std::set<ProcHandle> > procSet;
    procSet = mapIter->second;
    if (!procSet.ptrEqual(0)) {
      sOutBuild->listStart();
      CallGraphCalleeProcIter procIter(procSet);
      for (; procIter.isValid(); ++procIter) {
        ProcHandle proc = procIter.current();
        sOutBuild->listItemStart();
        sOutBuild->outputString( ir.toString(proc) );
        sOutBuild->listItemEnd();
      }
      sOutBuild->listEnd();
    }

    sOutBuild->mapValueEnd();
    sOutBuild->mapEntryEnd();
  }

  sOutBuild->mapEnd("mCallToCalleeProcSetMap");
}

//--------------------------------------------------------------------
void
Node::dump (ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
  if (mSym.hval() == 0) {
    os << "<no-symbol>";
  } else {
    os << ir->toString(mSym);
  }
  os << " " << (isDefined() ? "[defined]" : "[referenced]");
}

void
Node::output(OA::IRHandlesIRInterface& ir) const
{
    std::stringstream label;
    if (mSym.hval() == 0) {
      label << "<no-symbol>";
    } else {
      label << ir.toString(mSym);
    }
    label << " " << (isDefined() ? "[defined]" : "[referenced]");
    sOutBuild->outputString(label.str());
}

void
Node::longdump ( ostream& os, 
                                   OA_ptr<IRHandlesIRInterface> ir)
{
    /*! Commented out by PLM 08/18/06
  // print the node ID
  os << "CallGraph Node: ";
  dump(os,ir);
  
  if (isAnEntry()) {
    os << " (ENTRY)";
  } else if (isAnExit()) {
    os << " (EXIT)"; 
  }
  os << endl;
  
  // print the source(s)
  unsigned int count = 0;
  OA_ptr<NodesIterator> srcIt = getSourceNodesIterator();
  for ( ; srcIt->isValid(); ++(*srcIt), ++count) {
    OA_ptr<Node> node = srcIt->current();
    if (count == 0) { os << " <-- ("; }
    else            { os << ", "; }

    node->dump(os,ir);
  }
  if (count > 0) { os << ")" << endl; }
  
  // print the sink(s)
  count = 0;
  //OutgoingEdgesIterator outIt(*this);
  OA_ptr<NodesIterator> outIt = getSinkNodesIterator();
  for ( ; outIt->isValid(); ++(*outIt), ++count) {
    //OA_ptr<Edge> edge = outIt.current();
    //OA_ptr<Node> node = edge->sink().convert<Node>();
    OA_ptr<Node> node = outIt->current();
    if (count == 0) { os << " --> ("; } 
    else            { os << ", "; }
    
    node->dump(os,ir);
    //os << " [";
    //edge->shortdump(os);
    //os << "]";
  }
  if (count > 0) { os << ")" << endl; }
*/
}

//--------------------------------------------------------------------

  } // end namespace CallGraph
} // end namespace OA

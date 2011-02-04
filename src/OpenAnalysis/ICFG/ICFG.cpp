/*! \file
  
  \brief Implementation of ICFG.

  \authors Michelle Strout, Barbara Kreaseck, Priyadarshini Malusare
  \version $Id: ICFG.cpp,v 1.2.6.1 2006/01/19 05:30:48 mstrout Exp $

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

#include "ICFG.hpp"

#include <OpenAnalysis/Utils/OutputBuilder.hpp>
namespace OA {
  namespace ICFG {

static bool debug = false;

//--------------------------------------------------------------------

//--------------------------------------------------------------------
// must be updated any time ICFG::Interface::EdgeType changes
static const char *sEdgeTypeToString[] = { 
  "CFLOW_EDGE",
  "CALL_EDGE",
  "RETURN_EDGE",
  "CALL_RETURN_EDGE"
};
// must be updated any time ICFG::Interface::EdgeType changes
static const char *sNodeTypeToString[] = { 
  "CFLOW_NODE",
  "CALL_NODE",
  "RETURN_NODE",
  "ENTRY_NODE",
  "EXIT_NODE"
};

//--------------------------------------------------------------------
/*! called by all Node constructors
 */

Node::Node (OA_ptr<ICFG> pICFG, ProcHandle proc, NodeType pType)
        : DGraph::NodeImplement(), mICFG(pICFG), mProc(proc), mType(pType) { Ctor(); }

Node::Node (OA_ptr<ICFG> pICFG, ProcHandle proc, NodeType pType, CallHandle call)
        : DGraph::NodeImplement(), mICFG(pICFG), mProc(proc), mType(pType), mCall(call) { Ctor(); }

Node::Node (OA_ptr<ICFG> pICFG, ProcHandle proc, NodeType pType,
          OA_ptr<CFG::NodeInterface> cNode)
        : DGraph::NodeImplement(), mICFG(pICFG), mProc(proc), mType(pType), mCFGNode(cNode) { Ctor(); }

Node::Node (Node& other)
      : DGraph::NodeImplement(), mICFG(other.mICFG), mProc(other.mProc),
        mType(other.mType), mCFGNode(other.mCFGNode) { Ctor(); }

Node::~Node () { }

NodeType Node::getType() const { return mType; }

ProcHandle Node::getProc() const { return mProc; }

CallHandle Node::getCall() const { return mCall; }


void Node::addCallEdge(OA_ptr<Edge> e) { mCallEdges->push_back(e); }

void Node::addReturnEdge(OA_ptr<Edge> e) { mReturnEdges->push_back(e); }

void Node::removeCallEdge( OA_ptr<Edge> e) { mCallEdges->remove(e); }

void Node::removeReturnEdge( OA_ptr<Edge> e) { mReturnEdges->remove(e); }

/*
unsigned int Node::getId() const { return mDGNode.getId(); }

int Node::num_incoming () const { return mDGNode.num_incoming(); }

int Node::num_outgoing () const { return mDGNode.num_outgoing(); }

bool Node::isAnEntry() const { return mDGNode.isAnEntry(); }

bool Node::isAnExit() const { return mDGNode.isAnExit(); }
*/

void Node::Ctor() { 
    mCallEdges = new std::list<OA_ptr<Edge> >;
    mReturnEdges = new std::list<OA_ptr<Edge> >;
}

//! number of statements in node
unsigned int Node::size () const
{
    return mCFGNode->size(); 
}

//! create a forward order iterator for the statements in the node
OA_ptr<CFG::NodeStatementsIteratorInterface> 
Node::getNodeStatementsIterator() const
{
    return mCFGNode->getNodeStatementsIterator();
}

//! create a reverse order iterator for the statements in the node
OA_ptr<CFG::NodeStatementsRevIteratorInterface> 
Node::getNodeStatementsRevIterator() const
{
    return mCFGNode->getNodeStatementsRevIterator();
}


//--------------------------------------------------------------------
void
Node::dump (ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
  os << sNodeTypeToString[mType] << std::endl;
//  mCFGNode->dump(os,ir);
}

/*
bool Node::operator==(DGraph::NodeInterface& other) 
{ 
    return mDGNode.operator==(other);
}

bool Node::operator<(DGraph::NodeInterface& other) 
{
    return mDGNode.operator<(other);
}
*/

void
Node::longdump (ICFG& icfg, 
                              ostream& os, 
                              OA_ptr<IRHandlesIRInterface> ir)
{
    /*
  // print the node ID
  os << "ICFGStandard Node: ";
  dump(os,ir);
  
  if (isAnEntry()) {
    os << " (ENTRY)";
  } else if (isAnExit()) {
    os << " (EXIT)"; 
  }
  os << endl;
  
  // print the source(s)
  unsigned int count = 0;
  OA_ptr<NodesIteratorInterface> srcIter = getICFGSourceNodesIterator();
  for ( ; srcIter->isValid(); ++(*srcIter), ++count) {
    OA_ptr<NodeInterface> node = srcIter->currentICFGNode();
    if (count == 0) { os << " <-- ("; }
    else            { os << ", "; }

    node->dump(os,ir);
  }
  if (count > 0) { os << ")" << endl; }
  
  // print the sink(s)
  count = 0;
  OA_ptr<NodesIterator> outIter = getICFGSinkNodesIterator();
  for ( ; outIter->isValid(); ++(*outIter), ++count) {
    OA_ptr<Node> node = outIter->current();
    if (count == 0) { os << " --> ("; } 
    else            { os << ", "; }
    
    node->dump(os,ir);
  }
  if (count > 0) { os << ")" << endl; }
  */
}


void
Node::dumpdot (ICFG &currcfg, ostream& os, 
                             OA_ptr<IRHandlesIRInterface> ir)
{
    /*
  // print the node
  os << getId() << " [ label=\"==  ICFG ";
  os << sNodeTypeToString[getType()] << " " << getId() << " ==";
  
  if (isAnEntry()) {
    os << " (entry)";
  } 
  else if (isAnExit()) {
    os << " (exit)"; 
  }
  os << "\\n"; 

  // print the node contents
  OA_ptr<CFG::Interface::NodeStatementsIterator> stmtIter 
      = getNodeStatementsIterator();

//  for ( ; stmtIter->isValid(); ++(*stmtIter)) {
//    StmtHandle s = stmtIter->current();
//    os << std::endl << ir->toString(s) << "\\n";
//  }

  if (stmtIter->isValid()) {
    os << "\\n"; // internal dot line break
  }
  for ( ; stmtIter->isValid(); ++(*stmtIter)) {
    StmtHandle s = stmtIter->current();
    std::string st = ir->toString(s);

    // replace any '\"' in fortran code with '\''
    // because it messes up the dot files
    // (could replace each '\"' with two characters: '\\' '\"' )
    char * cst;
    cst = (char *)malloc(strlen(st.c_str()) + 1);
    strcpy(cst, st.c_str());
    for (unsigned int j = 0; j < strlen(cst); j++) {
      if (cst[j]=='\"') {
        cst[j]='\'';
      }
    }

    os << cst << "\\n";
  }

  os << "\" ];" << endl;
  os.flush();
  */
}

void Node::output(OA::IRHandlesIRInterface& ir) {

    std::ostringstream label;

    label <<  "==  ICFG ";
    label << sNodeTypeToString[getType()] << " " << getId() << " ==";
    if (isAnEntry()) {
      label << " (entry)";
    } 
    else if (isAnExit()) {
      label << " (exit)";
    }
    label << "\n";

    if(mType == CALL_NODE) {

      OA::CallHandle call = getCall();
      std::string st = ir.toString(call);
      char * cst;
      cst = (char *)malloc(strlen(st.c_str()) + 1);
      strcpy(cst, st.c_str());
      label << cst << "\n";

    } else {

      // print the node contents
      OA_ptr<CFG::NodeStatementsIteratorInterface> stmtIter 
        = getNodeStatementsIterator();
      for ( ; stmtIter->isValid(); ++(*stmtIter)) {
           StmtHandle s = stmtIter->current();
           std::string st = ir.toString(s);
        
           // replace any '\"' in fortran code with '\''
           // because it messes up the dot files
           // (could replace each '\"' with two characters: '\\' '\"' )
           char * cst;
           cst = (char *)malloc(strlen(st.c_str()) + 1);
           strcpy(cst, st.c_str());
           for (unsigned int j = 0; j < strlen(cst); j++) {
                if (cst[j]=='\"') {
                  cst[j]='\'';
                }
           }
           label << cst << "\n";
       }
    }

    sOutBuild->outputString(label.str());

}


/*
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
*/



//--------------------------------------------------------------------

/*
OA_ptr<DGraph::EdgesIteratorInterface> 
    Node::getIncomingEdgesIterator() const
{
    
    return mDGNode.getIncomingEdgesIterator();
}

OA_ptr<DGraph::EdgesIteratorInterface> 
    Node::getOutgoingEdgesIterator() const
{
    return mDGNode.getOutgoingEdgesIterator();
}

OA_ptr<DGraph::NodesIteratorInterface> 
    Node::getSourceNodesIterator() const
{
    return mDGNode.getSourceNodesIterator();
}

OA_ptr<DGraph::NodesIteratorInterface> 
    Node::getSinkNodesIterator() const
{
    return mDGNode.getSinkNodesIterator();
}
*/

//-------------------------------------------------------------------

OA_ptr<EdgesIteratorInterface>
    Node::getICFGIncomingEdgesIterator() const
{
   OA_ptr<EdgesIterator> retval;
   retval = new EdgesIterator(getIncomingEdgesIterator());
   return retval;

}

OA_ptr<EdgesIteratorInterface>
    Node::getICFGOutgoingEdgesIterator() const
{
   OA_ptr<EdgesIterator> retval;
   retval = new EdgesIterator(getOutgoingEdgesIterator());
   return retval;

}


bool lt_Node::operator()(const OA_ptr<DGraph::NodeInterface> n1,
                    const OA_ptr<DGraph::NodeInterface> n2) const {
      return (n1->getId() < n2->getId());
    }



bool lt_Edge::operator()(const OA_ptr<DGraph::EdgeInterface> e1,
                      const OA_ptr<DGraph::EdgeInterface> e2) const
{
    unsigned int src1 = e1->getSource()->getId();
    unsigned int src2 = e2->getSource()->getId();
    if (src1 == src2) {
        return (e1->getSink()->getId() < e2->getSink()->getId());
    } else {
        return (src1 < src2);
    }
}



OA_ptr<NodesIteratorInterface>
    Node::getICFGSourceNodesIterator() const
{
   OA_ptr<NodesIterator> retval;
   retval = new NodesIterator(getSourceNodesIterator());
   return retval;

}

OA_ptr<NodesIteratorInterface>
    Node::getICFGSinkNodesIterator() const
{
   OA_ptr<NodesIterator> retval;
   retval = new NodesIterator(getSinkNodesIterator());
   return retval;

}

// -----------------------------------------------------------------

Edge::Edge (OA_ptr<ICFG> pICFG, OA_ptr<DGraph::NodeInterface> pNode1, 
            OA_ptr<DGraph::NodeInterface> pNode2, EdgeType pType,
            CallHandle call, CFG::EdgeType cfgType, ExprHandle cfgExpr)
    : DGraph::EdgeImplement (pNode1, pNode2), mICFG(pICFG), 
      mNode1(pNode1.convert<Node>()), mNode2(pNode2.convert<Node>()), 
      mType(pType), mCall(call),
      mCFGType(cfgType), mCFGExpr(cfgExpr)
    {
    }



Edge::Edge (OA_ptr<ICFG> pICFG, OA_ptr<DGraph::NodeInterface> pNode1, 
            OA_ptr<DGraph::NodeInterface> pNode2, EdgeType pType,
            CFG::EdgeType cfgType, ExprHandle cfgExpr)
    : DGraph::EdgeImplement (pNode1, pNode2), mICFG(pICFG), 
      mNode1(pNode1.convert<Node>()), mNode2(pNode2.convert<Node>()),
      mType(pType), mCall(CallHandle(0)),
      mCFGType(cfgType), mCFGExpr(cfgExpr)
    {
    }



Edge::~Edge () {}

EdgeType Edge::getType() const { return mType; }

ProcHandle Edge::getSourceProc() const { return mNode1->getProc(); }

ProcHandle Edge::getSinkProc() const { return mNode2->getProc(); }

CallHandle Edge::getCall() const { return mCall; }

CFG::EdgeType Edge::getCFGType() const { return mCFGType; }

ExprHandle Edge::getCFGExpr() const { return mCFGExpr; }


/*
unsigned int Edge::getId() const { return mDGEdge.getId(); }

OA_ptr<DGraph::NodeInterface> Edge::getSource () const { return mDGEdge.getSource(); }

OA_ptr<DGraph::NodeInterface> Edge::getTail () const { return mDGEdge.getSource(); }

OA_ptr<DGraph::NodeInterface> Edge::getSink () const { return mDGEdge.getSink(); }

OA_ptr<DGraph::NodeInterface> Edge::getHead () const { return mDGEdge.getSink(); }
*/

 OA_ptr<NodeInterface> Edge::getICFGSource() const
    {
        return getSource().convert<Node>();
    }

    OA_ptr<NodeInterface> Edge::getICFGSink() const
    {
        return getSink().convert<Node>();
    }


/*
bool Edge::operator==(DGraph::EdgeInterface& other) 
{
    //! commented out by PLM 08/21/06
    //Edge& recastOther = dynamic_cast<Edge&>(other);
    //return mDGEdge == recastOther.mDGEdge; 
    
    return mDGEdge.operator==(other);
}

bool Edge::operator<(DGraph::EdgeInterface& other) 
{ 
    return mDGEdge.operator<(other);
}
*/


void Edge::dump(ostream& os)
{
  os << sEdgeTypeToString[mType];
}

void Edge::dumpdot(ostream& os)
{
    os << getSource()->getId() << " -> " << getSink()->getId(); 
    switch(mType) {
        case (CALL_EDGE):
            os << " [label=\"CALL_EDGE\", style=dashed,color=red];" 
               << std::endl;
            break;
        case (RETURN_EDGE):
            os << " [label=\"RETURN_EDGE\", style=dashed,color=blue];" 
               << std::endl;
            break;
        case (CFLOW_EDGE):
            os << ";" << std::endl;
            break;
         case (CALL_RETURN_EDGE):
           os << " [label=\"CALL_RETURN_EDGE\", style=dashed];" << std::endl;
    }

    os.flush();
}

void Edge::output(OA::IRHandlesIRInterface& ir) {

    std::ostringstream label;

    if (debug) {
      label << "<Edge_" << getId() << ":" << getSource()->getId()
            << "==>" << getSink()->getId();
      label << " [from CFG: " << CFG::edgeTypeToString(mCFGType);
      if (mCFGExpr != ExprHandle(0)) {
        label << ", Expr: " << ir.toString(mCFGExpr);
      }
      label << "] ";
      label << "> ";
    }

    switch(mType) {
        case (CALL_EDGE):
          label << "CALL_(" << ir.toString(mCall) << ")";
            break;
        case (RETURN_EDGE):
          label << "RETURN_(" << ir.toString(mCall) << ")";
            break;
        case (CFLOW_EDGE):
            break;
        case (CALL_RETURN_EDGE):
          label << "CALL_RETURN";
            break;
    }

    sOutBuild->outputString( label.str() );
}

NodesIterator::NodesIterator(OA_ptr<DGraph::NodesIteratorInterface> ni)
        : DGraph::NodesIteratorImplement(ni) {}


EdgesIterator::EdgesIterator(OA_ptr<DGraph::EdgesIteratorInterface> ni)
       : DGraph::EdgesIteratorImplement(ni) {}


   OA_ptr<EdgeInterface> EdgesIterator::currentICFGEdge() const
      {
         return current().convert<Edge>();
      }



   OA_ptr<NodeInterface> NodesIterator::currentICFGNode() const
      {
           return current().convert<Node>();
      }



//--------------------------------------------------------------------
//--------------------------------------------------------------------
/*
 int ICFG::getNumNodes() { return mDGraph.getNumNodes(); }
  int ICFG::getNumEdges() { return mDGraph.getNumEdges(); }
*/



ICFG::ICFG() 
{
  OA_DEBUG_CTRL_MACRO("DEBUG_ICFG", debug);

  mEntry = mExit = NULL;
  mCallNodes = new std::list<OA_ptr<Node> >;
  mReturnNodes = new std::list<OA_ptr<Node> >;
  mCallEdges = new std::list<OA_ptr<Edge> >;
  mReturnEdges = new std::list<OA_ptr<Edge> >;

}


ICFG::~ICFG()
{
  mEntry = NULL;
  mExit = NULL;
}

//--------------------------------------------------------------------
// ICFG Methods
//--------------------------------------------------------------------
/*
OA_ptr<Edge> 
ICFG::getICFGEdge(
        const OA_ptr<DGraph::EdgeImplement> dgEdge) const
{
  OA_ptr<Edge> retval;

  return retval;  
}

OA_ptr<Node> 
ICFG::getICFGNode(
        const OA_ptr<DGraph::NodeImplement> dgNode) const
{
  OA_ptr<Node> retval;

  return retval;
}
*/

void ICFG::addEdge(OA_ptr<Edge> pEdge)
{
  assert(!pEdge.ptrEqual(0));

  // add associated DGraph edge to underlying DGraph
  DGraph::DGraphImplement::addEdge(pEdge);

  // add to complete list of edges

  OA_ptr<Node> pNode1, pNode2;
  switch (pEdge->getType()) {
      // a call edge
      case CALL_EDGE:
          {
        // put in set of call edges for pNode1 and pNode2
        OA_ptr<DGraph::NodeInterface> dNode1 = pEdge->getSource();
        pNode1 = dNode1.convert<Node>();
        pNode1->addCallEdge(pEdge);
        OA_ptr<DGraph::NodeInterface> dNode2 = pEdge->getSource();
        pNode2 = dNode2.convert<Node>();
        pNode2->addCallEdge(pEdge);

        // store in call edge set
        mCallEdges->push_back(pEdge);
        break;
          }
      // a return edge
      case RETURN_EDGE:
          {
        // put in set of return edges for pNode1 and pNode2
        OA_ptr<DGraph::NodeInterface> dNode1 = pEdge->getSource();
        pNode1 = dNode1.convert<Node>();
        pNode1->addReturnEdge(pEdge);
        OA_ptr<DGraph::NodeInterface> dNode2 = pEdge->getSource();
        pNode2 = dNode2.convert<Node>();
        pNode2->addReturnEdge(pEdge);

        // store in return edge set
        mReturnEdges->push_back(pEdge);
        break;
          }
      default:
        break;
  }

}

/*
void ICFG::addNode(OA_ptr<Node> pNode)
{
    //! commented out by PLM 08/22/06
    //if ( pNode->mDGNode.ptrEqual(0) ) {
    //    assert(0);
    //} else {
    //    mDGNodeToICFGNode[pNode->mDGNode] = pNode;
    //}

    //mDGraph->addNode(pNode->mDGNode);
    
    mDGraph.addNode(pNode);
    
    // add to complete list of edges

}
*/

    // from MustMayActive branch:
//void ICFG::removeEdge(OA_ptr<DGraph::EdgeInterface> pEdge)
void ICFG::removeEdge(OA_ptr<Edge> pEdge)
{
  assert(!pEdge.ptrEqual(0));

  // erase the entry for the associated DGEdge 
//  mDGEdgeToICFGEdge.erase(pEdge->mDGEdge);

  // remove associated DGraph edge to underlying DGraph
  //mDGraph->removeEdge(pEdge->mDGEdge);
  DGraph::DGraphImplement::removeEdge(pEdge);

  // add to complete list of edges
  //mEdges->remove(pEdge);

  OA_ptr<Node> pNode1, pNode2;
  OA_ptr<DGraph::NodeInterface> dpNode1, dpNode2;
  switch (pEdge->getType()) {
      // a call edge
      case CALL_EDGE:
        // remove from set of call edges for pNode1 and pNode2
        dpNode1 = pEdge->getSource();
        pNode1 = dpNode1.convert<Node>();
        pNode1->removeCallEdge(pEdge);
        dpNode2 = pEdge->getSource();
        pNode2 = dpNode2.convert<Node>();
        pNode2->removeCallEdge(pEdge);

        // remove from call edge set
        mCallEdges->remove(pEdge);
        break;

      // a return edge
      case RETURN_EDGE:
        // remove from set of return edges for pNode1 and pNode2
        dpNode1 = pEdge->getSource();
        pNode1 = dpNode1.convert<Node>();
        pNode1->removeReturnEdge(pEdge);
        dpNode2 = pEdge->getSource();
        pNode2 = dpNode2.convert<Node>();
        pNode2->removeReturnEdge(pEdge);

        // remove from return edge set
        mReturnEdges->remove(pEdge);
        break;

      default:
        break;
  }

}

/*
     //from MustMayActive branch:
void ICFG::removeNode(OA_ptr<DGraph::NodeInterface> pNode)
{
    //assert(! pNode->mDGNode.ptrEqual(0) );
    //assert(! pNode.ptrEqual(0));

    // remove the associated DGNode from map
//    mDGNodeToICFGNode.erase(pNode->mDGNode);

    //mDGraph->removeNode(pNode->mDGNode);
    //mDGraph.removeNode(pNode);
    
    // remove from complete list of edges
    //mNodes->remove(pNode);
}
*/


/*
OA_ptr<DGraph::NodesIteratorInterface> ICFG::getNodesIterator() const
{
  return mDGraph.getNodesIterator(); 
}


OA_ptr<DGraph::NodesIteratorInterface>
ICFG::getEntryNodesIterator( ) const
{
  return mDGraph.getEntryNodesIterator();
}

OA_ptr<DGraph::NodesIteratorInterface>
ICFG::getExitNodesIterator( ) const
{
   return mDGraph.getExitNodesIterator();
}

OA_ptr<DGraph::EdgesIteratorInterface> ICFG::getEdgesIterator() const
{ 
   return mDGraph.getEdgesIterator();    
}


OA_ptr<DGraph::NodesIteratorInterface>
ICFG::getReversePostDFSIterator( DGraph::DGraphEdgeDirection pOrient)
{
   return mDGraph.getReversePostDFSIterator(pOrient);
}


OA_ptr<DGraph::NodesIteratorInterface> 
      ICFG::getDFSIterator(OA_ptr<DGraph::NodeInterface> n)
{
   return mDGraph.getDFSIterator(n);
}
*/


// -----------------------------------------------------------------

OA_ptr<NodesIteratorInterface> ICFG::getICFGNodesIterator() const
{
   OA_ptr<NodesIterator> retval;
   retval = new NodesIterator(getNodesIterator());
   return retval;

}


OA_ptr<NodesIteratorInterface>
ICFG::getICFGEntryNodesIterator( ) const
{
 OA_ptr<NodesIterator> retval;
   retval = new NodesIterator(getEntryNodesIterator());
   return retval;


}

OA_ptr<NodesIteratorInterface>
ICFG::getICFGExitNodesIterator( ) const
{
 OA_ptr<NodesIterator> retval;
   retval = new NodesIterator(getExitNodesIterator());
   return retval;


}

OA_ptr<EdgesIteratorInterface> ICFG::getICFGEdgesIterator() const
{
 OA_ptr<EdgesIterator> retval;
   retval = new EdgesIterator(getEdgesIterator());
   return retval;


}


OA_ptr<NodesIteratorInterface>
ICFG::getICFGReversePostDFSIterator( DGraph::DGraphEdgeDirection pOrient)
{
 OA_ptr<NodesIterator> retval;
   retval = new NodesIterator(getReversePostDFSIterator(pOrient));
   return retval;


}


OA_ptr<NodesIteratorInterface>
      ICFG::getICFGDFSIterator(OA_ptr<NodeInterface> n)
{
 OA_ptr<NodesIterator> retval;
   retval = new NodesIterator(getDFSIterator(n));
   return retval;


}



void
ICFG::dump (ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
  os << "===== ICFG: =====\n"
     << endl;
  
  // print the contents of all the nodes
  OA_ptr<DGraph::NodesIteratorInterface> nodeIter = getNodesIterator();
  for ( ; nodeIter->isValid(); ++(*nodeIter)) {
    OA_ptr<DGraph::NodeInterface> dnode = nodeIter->current();
    OA_ptr<Node> node = dnode.convert<Node>();
    node->longdump(*this,os, ir);
    os << endl;
  }
  
  os << "====================" << endl;

}

void
ICFG::dumpdot(ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
  os << "digraph OA_ICFG {" << endl;
  os << "node [shape=rectangle];" << endl;

  // print the contents of all the nodes 
  // first sort group nodes by procedure
  std::map<ProcHandle,std::set<OA_ptr<Node> > > procNodeSet;
  OA_ptr<DGraph::NodesIteratorInterface> nodesIterPtr = getNodesIterator();
  for ( ; nodesIterPtr->isValid(); ++(*nodesIterPtr) ) {
    OA_ptr<DGraph::NodeInterface> dnode = nodesIterPtr->current();
    OA_ptr<Node> node = dnode.convert<Node>();
    ProcHandle proc = node->getProc();
    procNodeSet[proc].insert(node);
  }

  // sort edges by procedure, call and return edges will be put in 
  // a separate set
  std::set<OA_ptr<Edge> > sepSet;
  std::map<ProcHandle,std::set<OA_ptr<Edge> > > procEdgeSet;
  OA_ptr<DGraph::EdgesIteratorInterface> edgeIterPtr = getEdgesIterator();
  for ( ; edgeIterPtr->isValid(); ++(*edgeIterPtr) ) {
    OA_ptr<DGraph::EdgeInterface> dedge = edgeIterPtr->current();
    OA_ptr<Edge> edge = dedge.convert<Edge>();
    if (edge->getSinkProc() == edge->getSourceProc()) {
        procEdgeSet[edge->getSinkProc()].insert(edge);
    } else {
        sepSet.insert(edge);
    }
  }

  int procNum = 0; // for labeling each proc uniquely
  // then output nodes and edges by procedure
  std::map<ProcHandle,std::set<OA_ptr<Node> > >::iterator mapIter;
  for (mapIter=procNodeSet.begin(); mapIter!=procNodeSet.end(); mapIter++ ) {
    // subgraph
    std::ostringstream label;
    label << ir->toString(mapIter->first) << "_" << procNum++;
    os << "subgraph cluster_" << label.str() << " {" << std::endl;
    os << "    label=\"" << label.str() << "\"" << std::endl;
      
    // set of nodes
    std::set<OA_ptr<Node> > nodeSet = mapIter->second;
    std::set<OA_ptr<Node> >::iterator nodeIter;
    for (nodeIter=nodeSet.begin(); nodeIter!=nodeSet.end(); nodeIter++ ) {
      (*nodeIter)->dumpdot(*this,os,ir);
    }
    
    // set of edges
    std::set<OA_ptr<Edge> > edgeSet = procEdgeSet[mapIter->first];
    std::set<OA_ptr<Edge> >::iterator edgeIter;
    for (edgeIter=edgeSet.begin(); edgeIter!=edgeSet.end(); edgeIter++ ) {
      (*edgeIter)->dumpdot(os);
    }
    os << "}" << endl;
  }

  // print rest of edges
  std::set<OA_ptr<Edge> >::iterator edgeIter;
  for (edgeIter=sepSet.begin(); edgeIter!=sepSet.end(); edgeIter++ ) {
      (*edgeIter)->dumpdot(os);
  }
  
  os << "}" << endl;
  os.flush();
}

void ICFG::output(OA::IRHandlesIRInterface& ir) {

  // output the contents of the ICFG
  // first sort group nodes by procedure
  std::map<ProcHandle,std::set<OA_ptr<Node> > > procNodeSet;
  OA_ptr<DGraph::NodesIteratorInterface> nodesIterPtr = getNodesIterator();
  for ( ; nodesIterPtr->isValid(); ++(*nodesIterPtr) ) {
    OA_ptr<DGraph::NodeInterface> dnode = nodesIterPtr->current();
    OA_ptr<Node> node = dnode.convert<Node>();
    ProcHandle proc = node->getProc();
    procNodeSet[proc].insert(node);
  }

  // then sort edges by procedure:
  // call and return edges will be put in a separate set
  std::set<OA_ptr<Edge> > sepSet;
  std::map<ProcHandle,std::set<OA_ptr<Edge> > > procEdgeSet;
  OA_ptr<DGraph::EdgesIteratorInterface> edgeIterPtr = getEdgesIterator();
  for ( ; edgeIterPtr->isValid(); ++(*edgeIterPtr) ) {
    OA_ptr<DGraph::EdgeInterface> dedge = edgeIterPtr->current();
    OA_ptr<Edge> edge = dedge.convert<Edge>();
    if (edge->getSinkProc() == edge->getSourceProc()) {
        procEdgeSet[edge->getSinkProc()].insert(edge);
    } else {
        sepSet.insert(edge);
    }
  }

  

  int procNum = 0; // numbering subgraphs in case we have duplicate names
  sOutBuild->graphStart("ICFG");

  // then output nodes and edges by procedure
  std::map<ProcHandle,std::set<OA_ptr<Node> > >::iterator mapIter;
  for (mapIter=procNodeSet.begin(); mapIter!=procNodeSet.end(); mapIter++ ) {

    // subgraph
    std::ostringstream label;
    label << ir.toString(mapIter->first) << "_" << procNum++;

    sOutBuild->graphSubStart( label.str() );
    
    // set of nodes
    std::set<OA_ptr<Node> > nodeSet = mapIter->second;
    std::set<OA_ptr<Node> >::iterator nodeIter;
    for (nodeIter=nodeSet.begin(); nodeIter!=nodeSet.end(); nodeIter++ ) {
      OA_ptr<Node> node = (*nodeIter);
      sOutBuild->graphNodeStart(node->getId());
         sOutBuild->graphNodeLabelStart();
            node->output(ir);
         sOutBuild->graphNodeLabelEnd();
      sOutBuild->graphNodeEnd();
    }
    
    // set of edges
    std::set<OA_ptr<Edge> > edgeSet = procEdgeSet[mapIter->first];
    std::set<OA_ptr<Edge> >::iterator edgeIter;
    for (edgeIter=edgeSet.begin(); edgeIter!=edgeSet.end(); edgeIter++ ) {
      OA_ptr<Edge> edge = (*edgeIter);
      OA_ptr<DGraph::NodeInterface> dsourceNode = edge->getSource();
      OA_ptr<Node> sourceNode = dsourceNode.convert<Node>();
      OA_ptr<DGraph::NodeInterface> dsinkNode   = edge->getSink();
      OA_ptr<Node> sinkNode = dsinkNode.convert<Node>();
      
      sOutBuild->graphEdgeStart();
         sOutBuild->graphEdgeSourceNode(sourceNode->getId());
         sOutBuild->graphEdgeSinkNode(sinkNode->getId());
         sOutBuild->graphEdgeLabelStart();
            edge->output(ir);
         sOutBuild->graphEdgeLabelEnd();
         std::ostringstream extras;
         switch(edge->mType) {
         case (CALL_EDGE):
           extras << ",style=dashed,color=red";
           break;
         case (RETURN_EDGE):
           extras << ",style=dashed,color=blue";
           break;
         case (CFLOW_EDGE):
           break;
         }
         sOutBuild->outputString( extras.str() );
      sOutBuild->graphEdgeEnd();
    }

    sOutBuild->graphSubEnd( label.str() );
  }

  // print rest of edges
  std::set<OA_ptr<Edge> >::iterator edgeIter;
  for (edgeIter=sepSet.begin(); edgeIter!=sepSet.end(); edgeIter++ ) {
      OA_ptr<Edge> edge = (*edgeIter);
      OA_ptr<DGraph::NodeInterface> dsourceNode = edge->getSource();
      OA_ptr<Node> sourceNode = dsourceNode.convert<Node>();
      OA_ptr<DGraph::NodeInterface> dsinkNode   = edge->getSink();
      OA_ptr<Node> sinkNode = dsinkNode.convert<Node>();
      
      sOutBuild->graphEdgeStart();
         sOutBuild->graphEdgeSourceNode(sourceNode->getId());
         sOutBuild->graphEdgeSinkNode(sinkNode->getId());
         sOutBuild->graphEdgeLabelStart();
            edge->output(ir);
         sOutBuild->graphEdgeLabelEnd();
         std::ostringstream extras;
         switch(edge->mType) {
         case (CALL_EDGE):
           extras << ",style=dashed,color=red";
           break;
         case (RETURN_EDGE):
           extras << ",style=dashed,color=blue";
           break;
         case (CFLOW_EDGE):
           break;
         }
         sOutBuild->outputString( extras.str() );
      sOutBuild->graphEdgeEnd();
  }

//      mDGraph->output(ir);
  
  sOutBuild->graphEnd("ICFG");
}

/*
void ICFG::addNode(OA_ptr<DGraph::NodeInterface> n)
{
   mDGraph.addNode(n); 
}

void ICFG::addEdge(OA_ptr<DGraph::EdgeInterface> e)
{

   mDGraph.addEdge(e);
}
*/


  } // end namespace ICFG
} // end namespace OA

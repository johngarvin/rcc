/*! \file
  
  \brief Implementation of CFG.

  Implements the CFG::CFGInterface for viewing the contents and structure
  of a CFG.

  Also implements methods useful for constructing a CFG including methods
  that access a mapping of labels (OA::StmtLabel's) to Nodes.

  \authors Arun Chauhan (2001 as part of Mint), Nathan Tallent, Michelle Strout
  \version $Id: CFGStandard.cpp,v 1.25 2005/06/10 02:32:03 mstrout Exp $
  \version $Id: CFG.cpp,  2006/08/27 02:32:03 PLM Exp $ 

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/


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
#include "CFG.hpp"

#include <OpenAnalysis/Utils/OutputBuilder.hpp>
namespace OA {
  namespace CFG {

//--------------------------------------------------------------------
// must be updated any time CFG::EdgeType changes
static const char *sEdgeTypeToString[] = { 
  "TRUE_EDGE",
  "FALLTHROUGH_EDGE",
  "FALSE_EDGE",
  "BACK_EDGE",
  "MULTIWAY_EDGE",
  "BREAK_EDGE",
  "CONTINUE_EDGE",
  "RETURN_EDGE",
  "NO_EDGE"
};
    /*
// must be updated any time CFG::EdgeType changes
const char *Edge::sEdgeTypeToString[] = {
  "TRUE_EDGE", "FALLTHROUGH_EDGE", "FALSE_EDGE",
  "BACK_EDGE", "MULTIWAY_EDGE", "BREAK_EDGE",
  "CONTINUE_EDGE", "RETURN_EDGE"
};
    */
    std::string edgeTypeToString(EdgeType et)
{
  std::string str = sEdgeTypeToString[et];
  //    return sEdgeTypeToString[et];
  return str;
}


//--------------------------------------------------------------------
NodesIterator::NodesIterator(OA_ptr<DGraph::NodesIteratorInterface> ni)
            : DGraph::NodesIteratorImplement(ni) {
}



CFG::CFG()
{
}

OA_ptr<NodeInterface> NodesIterator::currentCFGNode() const
{
    return current().convert<Node>();
}

OA_ptr<EdgeInterface> EdgesIterator::currentCFGEdge() const
{
    return current().convert<Edge>();
}

Node::~Node() 
{
    mStmt_list->clear(); 
}

/*
unsigned int Node::getId() const 
{
    return mdgNode.getId();
 }
 
int Node::num_incoming () const 
{ 
    return mdgNode.num_incoming(); 
}

int Node::num_outgoing () const 
{
    return mdgNode.num_outgoing(); 
}

bool Node::isAnEntry() const 
{ 
    return mdgNode.isAnEntry(); 
}

bool Node::isAnExit() const 
{ 
    return mdgNode.isAnExit(); 
}

bool Node::operator== (DGraph::NodeInterface& other)
{
    return mdgNode.operator==(other);
}

bool Node::operator< (DGraph::NodeInterface& other)
{
    return mdgNode.operator<(other);
}

void Node::addIncomingEdge(OA_ptr<DGraph::EdgeInterface> e)
{
    mdgNode.addIncomingEdge(e);
}

void Node::addOutgoingEdge(OA_ptr<DGraph::EdgeInterface> e)
{
     mdgNode.addOutgoingEdge(e);
}

void Node::removeIncomingEdge(OA_ptr<DGraph::EdgeInterface> e)
{
    mdgNode.removeIncomingEdge(e);
}

void Node::removeOutgoingEdge(OA_ptr<DGraph::EdgeInterface> e)
{
     mdgNode.removeOutgoingEdge(e);
}
*/

unsigned int Node::size() const 
{ 
     return mStmt_list->size(); 
}

void Node::add(StmtHandle h) 
{
     mStmt_list->push_back(h); 
}

void Node::add_front(StmtHandle h) 
{
     mStmt_list->push_front(h); 
}

/*!
    \brief  Remove 'h' from the statement list; return the removed handle or
            0 if not found.
*/
StmtHandle Node::erase(StmtHandle h)
{
  std::list<StmtHandle>::iterator it;
  for (it = mStmt_list->begin(); (it != mStmt_list->end()); ++it) {
    StmtHandle stmt = *it;
    if (stmt == h) {
      mStmt_list->erase(it);
      return h;
    }
  }
  return 0;
}


bool Node::empty() const 
{ 
  return mStmt_list->empty(); 
}


OA_ptr<NodeStatementsIteratorInterface>
          Node::getNodeStatementsIterator() const
{
  OA_ptr<NodeStatementsIterator> retval;
  retval = new NodeStatementsIterator(*this);
  return retval;
}



OA_ptr<NodeStatementsRevIteratorInterface>
        Node::getNodeStatementsRevIterator()  const
{
  OA_ptr<NodeStatementsRevIterator> retval;
  retval = new NodeStatementsRevIterator(*this);
  return retval;
}

/*
OA_ptr<DGraph::EdgesIteratorInterface> Node::getIncomingEdgesIterator() const
{
  return mdgNode.getIncomingEdgesIterator();
}

OA_ptr<DGraph::EdgesIteratorInterface> Node::getOutgoingEdgesIterator() const
{
  return mdgNode.getOutgoingEdgesIterator();
}

OA_ptr<DGraph::NodesIteratorInterface> Node::getSourceNodesIterator() const
{
  return mdgNode.getSourceNodesIterator();
}

OA_ptr<DGraph::NodesIteratorInterface> Node::getPredNodesIterator() const
{
  return getSourceNodesIterator(); 
}

OA_ptr<DGraph::NodesIteratorInterface> Node::getSinkNodesIterator() const
{
  return mdgNode.getSinkNodesIterator();
}

OA_ptr<DGraph::NodesIteratorInterface> Node::getSuccNodesIterator() const
{
  return getSinkNodesIterator();
}
*/

OA_ptr<EdgesIteratorInterface> Node::getCFGIncomingEdgesIterator() const
{
   OA_ptr<EdgesIterator> retval;
   retval = new EdgesIterator(getIncomingEdgesIterator());
   return retval;

}

OA_ptr<EdgesIteratorInterface> Node::getCFGOutgoingEdgesIterator() const
{
   OA_ptr<EdgesIterator> retval;
   retval = new EdgesIterator(getOutgoingEdgesIterator());
   return retval;

}

OA_ptr<NodesIteratorInterface> Node::getCFGSourceNodesIterator() const
{
   OA_ptr<NodesIterator> retval;
   retval = new NodesIterator(getSourceNodesIterator());
   return retval;

}

OA_ptr<NodesIteratorInterface> Node::getCFGSinkNodesIterator() const
{
   OA_ptr<NodesIterator> retval;
   retval = new NodesIterator(getSinkNodesIterator());
   return retval;

}


OA_ptr<NodesIteratorInterface> Node::getCFGPredNodesIterator() const
{
   OA_ptr<NodesIterator> retval;
   retval = new NodesIterator(getSourceNodesIterator());
   return retval;

}

OA_ptr<NodesIteratorInterface> Node::getCFGSuccNodesIterator() const
{
   OA_ptr<NodesIterator> retval;
   retval = new NodesIterator(getSinkNodesIterator());
   return retval;

}


Node::Node() { Ctor(); }

Node::Node(StmtHandle n)
            {  Ctor(); mStmt_list->push_back(n);  }


void Node::dump(std::ostream& os) {
    /*
      os << "CFG Node = < ";
      mdgNode.dump(os);
      // can't print statements here without IR
      os << " >";
      */
    } // override the dump from DGraph::DGraphImplement



void Node::Ctor() 
{

    mStmt_list = new std::list<StmtHandle>; 
}


std::list<StmtHandle>::iterator Node::getStmtListBegin () 
{
      return mStmt_list->begin();
}

std::list<StmtHandle>::iterator Node::getStmtListEnd () 
{
      return mStmt_list->end();
}

std::list<StmtHandle>::reverse_iterator Node::getStmtListRBegin () 
{
      return mStmt_list->rbegin();
}

std::list<StmtHandle>::reverse_iterator Node::getStmtListREnd () 
{
      return mStmt_list->rend();
}

/*! Edge */

Edge::Edge(OA_ptr<DGraph::NodeInterface> source,
             OA_ptr<DGraph::NodeInterface> sink, EdgeType _type,
             OA::ExprHandle _expr) : DGraph::EdgeImplement (source, sink), mType(_type), mExpr(_expr)
{
}


Edge::~Edge() 
{
}

/*
OA_ptr<DGraph::NodeInterface> Edge::getSource() const
{
      return mdgEdge.getSource();
}

OA_ptr<DGraph::NodeInterface> Edge::getSink() const
{
      return mdgEdge.getSink();
}
*/

OA_ptr<NodeInterface> Edge::getCFGSource() const
{
      return getSource().convert<NodeInterface>();
}

OA_ptr<NodeInterface> Edge::getCFGSink() const
{
      return getSink().convert<NodeInterface>();
}

/*
bool Edge::operator== (DGraph::EdgeInterface& other)
{
  return mdgEdge.operator==(other);
}

bool Edge::operator< (DGraph::EdgeInterface& other)
{
  return mdgEdge.operator<(other);
}

unsigned int Edge::getId() const 
{
  return mdgEdge.getId(); 
}
*/

EdgeType Edge::getType() const 
{
      return mType; 
}

ExprHandle Edge::getExpr() const 
{ 
      return mExpr; 
}

/*
void Edge::setId(unsigned int id) 
{
}
*/

void Edge::setType(EdgeType type) 
{ 
  mType = type; 
}

void Edge::setExpr(ExprHandle expr) 
{
  mExpr = expr; 
}

/*! CFG */
/*
int CFG::getNumNodes() { return mDG.getNumNodes(); }

int CFG::getNumEdges() { return igned int Node::getId() const {
           return mdgNode.getId();
                   }
                   DG.getNumEdges(); }
*/

OA_ptr<NodeInterface> CFG::getEntry() const 
{
   return mEntry; 
}

OA_ptr<NodeInterface> CFG::getExit() const
{
   return mExit; 
}

SymHandle CFG::getName() const 
{ 
   return mName; 
}


/*
OA_ptr<DGraph::NodesIteratorInterface> CFG::CFG::getNodesIterator() const
{
   return mDG.getNodesIterator();
}

OA_ptr<DGraph::EdgesIteratorInterface> CFG::CFG::getEdgesIterator() const
{
   return mDG.getEdgesIterator();
}

OA_ptr<DGraph::NodesIteratorInterface> CFG::CFG::getEntryNodesIterator() const
{
   return mDG.getEntryNodesIterator();
}

OA_ptr<DGraph::NodesIteratorInterface> CFG::CFG::getExitNodesIterator() const
{
   return mDG.getExitNodesIterator();
}


OA_ptr<DGraph::NodesIteratorInterface>
    CFG::getReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient) 
{
   return mDG.getReversePostDFSIterator(pOrient);
}


OA_ptr<DGraph::NodesIteratorInterface> CFG::getDFSIterator(OA_ptr<DGraph::NodeInterface> n) 
{
   return mDG.getDFSIterator(n);
}
*/


OA_ptr<NodesIteratorInterface> CFG::getCFGNodesIterator() const
{
   OA_ptr<NodesIterator> retval;
   retval = new NodesIterator(getNodesIterator());
   return retval;
}

// method to get a more specific iterator
OA_ptr<EdgesIteratorInterface> CFG::getCFGEdgesIterator() const
{
   OA_ptr<EdgesIterator> retval;
   retval = new EdgesIterator(getEdgesIterator());
   return retval;
}


OA_ptr<NodesIteratorInterface> CFG::getCFGEntryNodesIterator() const
{
   OA_ptr<NodesIterator> retval;
   retval = new NodesIterator(getEntryNodesIterator());
   return retval;
}

OA_ptr<NodesIteratorInterface> CFG::getCFGExitNodesIterator() const
{
   OA_ptr<NodesIterator> retval;
   retval = new NodesIterator(getExitNodesIterator());
   return retval;
}

OA_ptr<NodesIteratorInterface>
    CFG::getCFGReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient)
{
   OA_ptr<NodesIterator> retval;
   retval = new NodesIterator(getReversePostDFSIterator(pOrient));
   return retval;
}


OA_ptr<NodesIteratorInterface> CFG::getCFGDFSIterator(OA_ptr<NodeInterface> n)
{
   OA_ptr<NodesIterator> retval;
   retval = new NodesIterator(getDFSIterator(n));
   return retval;
}


/*
void CFG::addNode(OA_ptr<DGraph::NodeInterface> n)
{
  mDG.addNode(n);
}

void CFG::addEdge(OA_ptr<DGraph::EdgeInterface> e)
{
  
   mDG.addEdge(e);
}
*/
/*
void CFG::output(IRHandlesIRInterface& ir) const
{
}
*/


void CFG::setEntry(OA_ptr<Node> n) 
{ 
   mEntry = n; 
}

void CFG::setExit(OA_ptr<Node> n) 
{ 
   mExit = n; 
}


OA_ptr<Edge> CFG::connect(OA_ptr<NodeInterface> src,
                       OA_ptr<NodeInterface> dst,
                       EdgeType type) 
{
      OA_ptr<Edge> e;
      e = new Edge (src, dst, type, 0);
      addEdge (e);
      return e;
}

OA_ptr<Edge> CFG::connect(OA_ptr<NodeInterface> src,
                       OA_ptr<NodeInterface> dst,
               EdgeType type, ExprHandle expr)
{
      OA_ptr<Edge> e;
      e = new Edge (src, dst, type, expr);
      addEdge (e);
      return e;
}

void CFG::disconnect(OA_ptr<EdgeInterface> e)
{
      removeEdge(e); 
}

void CFG::disconnect(OA_ptr<NodeInterface> n)
{
      removeNode(n); 
}

void CFG::mapLabelToNode(OA::StmtLabel lab, OA_ptr<Node> n)
{
      mlabel_to_node_map[lab] = n; 
}


//! determine if the given label is currently mapped to a node
bool CFG::isLabelMappedToNode(StmtLabel lab)
{
      return (mlabel_to_node_map.find(lab) != mlabel_to_node_map.end()) ? true: false; 
}

void NodeStatementsIterator::operator++() 
{
      if (mIter != mList->end()) 
      { 
          mIter++; 
      }
}

void NodeStatementsIterator::operator++(int) 
{
     ++*this; 
}  // postfix


bool NodeStatementsIterator::isValid() const
{
    return (mIter != mList->end()); 
}


StmtHandle NodeStatementsIterator::current() const 
{ 
    return *mIter; 
}

void NodeStatementsIterator::reset() 
{
    mIter = mList->begin(); 
}

void NodeStatementsRevIterator::operator++() 
{ 
    if (mRevIter != mList->rend()) 
        ++mRevIter; 
}

void NodeStatementsRevIterator::operator++(int) 
{ 
    ++*this; 
}  // postfix


bool NodeStatementsRevIterator::isValid() const 
{
   return (mRevIter != mList->rend()); 
}


StmtHandle NodeStatementsRevIterator::current() const 
{
   return *mRevIter; 
}


void NodeStatementsRevIterator::reset() 
{
   mRevIter = mList->rbegin(); 
}

OA_ptr<Node> CFG::NodeLabel::getNode() 
{ 
   return mN; 
}

EdgeType CFG::NodeLabel::getEdgeType()
{ 
   return mEt; 
}

OA::ExprHandle CFG::NodeLabel::getExpr()
{
   return mEh; 
}

void CFG::NodeLabelListIterator::operator++() 
{ 
   if (mIter != mList.end()) ++mIter;
}

void CFG::NodeLabelListIterator::operator++(int) 
{
   ++*this; 
}

bool CFG::NodeLabelListIterator::isValid() const 
{ 
   return (mIter != mList.end()); 
}

CFG::NodeLabel CFG::CFG::NodeLabelListIterator::current() const 
{
   return *mIter; 
}


//--------------------------------------------------------------------

//--------------------------------------------------------------------
void Edge::dump (ostream& os)
{
    /*! commented out by PLM 08/17/06
  os << "{ " << mId << " " 
     << edgeTypeToString(mType) << " " 
     << mExpr << " }";
  os.flush();
  */
}

void Edge::output(IRHandlesIRInterface& ir) const
{
    std::ostringstream label;
    label << "{ " 
       << edgeTypeToString(mType) << " " 
       << mExpr << " }";
    sOutBuild->outputString( label.str() );
}

//--------------------------------------------------------------------
/*! commented out by PLM 08/08/06
CFG::CFG (SymHandle _name) : mName(_name)
*/
/*!
 * \brief Constructs an empty standard CFGStandard
 *
 * \param _name    SymbolHandle for procedure name
 */
/*{ 
}
*/

CFG::~CFG()
/*!
 * \brief Destructor for CFGStandard
 */
{
  // FIXME: should iterate through all nodes and edges to delete them
  
  mEntry = NULL;
  mExit = NULL;
  mlabel_to_node_map.clear();
}



//--------------------------------------------------------------------
//--------------------------------------------------------------------
/*1 commented out by PLM 08/07/06
 * CFG::Edge::Edge (OA_ptr<NodeInterface> n1, 
                         OA_ptr<NodeInterface> n2, 
                         EdgeType _type, ExprHandle _expr) 
    : DGraph::EdgeInterface(n1.convert<Node>(), 
                                   n2.convert<Node>()), 
      mType(_type), mExpr(_expr)
      */
/*!
 * \brief Edge constructor
 *
 */
/*{
  mId = sNextId++;
}
*/

   
//--------------------------------------------------------------------

//--------------------------------------------------------------------
/*! 
   \brief Given a label, determine whether it has been encountered already.
   If so, return its associated block (obtained from the mlabel_to_node_map).
   If not, return 0
*/
OA_ptr<Node> CFG::getLabelBlock(OA::StmtLabel lab)
{
  OA_ptr<Node> retval; retval = 0;

  if (mlabel_to_node_map.find(lab) != mlabel_to_node_map.end()) {
    retval = mlabel_to_node_map[lab];
  }
  return retval;
}

//--------------------------------------------------------------------
/*! 
   \brief Given a label, determine whether it has been encountered already.
   If so, return its associated block (obtained from the mlabel_to_node_map).
   If not, create a new block and update the mlabel_to_node_map.
*/ 
OA_ptr<Node> CFG::node_from_label (OA::StmtLabel lab)
{
  OA_ptr<Node> node; node = 0;
  if (mlabel_to_node_map.find(lab) != mlabel_to_node_map.end()) {
    // This label has been encountered already, thus it already has an associated block.
    node = mlabel_to_node_map[lab];
  }
  else {
    // This label has not been encountered yet, so create a new block.
    node = new Node();
    addNode(node);
    mlabel_to_node_map[lab] = node; 
  }
  return node;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
/*!
    \brief Transfer all statements beginning at (and including) splitPoint to 
           the new basic block, 'newBlock'
*/
void Node::split(OA::StmtHandle splitPoint, OA_ptr<Node> newBlock)
{
  std::list<StmtHandle>::iterator splitPointIter;
  std::list<StmtHandle> tempList;
  std::list<StmtHandle>::iterator tempIter;
  
  for (splitPointIter = mStmt_list->begin();
       (splitPointIter != mStmt_list->end() && *splitPointIter != splitPoint);
       ++splitPointIter) {
  }
  // 'splitPointIter' now points to 'splitPoint'

  // splice them into temp list
  tempList.splice(tempList.end(), *mStmt_list, splitPointIter, mStmt_list->end());

  // then copy them into newBlock, can't splice to newBlock, because can't
  // assume we now how newBlock stores data
  for (tempIter=tempList.begin(); tempIter != tempList.end(); tempIter++) {
      newBlock->add(*tempIter);
  }
}


/*!
   \brief Split the basic block 'block' at the given split point and return
          the new basic block.  
          
   \param splitPoint The first statement in the new basic block.  
   
   After the operation is done, the new block
   contains all the outgoing edges from 'block'; in other words,
  'block' will have *no* outgoing edges.
*/
OA_ptr<Node>
CFG::splitBlock (OA_ptr<Node> block, StmtHandle splitPoint)
{
  // TODO: if a NOP is in the middle of being processed split into 
  // 2 NOPs so they can be split. ???

  OA_ptr<Node> newBlock; newBlock = new Node;
  addNode(newBlock);

  // 1. Move all instructions beginning at 'splitPoint' from 'block'
  // to the new block
  block->split((StmtHandle)splitPoint, newBlock);

  // 2. Change edge information
  std::list<OA_ptr<EdgeInterface> > del_list;
  
  // 2a. Outgoing edges of 'block' must become outgoing edges of 'newBlock'
  for (OA_ptr<EdgesIteratorInterface> ei = block->getCFGOutgoingEdgesIterator();
       ei->isValid(); ++(*ei))
  {
      // Add successor edges for new block
      OA_ptr<EdgeInterface> edge 
          = ei->currentCFGEdge();
      OA_ptr<NodeInterface> targBlock 
          = edge->getCFGSink();
      
      connect(newBlock, targBlock, edge->getType(), edge->getExpr());
      del_list.push_back(edge);

      // 2b. Update fallthrough, mFallThrough is datastructure used
      // by ManagerStandard, no longer used here, MMS 4/7/04
  }
  
  // 2c. Remove outgoing edges of 'block'
  std::list<OA_ptr<EdgeInterface> >::iterator dli = del_list.begin();
  for ( ; dli != del_list.end(); dli++) {
    disconnect(*dli);
  }

#if 0 // FIXME: Splitting won't change any label's block??
  // Update label->block maps.
  for (NodeStatementsIterator si(newBlock); (StmtHandle)si; ++si) {
    StmtHandle stmt = (StmtHandle)si;
    if (ir->GetLabel(stmt) != 0) {
      label_to_node_map.erase(ir->GetLabel(stmt));
      label_to_node_map[ir->GetLabel(stmt)] = newBlock;
    }
  }
#endif
  
  return newBlock;
}

//--------------------------------------------------------------------
//! Connect the src to each of the nodes in dst_list.
void
CFG::connect (OA_ptr<NodeInterface> src, 
                      CFG::NodeLabelList& dst_list)
{
    CFG::NodeLabelListIterator dst_list_iter(dst_list);
    while (dst_list_iter.isValid()) {
        NodeLabel nl = dst_list_iter.current();
        connect(src, nl.getNode(), nl.getEdgeType(), nl.getExpr());
        ++dst_list_iter;
    }
}


//! Connect each node in src_list to dst.
void
CFG::connect (CFG::NodeLabelList& src_list, 
                      OA_ptr<NodeInterface> dst)
{
  NodeLabelListIterator src_list_iter(src_list);
  while (src_list_iter.isValid()) {
    NodeLabel nl = src_list_iter.current();
    connect(nl.getNode(), dst, nl.getEdgeType(), nl.getExpr());
    ++src_list_iter;
  }
}

//! Nathan's
void
CFG::dump (ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
  /*! commented out by PLM 08/07/06  
  os << "========== CFG dump ==========" << endl 
     << ir->toString(mName) << endl 
     << "------------------------------" << endl;

  // print the contents of all the nodes
  OA_ptr<DGraph::NodesIteratorInterface> nodes_iter = getNodesIterator();
  for ( ; nodes_iter->isValid(); ++(*nodes_iter)) {
    OA_ptr<DGraph::NodeInterface> node = nodes_iter->current();
    // node->dump(std::cout, ir);
//    node->longdump(*this, os, ir);
    os << endl;
  }
  
  os << "==============================" << endl;
  dumpdot(os,ir);
  os << "==============================" << endl;
  */
}

void
Node::dump (ostream& os, OA_ptr<IRHandlesIRInterface> rir)
{
    /*! commented out y PLM 08/17/06
  // print the node ID
  os << "CFG Node: " << getId();
  if (num_incoming() == 0)
    os << " (root)";
  os << endl;

  // print the node contents
  CFG::NodeStatementsIterator stmtIt(*this);
  for ( ; stmtIt.isValid(); ++stmtIt) {
    os << "    ";
    StmtHandle s = stmtIt.current();
    rir->dump(s, os);
    os << endl;
  }
  // print the sink(s)
  OA_ptr<DGraph::EdgesIteratorInterface> out_iter = getOutgoingEdgesIterator();
  if (out_iter->isValid()) {
    OA_ptr<CFG::Edge> e = out_iter->current();
    
    OA_ptr<DGraph::EdgeInterface> de = out_iter->current();
    OA_ptr<CFG::Edge> e = de.convert<CFG::Edge>();
    os << " --> (" << (e->sink())->getId();
    os << " [";
    e->dump(os);
    os << "]";
    ++(*out_iter);
    for ( ; out_iter->isValid(); ++(*out_iter)) {
      e = out_iter->current();
      OA_ptr<DGraph::EdgeInterface> de = out_iter->current();
      OA_ptr<CFG::Edge> e = de.convert<CFG::Edge>();
      os << ", " << (e->sink())->getId();
      os << " [";
      e->dump(os);
      os << "]";
    }
    os << ")" << endl;
  }
  */
}

/*
void
Node::longdump (CFG& currcfg, ostream& os, 
                             OA_ptr<IRHandlesIRInterface> rir)
{*/
    /*! commented out by PLM 08/17/06
  // print the node ID
  os << "CFG Node: " << getId();
  if (num_incoming() == 0)
    os << " (root)";
  if (currcfg.getEntry().ptrEqual(this)) {
    os << " [ENTRY]";
  } 
  else if (currcfg.getExit().ptrEqual(this)) {
    os << " [EXIT]"; 
  }
  os << endl;
  
  // print the node contents
  CFG::NodeStatementsIterator stmtIt(*this);
  for ( ; stmtIt.isValid(); ++stmtIt) {
    os << "    ";
    StmtHandle s = stmtIt.current();
    rir->dump(s, os);
    os << endl;
  }
  
  // print the sources(s)
  OA_ptr<DGraph::EdgesIteratorInterface> in_iter = getIncomingEdgesIterator();
  if (in_iter->isValid()) {
    OA_ptr<Edge> e = in_iter->current();
    os << " <-- (" << (e->source())->getId();
    os << " [";
    e->dump(os);
    os << "]";
    ++(*in_iter);
    for ( ; in_iter->isValid(); ++(*in_iter)) {
      e = in_iter->current();
      os << ", " << (e->source())->getId();
      os << " [";
      e->dump(os);
      os << "]";
    }
    os << ")" << endl;
  }
  
  // print the sink(s)
  OA_ptr<DGraph::EdgesIterator> out_iter = getOutgoingEdgesIterator();
  if (out_iter->isValid()) {
    OA_ptr<Edge> e = out_iter->current();
    os << " --> (" << (e->sink())->getId();
    os << " [";
    e->dump(os);
    os << "]";
    ++(*out_iter);
    for ( ; out_iter->isValid(); ++(*out_iter)) {
      e = out_iter->current();
      os << ", " << (e->sink())->getId();
      os << " [";
      e->dump(os);
      os << "]";
    }
    os << ")" << endl;
  }
  */
//}

/*void
dumpdot(ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{*/
    /*!commented out by PLM 08/17/06
  //const char* nm = ir.GetSymNameFromSymHandle(name);
  os << "digraph OA_CFG_" << ir->toString(mName) << " {" << endl;
  os << "node [shape=rectangle];" << endl;

  // print the contents of all the nodes (including edges)
  OA_ptr<NodesIterator> nodesIterPtr = getNodesIterator();
  for ( ; nodesIterPtr->isValid(); ++(*nodesIterPtr) ) {
    nodesIterPtr->current()->dumpdot(*this,os,ir);
  }

  // print all edges
  OA_ptr<EdgesIterator> edgesIterPtr = getEdgesIterator();
  for ( ; edgesIterPtr->isValid(); ++(*edgesIterPtr) ) {
    edgesIterPtr->current()->dumpdot(os,ir);
  }
  
  os << "}" << endl;
  os.flush();
  */
//}

/*
void
dumpdot (CFG &currcfg, ostream& os, 
                            OA_ptr<IRHandlesIRInterface> ir)
{*/
    /*! commented out by PLM 08/17/06
  // print the node
  os << getId() << " [ label=\"====== CFG node " << getId() << " ======";
  
  if (currcfg.getEntry().ptrEqual(this)) {
    os << " (entry)";
  } 
  else if (currcfg.getExit().ptrEqual(this)) {
    os << " (exit)"; 
  }
  os << "\\n";
  // print the node contents
  CFG::NodeStatementsIterator stmtIt(*this);
  for ( ; stmtIt.isValid(); ++stmtIt) {
    StmtHandle s = stmtIt.current();
    os << std::endl << ir->toString(s) << "\\n";
  }
  os << "\" ];" << endl;
  os.flush();
  */
/*}
*/

void Node::output(OA::IRHandlesIRInterface& ir) const
{
    std::ostringstream label;
    
    label <<  "====== CFG node " << getId() << " ======\n";
    
    
    // print the node contents
    NodeStatementsIterator stmtIt(*this);
    for ( ; stmtIt.isValid(); ++stmtIt) {
        StmtHandle s = stmtIt.current();
        label << ir.toString(s) + "\n";
    }
    
    sOutBuild->outputString(label.str());
}

void Edge::dumpdot(ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
  /*! commented out by PLM 08/17/06
  os << source()->getId() << " -> " << sink()->getId() << ";" << endl;
  os.flush();
  */
}



  } // end of CFG namespace
} // end of OA namespace

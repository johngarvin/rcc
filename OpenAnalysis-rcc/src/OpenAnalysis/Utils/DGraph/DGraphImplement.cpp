

#include "DGraphImplement.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

static bool debug = false;

namespace OA {
    namespace DGraph {

//========================================================
// DGraphInterface implementation
//========================================================


unsigned int DGraph::NodeImplement::sNextId = 1;
unsigned int DGraph::EdgeImplement::sNextId = 1;
 
NodeImplement::NodeImplement()
{
    OA_DEBUG_CTRL_MACRO("DEBUG_DGraphImplement:ALL", debug);
    if(debug)
    {
        std::cout << "Inside DGraphImplement NodeImplement Constructor" << std::endl;
    }
    mId = sNextId++;
    mIncomingEdges = new std::list<OA_ptr<EdgeInterface> >;
    mOutgoingEdges = new std::list<OA_ptr<EdgeInterface> >;
}


//--------------------------------------------------------------------
/*! Remove this edge from the BaseGraph as well as from the list of
    incoming or outgoing edges of the two nodes that
    form this edge. */
void
DGraphImplement::removeEdge (OA_ptr<DGraph::EdgeInterface> pEdge)
{
   mEdgeSet->erase(pEdge);
 
   // put edge into approprate outgoing edges and incoming edges
   pEdge->getSource()->removeOutgoingEdge(pEdge);
   pEdge->getSink()->removeIncomingEdge(pEdge);
 
}

//------------------------------------------------------

//-----------------------------------------------------
/*! Remove the given node and all the incoming and outgoing edges incident on it. */
void
DGraphImplement::removeNode (OA_ptr<DGraph::NodeInterface> pNode)
{
   OA_ptr<EdgesIteratorInterface> einIter =  pNode->getIncomingEdgesIterator();

   for(einIter = pNode->getIncomingEdgesIterator(); einIter->isValid(); ) {
        OA_ptr<EdgeInterface> edge = einIter->current();
        (*einIter)++;
        pNode->removeIncomingEdge(edge);
        edge->getSource()->removeOutgoingEdge(edge);
        mEdgeSet->erase(edge);
   }

   for(einIter = pNode->getOutgoingEdgesIterator(); einIter->isValid(); ) {
        OA_ptr<EdgeInterface> edge = einIter->current();
        (*einIter)++;
        pNode->removeOutgoingEdge(edge);
        edge->getSink()->removeIncomingEdge(edge);
        mEdgeSet->erase(edge);
   }

   mNodeSet->erase(pNode);

}
//-----------------------------------------------------

//========================================================
// Iterators
//========================================================



OA_ptr<NodesIteratorInterface> DGraphImplement::getNodesIterator() const
{
   if(debug)
   {
       std::cout << "Inside DGraphImplement::getNodesIterator" <<std::endl;
   }
           
   OA_ptr<NodesIteratorInterface> retval;
   
   OA_ptr<std::list<OA_ptr<NodeInterface> > > nodeList;
   nodeList = new std::list<OA_ptr<NodeInterface> >;
   std::set<OA_ptr<NodeInterface> >::iterator setIter;
   for (setIter=mNodeSet->begin(); setIter!=mNodeSet->end(); setIter++) {
     if(debug)
     {
        std::cout << "DGraphImplement mNodeSet";
         OA_ptr<NodeInterface> nn = (*setIter);
         std::cout << nn->getId() << std::endl;
         nn->dump(std::cout);
         std::cout << std::endl;
     }
     nodeList->push_back(*setIter);
   }
   retval = new NodesIteratorImplement(nodeList);

   return retval;
}


OA_ptr<NodesIteratorInterface> DGraphImplement::getEntryNodesIterator() const
{
  OA_ptr<NodesIteratorInterface> retval;
  OA_ptr<std::list<OA_ptr<NodeInterface> > > tempList
     = create_entry_list();
  retval = new NodesIteratorImplement(tempList);
  return retval;

}


OA_ptr<NodesIteratorInterface> DGraphImplement::getExitNodesIterator() const
{
  OA_ptr<NodesIteratorInterface> retval;
  OA_ptr<std::list<OA_ptr<NodeInterface> > > templist
    = create_exit_list();
  retval = new NodesIteratorImplement(templist);
  return retval;

}


OA_ptr<NodesIteratorInterface>
    DGraphImplement::getReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient)
{
  OA_ptr<NodesIteratorInterface> retval;
  OA_ptr<std::list<OA_ptr<NodeInterface> > > templist
     = create_reverse_post_order_list(pOrient);
  retval = new NodesIteratorImplement(templist);
  return retval;
}


void DGraphImplement::createDFSList(OA_ptr<NodeInterface> pNode, OA_ptr<std::list<OA_ptr<NodeInterface> > > pList)
{
  // mark as visited so that we don't get in an infinite
  // loop on cycles in the graph
   mVisitMap[pNode] = true;

   pList->push_back(pNode);

   OA_ptr<NodesIteratorInterface> neighIter =  pNode->getSinkNodesIterator();

   for (; neighIter->isValid(); ++(*neighIter)) {
        OA_ptr<NodeInterface> n;
        n = neighIter->current();

        // if the node hasn't been visited then call recursively
        if (!mVisitMap[n]) {
//            pList->push_back(pNode);
            createDFSList(n, pList);
         }
     }


     // add ourselves to the beginning of the list

}


OA_ptr<NodesIteratorInterface>
    DGraphImplement::getDFSIterator(OA_ptr<NodeInterface> n)
{
  OA_ptr<NodesIteratorInterface> retval;

         OA_ptr<NodesIteratorInterface> nodeIter = getNodesIterator();
         for ( ; nodeIter->isValid(); (*nodeIter)++ ) {
            OA_ptr<NodeInterface> node = nodeIter->current();
            mVisitMap[node] = false;
         }

         OA_ptr<std::list<OA_ptr<NodeInterface> > > templist;
         templist = new std::list<OA_ptr<NodeInterface> >;
         createDFSList(n, templist);

         /*! commented out by PLM 09/13/06
         nodeIter = getEntryNodesIterator();
         for ( ; nodeIter->isValid(); (*nodeIter)++ ) {
            OA_ptr<NodeInterface> node = nodeIter->current();
            createDFSList(node,templist);
         }
         */

         retval = new NodesIteratorImplement(templist);


         return retval;

}



OA_ptr<EdgesIteratorInterface> DGraphImplement::getEdgesIterator() const
{
  OA_ptr<EdgesIteratorInterface> retval;
  OA_ptr<std::list<OA_ptr<EdgeInterface> > > edgelist;
  edgelist = new std::list<OA_ptr<EdgeInterface> >;
  std::set<OA_ptr<EdgeInterface> >::iterator setIter;
  if(debug)
     {
       std::cout << "DGraphImplement Printing EdgesIterator" << std::endl;
     }

  for (setIter=mEdgeSet->begin(); setIter!=mEdgeSet->end(); setIter++) {
     edgelist->push_back(*setIter);
  }
  retval = new EdgesIteratorImplement(edgelist);
  return retval;
}


//========================================================
// Output routines
//========================================================
void DGraphImplement::output(IRHandlesIRInterface& ir) const
{
    if(debug)
    {
        std::cout << "Inside DGRaphImplement output " << std::endl;
    }
    OA_ptr<NodesIteratorInterface> nodesIter;
    OA_ptr<EdgesIteratorInterface> edgesIter;

    std::string gname = getGraphName();
    sOutBuild->graphStart(gname);

    // iterate through all nodes
    // and call virtual output methods
    for (nodesIter=getNodesIterator(); nodesIter->isValid(); ++(*nodesIter) ) {
        OA_ptr<NodeInterface> node = nodesIter->current();
        sOutBuild->graphNodeStart(node->getId());
        sOutBuild->graphNodeLabelStart();
        node->output(ir);
        sOutBuild->graphNodeLabelEnd();
        sOutBuild->graphNodeEnd();
    }

    // iterate through all edges
    // and call virtual output methods
    if(debug)
    {
        std::cout << "Printing Edges in the DGraphImplement output" << std::endl; 
    }
    for(edgesIter = getEdgesIterator(); edgesIter->isValid(); (*edgesIter)++) {
        OA_ptr<EdgeInterface> edge = edgesIter->current();
        OA_ptr<NodeInterface> sourceNode = edge->getSource();
        OA_ptr<NodeInterface> sinkNode   = edge->getSink();

        sOutBuild->graphEdgeStart();
        sOutBuild->graphEdgeSourceNode(sourceNode->getId());
        sOutBuild->graphEdgeSinkNode(sinkNode->getId());
        sOutBuild->graphEdgeLabelStart();
        edge->output(ir);
        sOutBuild->graphEdgeLabelEnd();
        sOutBuild->graphEdgeEnd();
    }

    sOutBuild->graphEnd(gname);
}


//========================================================
// Construction
//========================================================
void DGraphImplement::addNode(OA_ptr<NodeInterface> n)
{

    mNodeSet->insert(n);
}

void DGraphImplement::addEdge(OA_ptr<EdgeInterface> e)
{

    if (debug) {
        std::cout << "DGraphImplement::addEdge(e)" << std::endl;
        std::cout << "\te = ";
        e->dump(std::cout);
    }

    // insert the nodes if they don't already exist in the graph
    if (mNodeSet->find(e->getSource()) == mNodeSet->end()) {
        addNode(e->getSource());
        if (debug) {
          std::cout << "\tadding e->getSource, which is ..." << std::endl;
          e->getSource()->dump(std::cout);
        }
    }

    if (mNodeSet->find(e->getSink()) == mNodeSet->end()) {
      addNode(e->getSink());
      if (debug) {
        std::cout << "\tadding e->getSink, which is ..." << std::endl;
        e->getSink()->dump(std::cout);
      }
    }
    mEdgeSet->insert(e);

    // put edge into approprate outgoing edges and incoming edges
    e->getSource()->addOutgoingEdge(e);
    e->getSink()->addIncomingEdge(e);
}
        
std::string DGraphImplement::getGraphName() const
{
    std::string retval("DGraph");
    return retval;
}

//========================================================
// Helper methods
//========================================================

//! returns a list of DGraph NodeInterface's that do not have incoming
OA_ptr<std::list<OA_ptr<NodeInterface> > > 
DGraphImplement::create_entry_list() const
{
    OA_ptr<std::list<OA_ptr<NodeInterface> > > retval;
    retval = new std::list<OA_ptr<NodeInterface> >;

    OA_ptr<NodesIteratorInterface> nodeIter = getNodesIterator();
    for ( ; nodeIter->isValid(); (*nodeIter)++ ) {
        OA_ptr<NodeInterface> node = nodeIter->current();
        if (node->isAnEntry()) {
            retval->push_back(node);
        }
    }

    return retval;
}


OA_ptr<std::list<OA_ptr<NodeInterface> > > 
DGraphImplement::create_exit_list() const
{
    OA_ptr<std::list<OA_ptr<NodeInterface> > > retval;
    retval = new std::list<OA_ptr<NodeInterface> >;

    OA_ptr<NodesIteratorInterface> nodeIter = getNodesIterator();
    for ( ; nodeIter->isValid(); (*nodeIter)++ ) {
        OA_ptr<NodeInterface> node = nodeIter->current();
        if (node->isAnExit()) {
            if (debug) {
                std::cout << "DGraphImplement::create_exit_list" << std::endl;
                std::cout << "\tNode = <";
                node->dump(std::cout);
                std::cout << ">" << std::endl;
            }

            retval->push_back(node);
        }
    }

    return retval;
}


OA_ptr<std::list<OA_ptr<NodeInterface> > >
DGraphImplement::create_reverse_post_order_list(DGraphEdgeDirection pOrient)
{
    // loop over all nodes and set their visit field to false
    OA_ptr<NodesIteratorInterface> nodeIter = getNodesIterator();
    for ( ; nodeIter->isValid(); (*nodeIter)++ ) {
        OA_ptr<NodeInterface> node = nodeIter->current();

        if(debug)
        {
            std::cout << "Set mVisitMap[node]=false" << node->getId() << std::endl;
        }

        mVisitMap[node] = false;
    }

    // generate a list of nodes in the requested ordering
    OA_ptr<std::list<OA_ptr<NodeInterface> > > retval;
    retval = new std::list<OA_ptr<NodeInterface> >;

    // if original edge direction then start ordering from entry nodes
    if (pOrient==DGraph::DEdgeOrg) {
      nodeIter = getEntryNodesIterator();

    // reverse edge direction start at exit nodes
    } else {
      nodeIter = getExitNodesIterator();
    }

    for ( ; nodeIter->isValid(); (*nodeIter)++ ) {
        OA_ptr<NodeInterface> on = nodeIter->current();
        if(debug)
        {
              std::cout << "******DGraphImplement::currentNode" << on->getId();
              on->dump(std::cout);
              std::cout << std::endl;
              std::cout << "*******";
        }
        reverse_postorder_recurse(nodeIter->current(), pOrient, retval);
    }

    return retval;
}


void DGraphImplement::reverse_postorder_recurse( OA_ptr<NodeInterface> pNode,
                                      DGraphEdgeDirection pOrient,
                                      OA_ptr<std::list<OA_ptr<NodeInterface> > > pList )
{
        if (debug) {
          std::cout << "DGraphImplement::reverse_postorder_recurse" << std::endl;
          std::cout << "\tpNode = <";
          pNode->dump(std::cout);
          if(debug)
                {
                  std::cout << "DGraphImplement mNodeSet";
                  std::cout << pNode->getId() << std::endl;
                  std::cout << std::endl;
                }

          std::cout << ">" << std::endl;
        }

        OA_ptr<NodesIteratorInterface> neighIter;

        // mark as visited so that we don't get in an infinite
        // loop on cycles in the graph
        mVisitMap[pNode] = true;

        // loop over the successors or predecessors based on orientation
        if (pOrient==DGraph::DEdgeOrg) {
           OA_ptr<NodesIteratorInterface> it =
           pNode->getSinkNodesIterator(); // Stupid Sun CC 5.4
           neighIter = it;
        } else {
           OA_ptr<NodesIteratorInterface> it =
           pNode->getSourceNodesIterator(); // Stupid Sun CC 5.4
           neighIter = it;
        }

        for (; neighIter->isValid(); ++(*neighIter)) {
           OA_ptr<NodeInterface> n;
           n = neighIter->current();
           if(debug)
                {
                  std::cout << "DGraphImplement mNodeSet";
                  std::cout << n->getId() << std::endl;
                  std::cout << std::endl;
                }


           // if the node hasn't been visited then call recursively
           if (!mVisitMap[n]) {
             reverse_postorder_recurse(n, pOrient, pList);
           }
         }

         // add ourselves to the beginning of the list
         pList->push_front(pNode);
      }
      

      OA_ptr<NodeInterface> EdgeImplement::getSource() const
        {
          return mSourceNode;
        }

      OA_ptr<NodeInterface> EdgeImplement::getSink() const
        {
          return mSinkNode;
        }

      bool EdgeImplement::operator== (EdgeInterface& other)
        {
          return getId() == other.getId();
        }

      bool EdgeImplement::operator< (EdgeInterface& other)
        {
          return getId() < other.getId();
        }

      void EdgeImplement::output(IRHandlesIRInterface& ir) const
        {
          std::ostringstream label;
          label << getId();
          sOutBuild->outputString(label.str());
        }

      void EdgeImplement::dump(std::ostream& os)
        {
          os << "EdgeImplement: getId() = " << getId() << std::endl;
        }

      EdgeImplement::EdgeImplement(OA_ptr<NodeInterface> source,
                      OA_ptr<NodeInterface> sink)
            : mSourceNode(source), mSinkNode(sink)
            {
                mId = sNextId++;
            }

      void NodeImplement::addOutgoingEdge(OA_ptr<EdgeInterface> e)
        {
           assert(!e.ptrEqual(0));
           mOutgoingEdges->push_back(e);
        }

      void NodeImplement::addIncomingEdge(OA_ptr<EdgeInterface> e)
        {
           assert(!e.ptrEqual(0));
           mIncomingEdges->push_back(e);
        }

      void NodeImplement::removeOutgoingEdge(OA_ptr<EdgeInterface> e)
        {
           assert(!e.ptrEqual(0));
           mOutgoingEdges->remove(e);
        }

      void NodeImplement::removeIncomingEdge(OA_ptr<EdgeInterface> e)
        {
           assert(!e.ptrEqual(0));
           mIncomingEdges->remove(e);
        }


      void NodeImplement::dump(std::ostream& os)
        {
          os << "[ " << getId() << " ]";
        }

      void NodeImplement::output(IRHandlesIRInterface& ir) const
        {
          std::ostringstream label;
          label << getId();
          sOutBuild->outputString(label.str());
        }

      bool NodeImplement::operator< (NodeInterface& other)
        {
          return getId() < other.getId();
        }

      bool NodeImplement::operator== (NodeInterface& other)
        {
          if(debug)
          {
              std::cout << "Inside DGraphImplement operator==" << std::endl;
          }
          if(debug)
          {
          unsigned int idi = getId();
          unsigned int oidi = other.getId();
          std::cout << "DGraphImplement::operator::getId" << idi <<std::endl;
          std::cout << "DGraphImplement::operator::other.getId" << oidi << std::endl;
          }
          return getId() == other.getId();
        }

      OA_ptr<NodesIteratorInterface> NodeImplement::getSinkNodesIterator() const
        {
          OA_ptr<NodesIteratorInterface> retval;
          // put all sink nodes in a list
          OA_ptr<std::list<OA_ptr<NodeInterface> > > tempList;
          tempList = new std::list<OA_ptr<NodeInterface> >;
          std::list<OA_ptr<EdgeInterface> >::iterator iter;
          for(iter=mOutgoingEdges->begin(); iter != mOutgoingEdges->end();
          iter++)
          {
              OA_ptr<EdgeInterface> e = *iter;
              tempList->push_back(e->getSink());
          }
          retval = new NodesIteratorImplement(tempList);
          return retval;
        }


      OA_ptr<NodesIteratorInterface> NodeImplement::getSourceNodesIterator() const
        {
          OA_ptr<NodesIteratorInterface> retval;
          // put all source nodes in a list
          OA_ptr<std::list<OA_ptr<NodeInterface> > > tempList;
          tempList = new std::list<OA_ptr<NodeInterface> >;
          std::list<OA_ptr<EdgeInterface> >::iterator iter;
          for(iter=mIncomingEdges->begin(); iter != mIncomingEdges->end();
          iter++)
           {
            OA_ptr<EdgeInterface> e = *iter;
            tempList->push_back(e->getSource());
           }
          retval = new NodesIteratorImplement(tempList);
          return retval;
        }

      OA_ptr<EdgesIteratorInterface> NodeImplement::getOutgoingEdgesIterator() const
        {
          OA_ptr<EdgesIteratorInterface> retval;
          retval = new EdgesIteratorImplement(mOutgoingEdges);
          return retval;
        }

      OA_ptr<EdgesIteratorInterface> NodeImplement::getIncomingEdgesIterator() const
        {
          OA_ptr<EdgesIteratorInterface> retval;
          retval = new EdgesIteratorImplement(mIncomingEdges);
          return retval;
        }

       bool NodeImplement::isAnExit() const 
        {
          return (num_outgoing() == 0); 
        }

       bool NodeImplement::isAnEntry() const 
        {
          return (num_incoming() == 0); 
        }

       int NodeImplement::num_outgoing () const 
        { 
          return mOutgoingEdges->size(); 
        }

       int NodeImplement::num_incoming () const 
        {
          return mIncomingEdges->size(); 
        }

       unsigned int NodeImplement::getId () const 
        {
          return mId; 
        }

       NodesIteratorImplement::NodesIteratorImplement(OA_ptr<NodesIteratorInterface> ni)
        {
          //constructFromOtherIter(ni);
          OA_ptr<NodesIteratorImplement> subClassPtr;
          subClassPtr = ni.convert<NodesIteratorImplement>();
          mNodeList = subClassPtr->mNodeList;
          mIter = mNodeList->begin();
        }

        bool lt_Node::operator()(const OA_ptr<NodeInterface> n1,
                      const OA_ptr<NodeInterface> n2) const
        {
              return (n1->getId() < n2->getId());
        }


        bool lt_Edge::operator()(const OA_ptr<EdgeInterface> e1,
                    const OA_ptr<EdgeInterface> e2) const {
              unsigned int src1 = e1->getSource()->getId();
              unsigned int src2 = e2->getSource()->getId();
              if (src1 == src2) {
                return (e1->getSink()->getId() < e2->getSink()->getId());
              } else {
                return (src1 < src2);
              }
        }


       NodesIteratorImplement::NodesIteratorImplement(OA_ptr<std::list<OA_ptr<NodeInterface> > > nlist)
            : mNodeList(nlist), mIter(nlist->begin()) 
        {
        }

       EdgesIteratorImplement::EdgesIteratorImplement(OA_ptr<EdgesIteratorInterface> ni)
        {
          //constructFromOtherIter(ni);
          OA_ptr<EdgesIteratorImplement> subClassPtr;
          subClassPtr = ni.convert<EdgesIteratorImplement>();
          mEdgeList = subClassPtr->mEdgeList;
          mIter = mEdgeList->begin();
        }

       EdgesIteratorImplement::EdgesIteratorImplement(OA_ptr<std::list<OA_ptr<EdgeInterface> > > elist)
            : mEdgeList(elist), mIter(elist->begin())
            {
            }


        DGraphImplement::DGraphImplement()
        {
          mNodeSet = new std::set<OA_ptr<NodeInterface> >;
          mEdgeSet = new std::set<OA_ptr<EdgeInterface> >;
        }

// temporary workaround until expression graphs have been 
// properly factored out (specialized) from DGraphImplement
OA_ptr<NodeInterface> DGraphImplement::getExprGraphRootNode() const {
  OA_ptr<NodesIteratorInterface> nodeIter = getExitNodesIterator();
  OA_ptr<NodeInterface> node = nodeIter->current();
  assert( !node.ptrEqual(0) );
  return node;
}

    } // end of namespace DGraph
}// end of namespace OA

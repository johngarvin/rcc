/*! \file
  
  \brief Declaration of Kildall iterative data flow solver for directed graphs.

  \authors John Mellor-Crummey (July 1994), 
           adapted for OA by Michelle Strout (April 2004)
  \version $Id: DGraphIterativeDFP.hpp,v 1.7 2005/06/10 02:32:04 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/


#ifndef WorkListPQueue_h
#define WorkListPQueue_h

//#ifndef DirectedGraph_h
//#include <libs/support/graphs/directedGraph/DirectedGraph.h>
//#endif
// Going to attempt to use DGraph instead
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphInterface.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphImplement.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/CFG/CFG.hpp>

#include <iostream>

#include <sys/times.h>

#include <vector>

#include <set>

#include <queue>



#include <OpenAnalysis/DataFlow/WorkList.hpp>

namespace OA {
  namespace DataFlow {

      /*
class node_compare {

    public:
    
      node_compare(std::map<OA_ptr<DGraph::NodeInterface>, int> NodeToPriorityMap) 
          : mNodeToPriorityMap(NodeToPriorityMap)
      {
          
      }

        int operator()( const OA_ptr<DGraph::NodeInterface> x,
                        const OA_ptr<DGraph::NodeInterface> y )
        {
            return ( mNodeToPriorityMap[x] > mNodeToPriorityMap[y] ) ;

        }

    private:
        std::map<OA_ptr<DGraph::NodeInterface>, int> mNodeToPriorityMap;
};
*/

static std::map<OA_ptr<DGraph::NodeInterface>, int> NodeToPriorityMap;

class node_compare {

    public:

        int operator()( const OA_ptr<DGraph::NodeInterface> x,
                        const OA_ptr<DGraph::NodeInterface> y)
        {
            return ( NodeToPriorityMap[x] > NodeToPriorityMap[y] ) ;
        }

};

      
//*********************************************************************
// class Worklist
//*********************************************************************
 
class Worklist_PQueue : public WorkList {

    public:


        Worklist_PQueue(OA_ptr<DGraph::DGraphInterface> dg,
                        DGraph::DGraphEdgeDirection alongFlow) { 

            OA_ptr<DGraph::NodesIteratorInterface> nodeIterPtr
               = dg->getReversePostDFSIterator(alongFlow);

            int priority=1;
            for (; nodeIterPtr->isValid(); ++(*nodeIterPtr)) {
                NodeToPriorityMap[nodeIterPtr->current()] = priority++;
                worklist.push(nodeIterPtr->current());
                worklistSet.insert(nodeIterPtr->current());
            }

        }

        virtual ~Worklist_PQueue() { }
        
        OA_ptr<DGraph::NodeInterface> getNext() 
        {
               OA_ptr<DGraph::NodeInterface> node;
               node = worklist.top();
               worklist.pop();
               worklistSet.erase(node);
               return node;
        }

        void add(OA_ptr<DGraph::NodeInterface> node)
        {
             if (worklistSet.find(node)==worklistSet.end()) {
                 worklist.push(node);
                 worklistSet.insert(node);
             }
        }

        bool isEmpty() 
        {
        
             if(worklist.empty()) 
             { 
                return true; 
             } else {
                return false;
             }
             
        }

        int getPriority( OA_ptr<DGraph::NodeInterface> node) {
            return NodeToPriorityMap[node];
        }

    private:

        // Member Functions
         
        virtual bool atDGraphNode(OA_ptr<DGraph::NodeInterface>,
                                  DGraph::DGraphEdgeDirection)
        {
            return false;
        }

        virtual bool atDGraphEdge(OA_ptr<DGraph::EdgeInterface>,
                                  DGraph::DGraphEdgeDirection)
        {
            return false;
        }

        virtual void finalizeNode(OA_ptr<DGraph::NodeInterface> node)
        {
        }
     
        virtual void finalizeEdge(OA_ptr<DGraph::EdgeInterface> edge)
        {
        }

        // Member Variables
       
        std::priority_queue<OA_ptr<DGraph::NodeInterface>,
                      std::vector<OA_ptr<DGraph::NodeInterface> >,
                      node_compare> worklist;
        
        std::set<OA_ptr<DGraph::NodeInterface> > worklistSet;
        
        
};


  } // end of DataFlow
}  // end of OA namespace

#endif

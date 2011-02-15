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

#ifndef WorkListQueue_h
#define WorkListQueue_h

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

//*********************************************************************
// class Worklist
//*********************************************************************
 
class Worklist_Queue : public WorkList {

    public:

        Worklist_Queue( OA_ptr<DGraph::DGraphInterface> dg,
                        DGraph::DGraphEdgeDirection alongFlow)
        {

           OA_ptr<DGraph::NodesIteratorInterface> nodeIterPtr
              = dg->getReversePostDFSIterator(alongFlow);

            
           for (; nodeIterPtr->isValid(); ++(*nodeIterPtr)) {
                worklist.push(nodeIterPtr->current());
                worklistSet.insert(nodeIterPtr->current());
           }
        }

        virtual ~Worklist_Queue() { }
        
        OA_ptr<DGraph::NodeInterface> getNext()
        {
             OA_ptr<DGraph::NodeInterface> node;     
             node = worklist.front();
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

        bool isEmpty() {

             if(worklist.empty()) {
                return true;
             } else {
                return false;
             }

        }
        


    private:

        // Member Functions

        bool atDGraphNode(OA_ptr<DGraph::NodeInterface>,
                                  DGraph::DGraphEdgeDirection)
        {
            return false;
        }

        bool atDGraphEdge(OA_ptr<DGraph::EdgeInterface>,
                                  DGraph::DGraphEdgeDirection)
        {
            return false;
        }

        void finalizeNode(OA_ptr<DGraph::NodeInterface> node)
        {
        }

        void finalizeEdge(OA_ptr<DGraph::EdgeInterface> edge)
        {
        }

        // Member Variables
        
        std::queue<OA_ptr<DGraph::NodeInterface> > worklist;
        
        std::set<OA_ptr<DGraph::NodeInterface> > worklistSet;
};


  } // end of DataFlow
}  // end of OA namespace

#endif

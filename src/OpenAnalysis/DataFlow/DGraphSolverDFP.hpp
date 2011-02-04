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

#ifndef DGraphSolverDFP_h
#define DGraphSolverDFP_h

//#ifndef DirectedGraph_h
//#include <libs/support/graphs/directedGraph/DirectedGraph.h>
//#endif
// Going to attempt to use DGraph instead
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphInterface.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphImplement.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/CFG/CFG.hpp>
#include <OpenAnalysis/DataFlow/WorkListPQueue.hpp>
#include <OpenAnalysis/DataFlow/WorkListQueue.hpp>


#include <iostream>
#include <sys/times.h>

#include <vector>
#include <set>
#include <queue>

namespace OA {
  namespace DataFlow {

/*! Constant used to specify algorithm used to solve data-flow problem. */
enum DFPImplement { ITERATIVE = 0, WORKLIST_PRIORITY_QUEUE, WORKLIST_QUEUE };

//*********************************************************************
// class DGraphIterativeDFP
//*********************************************************************

class DGraphSolverDFP {
        
public:

    DGraphSolverDFP();
    ~DGraphSolverDFP();
  //--------------------------------------------------------
  // Kildall iterative solver 
  //--------------------------------------------------------
  void solve(OA_ptr<DGraph::DGraphInterface> dg, 
             DGraph::DGraphEdgeDirection alongFlow,
             DFPImplement algorithm);

  int afterSolve_getNumIter() {return numIter;}
  
private:

  int numIter;

  //--------------------------------------------------------
  // initialization callback
  //--------------------------------------------------------
  virtual void initialize(OA_ptr<DGraph::DGraphInterface> dg) = 0;

  //--------------------------------------------------------
  // solver callbacks 
  //--------------------------------------------------------
  //   atDGraphNode and atDGraphEdge return true if change
  //   in dataflow information at the graph element

  virtual bool atDGraphNode(OA_ptr<DGraph::NodeInterface>, 
                            DGraph::DGraphEdgeDirection);

  virtual bool atDGraphEdge(OA_ptr<DGraph::EdgeInterface>, 
                            DGraph::DGraphEdgeDirection);

  //--------------------------------------------------------
  // finalization callbacks
  //--------------------------------------------------------
  virtual void finalizeNode(OA_ptr<DGraph::NodeInterface> node);

  virtual void finalizeEdge(OA_ptr<DGraph::EdgeInterface> edge);

  void Iterative_Solve(OA_ptr<DGraph::DGraphInterface> dg,
             DGraph::DGraphEdgeDirection alongFlow);


  void WorkList_Solve(OA_ptr<DGraph::DGraphInterface> dg,
             DGraph::DGraphEdgeDirection alongFlow,
             DFPImplement algorithm);

  
};

  } // end of DataFlow
}  // end of OA namespace

#endif

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

#ifndef WorkList_h
#define WorkList_h

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

namespace OA {
  namespace DataFlow {

//*********************************************************************
// class Worklist
//*********************************************************************

class WorkList {
    
    public:

        virtual OA_ptr<DGraph::NodeInterface> getNext() = 0;

        virtual void add(OA_ptr<DGraph::NodeInterface> node) = 0;

        virtual bool isEmpty() = 0;

};


  } // end of DataFlow
}  // end of OA namespace

#endif

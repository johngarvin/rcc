/*! \file
  
  \brief Abstract class for solving dataflow analysis problems on CFGs.

  \authors Michelle Strout (April 2004)
           similar to John Mellor-Crummey's CallGraphDFProblem.h
  \version $Id: CFGDFProblem.hpp,v 1.17 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  To use this for performing an intra-procedural dataflow analysis
  problem, privately inherit from the CFGDFProblem class and
  implement the callbacks.

  Added by Priysdarshini Malusare 07/26/06
*/

#ifndef CFGDFSolver_h
#define CFGDFSolver_h

#include <OpenAnalysis/Utils/OA_ptr.hpp>
//#include <OpenAnalysis/DataFlow/DGraphIterativeDFP.hpp>
#include <OpenAnalysis/DataFlow/DGraphSolverDFP.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/IRInterface/CFGIRInterface.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <map>

namespace OA {
  namespace DataFlow {

//*********************************************************************
// class CFGDFProblem
//*********************************************************************
//template <class DataFlowSet>
class CFGDFSolver  : private DataFlow::DGraphSolverDFP {

public:    
  typedef enum { Forward, Backward } DFDirectionType;

private:  
  OA_ptr<DataFlowSet> mTop;
  //OA_ptr<DataFlowSet> mBottom;
  DFDirectionType mDirection;

public:
  //--------------------------------------------------------
  // constructor/destructor
  //--------------------------------------------------------
  CFGDFSolver(DFDirectionType pDirection,  CFGDFProblem& prob);
  virtual ~CFGDFSolver() {}

  //! returns DataFlowSet for Exit for a forward problem 
  //! and for Entry for a backward proble
  OA_ptr<DataFlowSet> solve(OA_ptr<CFG::CFGInterface> cfg, 
                            DFPImplement algorithm);

public:
// It should be private for CFGDFSolver  
    //--------------------------------------------------------
    // For use within InterMPICFGDFProblem
    // DESIGN Note:  Not in CFGDFProblem because Managers inherit
    // from CFGDFProblem privately and I don't know if I want to
    // switch that to public.  See 406 in C++ book.
    // Note that these methods get a ptr to the DataFlowSet
    // and therefore any changes made will be noticed next time
    // performAnalysis is called.
    //--------------------------------------------------------
    OA_ptr<DataFlow::DataFlowSet> getInSet(OA_ptr<CFG::NodeInterface> node)
                   { return mNodeInSetMap[node]; }
    OA_ptr<DataFlow::DataFlowSet> getOutSet(OA_ptr<CFG::NodeInterface> node)
                   { return mNodeOutSetMap[node]; }
                                
private:
  //========================================================
  // implementation of DGraphIterativeDFP callbacks
  // These are implemented in CFGDFProblem.cpp for a CFG.
  //========================================================
  
  //--------------------------------------------------------
  // initialization upcall 
  //--------------------------------------------------------
  void initialize(OA_ptr<DGraph::DGraphInterface> dg);
 

  //--------------------------------------------------------
  // solver upcalls
  //--------------------------------------------------------
  bool atDGraphNode(OA_ptr<DGraph::NodeInterface> node, 
                    DGraph::DGraphEdgeDirection pOrient);
  
  //--------------------------------------------------------
  // finalization upcalls
  //--------------------------------------------------------
  void finalizeNode(OA_ptr<DGraph::NodeInterface> node);


  /*! 
     transfer function for a CFG::Interface::Node
     Will clone in set to block before passing it to this function
     so ok to return a modified in set
  */
  OA_ptr<DataFlowSet> transfer(OA_ptr<DataFlowSet> in, 
                               OA_ptr<DGraph::NodeInterface> n);

  //--------------------------------------------------------
  // debugging
  //--------------------------------------------------------
public:
  void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

protected:
  //std::map<OA::StmtHandle,DataFlowSet*> mStmtInSetMap;
  //std::map<OA::StmtHandle,DataFlowSet*> mStmtOutSetMap;
  std::map<OA_ptr<CFG::NodeInterface>,OA_ptr<DataFlowSet> > mNodeInSetMap;
  std::map<OA_ptr<CFG::NodeInterface>,OA_ptr<DataFlowSet> > mNodeOutSetMap;

  // whether or not the node has had the transfer function applied
  // to all statements at least once
  std::map<OA_ptr<CFG::NodeInterface>,bool> mNodeInitTransApp;
  CFGDFProblem& mDFProb; 
};

  } // end of DataFlow namespace
}  // end of OA namespace

#endif

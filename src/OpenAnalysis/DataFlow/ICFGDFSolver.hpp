/*! \file
  
  \brief Abstract class for solving dataflow analysis problems on ICFGs.

  \authors Michelle Strout 
  \version $Id: ICFGDFSolver.hpp,v 1.2 2005/06/10 02:32:04 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  To use this for performing an interprocedural flow sensitive
  but context insenstive dataflow analysis problems by subclassing
  ICFGDFProblem and implementing the required call backs.
*/

#ifndef ICFGDFSolver_H
#define ICFGDFSolver_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
//#include <OpenAnalysis/DataFlow/DGraphIterativeDFP.hpp>
#include <OpenAnalysis/DataFlow/DGraphSolverDFP.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>
#include <OpenAnalysis/DataFlow/ICFGDFProblem.hpp>
#include <OpenAnalysis/ICFG/ICFGInterface.hpp>
//#include <OpenAnalysis/IRInterface/CFGIRInterface.hpp>
#include <map>

namespace OA {
  namespace DataFlow {

  
//*********************************************************************
// class ICFGDFSolver
//*********************************************************************
class ICFGDFSolver  : private DataFlow::DGraphSolverDFP {
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
  ICFGDFSolver(DFDirectionType pDirection, ICFGDFProblem& prob);
  virtual ~ICFGDFSolver() {}

  //! solves data-flow problem, after done in and out nodes will
  //! have correct data-flow values
  void solve(OA_ptr<ICFG::ICFGInterface> icfg, DFPImplement algorithm);

  //---------------------------------
  // access to DGraphIterativeDFP info
  //---------------------------------
  int getNumIter() { return afterSolve_getNumIter(); }

private:
  //========================================================
  // implementation of DGraphIterativeDFP callbacks
  // These are implemented in ICFGDFSolver.cpp for a CFG.
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
  std::map<OA_ptr<ICFG::NodeInterface>,
           OA_ptr<DataFlowSet> > mNodeInSetMap;
  std::map<OA_ptr<ICFG::NodeInterface>,
           OA_ptr<DataFlowSet> > mNodeOutSetMap;

  // whether or not the node has had the transfer function applied
  // to all statements at least once
  std::map<OA_ptr<ICFG::NodeInterface>,bool> mNodeInitTransApp;

  ICFGDFProblem& mDFProb;

};

  } // end of DataFlow namespace
}  // end of OA namespace

#endif

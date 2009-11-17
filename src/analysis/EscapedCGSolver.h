// -*- Mode: C++ -*-
//
// Copyright (c) 2009 Rice University
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 

// File: EscapedCGSolver.h

// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ESCAPED_CG_SOLVER_H
#define ESCAPED_CG_SOLVER_H

#include <OpenAnalysis/CallGraph/CallGraphInterface.hpp>
#include <OpenAnalysis/DataFlow/CallGraphDFSolver.hpp>
#include <OpenAnalysis/DataFlow/CallGraphDFProblem.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>

class NameBoolDFSet;

class EscapedCGSolver : public OA::DataFlow::CallGraphDFProblem {
public:
  explicit EscapedCGSolver();
  ~EscapedCGSolver();

  void perform_analysis(OA::OA_ptr<OA::CallGraph::CallGraphInterface> call_graph,
			OA::DataFlow::DFPImplement algorithm);

  OA::OA_ptr<OA::DataFlow::DataFlowSet> getInSet(OA::OA_ptr<OA::CallGraph::NodeInterface> n);
  OA::OA_ptr<OA::DataFlow::DataFlowSet> getOutSet(OA::OA_ptr<OA::CallGraph::NodeInterface> n);

  //--------------------------------------------------------
  // initialization callbacks
  //--------------------------------------------------------
  //! Return an initialized top set
  OA::OA_ptr<OA::DataFlow::DataFlowSet>  initializeTop();

  //! Return an initialized bottom set
  OA::OA_ptr<OA::DataFlow::DataFlowSet>  initializeBottom();

  //! Should generate an in and out DataFlowSet for node
  OA::OA_ptr<OA::DataFlow::DataFlowSet>
  initializeNodeIN(OA::OA_ptr<OA::CallGraph::NodeInterface> n);
  OA::OA_ptr<OA::DataFlow::DataFlowSet>
  initializeNodeOUT(OA::OA_ptr<OA::CallGraph::NodeInterface> n);

  //! Should generate an initial DataFlowSet for a procedure
  //! Nodes that we don't have procedure definition for will get ProcHandle(0)
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeNode(OA::ProcHandle proc);
  
  //! Should generate an initial DataFlowSet, use if for a call if both caller
  //! and callee are defined
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeEdge(OA::CallHandle call,
				     OA::ProcHandle caller,
				     OA::ProcHandle callee);
  //! Should generate an initial DataFlowSet for a call,
  //! called when callee is not defined in call graph and therefore
  //! doesn't have a procedure definition handle
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeEdge(OA::CallHandle call,
				     OA::ProcHandle caller,
				     OA::SymHandle callee);

  //--------------------------------------------------------
  // solver callbacks
  //--------------------------------------------------------

  //! OK to modify set1 and return it as result, because solver
  //! only passes a tempSet in as set1
  OA::OA_ptr<OA::DataFlow::DataFlowSet>  meet(
			    OA::OA_ptr<OA::DataFlow::DataFlowSet> set1,
			    OA::OA_ptr<OA::DataFlow::DataFlowSet> set2) ;

  //! What the analysis does for the particular procedure
  OA::OA_ptr<OA::DataFlow::DataFlowSet>  atCallGraphNode(
				       OA::OA_ptr<OA::DataFlow::DataFlowSet> inSet,
				       OA::ProcHandle proc);
  
  //! What the analysis does for a particular call
  //! use if both caller and callee are defined
  OA::OA_ptr<OA::DataFlow::DataFlowSet>  atCallGraphEdge(
				       OA::OA_ptr<OA::DataFlow::DataFlowSet> inSet,
				       OA::CallHandle call,
				       OA::ProcHandle caller,
				       OA::ProcHandle callee);

  //! use if callee is not defined in the call graph
  OA::OA_ptr<OA::DataFlow::DataFlowSet>  atCallGraphEdge(
							 OA::OA_ptr<OA::DataFlow::DataFlowSet> inSet,
				       OA::CallHandle call,
				       OA::ProcHandle caller,
				       OA::SymHandle callee);
  
  //! translate results from procedure node to callee edge if top-down
  //! or to caller edge if bottom-up using data-flow set at procedure node
  //! the proc could be ProcHandle(0) if the called procedure has not
  //! been defined
  OA::OA_ptr<OA::DataFlow::DataFlowSet>  nodeToEdge(
				  OA::ProcHandle proc,
				  OA::OA_ptr<OA::DataFlow::DataFlowSet> procDFSet,
				  OA::CallHandle call);
  
  //! translate results from caller edge to procedure node if top-down
  //! or from callee edge if bottom-up using call data flow set
  OA::OA_ptr<OA::DataFlow::DataFlowSet>  edgeToNode(
				  OA::CallHandle call,
				  OA::OA_ptr<OA::DataFlow::DataFlowSet> callDFSet,
				  OA::ProcHandle proc);
private:
  bool m_solved;
  OA::OA_ptr<OA::DataFlow::CallGraphDFSolver> m_solver;
  OA::OA_ptr<NameBoolDFSet> m_top;
};

#endif

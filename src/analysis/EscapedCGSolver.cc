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

// File: EscapedCGSolver.cc

// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/NameBoolDFSet.h>
#include <analysis/EscapedDFSolver.h>
#include <analysis/VarRefFactory.h>

#include "EscapedCGSolver.h"

using namespace RAnnot;
using namespace OA;
using OA::DataFlow::DataFlowSet;

typedef NameBoolDFSet MyDFSet;

EscapedCGSolver::EscapedCGSolver()
  : m_solved(false)
{
}

EscapedCGSolver::~EscapedCGSolver()
{
}

void EscapedCGSolver::perform_analysis(OA_ptr<CallGraph::CallGraphInterface> call_graph,
						       DataFlow::DFPImplement algorithm)
{
  m_solver = new DataFlow::CallGraphDFSolver(DataFlow::CallGraphDFSolver::BottomUp, *this);
  m_solver->solve(call_graph, algorithm);
  m_solved = true;
}

OA_ptr<DataFlow::DataFlowSet> EscapedCGSolver::getInSet(OA_ptr<CallGraph::NodeInterface> n)
{
  assert(m_solved);
  return m_solver->getInSet(n);
}

OA_ptr<DataFlow::DataFlowSet> EscapedCGSolver::getOutSet(OA_ptr<CallGraph::NodeInterface> n)
{
  assert(m_solved);
  return m_solver->getOutSet(n);
}


//--------------------------------------------------------
// initialization callbacks
//--------------------------------------------------------
//! Return an initialized top set
OA_ptr<DataFlowSet> EscapedCGSolver::initializeTop()
{
  RAnnot::FuncInfo * fi;
  RAnnot::Var * m;
  VarRefFactory * const fact = VarRefFactory::get_instance();
  m_top = new MyDFSet();
  
  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_MENTION(fi, m) {
      OA_ptr<MyDFSet::NameBoolPair> element;
      element = new MyDFSet::NameBoolPair(fact->make_body_var_ref((*m)->getMention_c()), false);
      m_top->insert(element);
    }
  }
  return m_top; 
}

//! Return an initialized bottom set
OA_ptr<DataFlowSet> EscapedCGSolver::initializeBottom()
{
  // Not used.
  assert(0);
}

//! Should generate an in and out DataFlowSet for node
OA_ptr<DataFlowSet>
EscapedCGSolver::initializeNodeIN(OA_ptr<CallGraph::NodeInterface> n)
{
  return m_top->clone();
}

OA_ptr<DataFlowSet>
EscapedCGSolver::initializeNodeOUT(OA_ptr<CallGraph::NodeInterface> n)
{
  return m_top->clone();
}

//! Should generate an initial DataFlowSet for a procedure
//! Nodes that we don't have procedure definition for will get ProcHandle(0)
OA_ptr<DataFlowSet> EscapedCGSolver::initializeNode(ProcHandle proc)
{
  return m_top->clone();
}
  
//! Should generate an initial DataFlowSet, use if for a call if both caller
//! and callee are defined
OA_ptr<DataFlowSet> EscapedCGSolver::initializeEdge(CallHandle call,
						     ProcHandle caller,
						     ProcHandle callee)
{
  return m_top->clone();
}

//! Should generate an initial DataFlowSet for a call,
//! called when callee is not defined in call graph and therefore
//! doesn't have a procedure definition handle
OA_ptr<DataFlowSet> EscapedCGSolver::initializeEdge(CallHandle call,
						     ProcHandle caller,
						     SymHandle callee)
{
  return m_top->clone();
}

//--------------------------------------------------------
// solver callbacks
//--------------------------------------------------------

//! OK to modify set1 and return it as result, because solver
//! only passes a tempSet in as set1
OA_ptr<DataFlowSet>  EscapedCGSolver::meet(
			  OA_ptr<DataFlowSet> set1_orig,
			  OA_ptr<DataFlowSet> set2_orig)
{
  OA_ptr<MyDFSet> set1; set1 = set1_orig.convert<MyDFSet>();
  OA_ptr<MyDFSet> set2; set2 = set2_orig.convert<MyDFSet>();
  
  OA_ptr<MyDFSet> out; out = set1->meet(set2);
  return out.convert<DataFlow::DataFlowSet>();
}

//! What the analysis does for the particular procedure
OA_ptr<DataFlowSet>  EscapedCGSolver::atCallGraphNode(
				     OA_ptr<DataFlowSet> inSetOrig,
				     OA::ProcHandle proc)
{
  OA_ptr<NameBoolDFSet> inSet; inSet = inSetOrig.convert<NameBoolDFSet>();
  FuncInfo * fi = getProperty(FuncInfo, HandleInterface::make_sexp(proc));
  EscapedDFSolver esc_solver(R_Analyst::get_instance()->get_interface());
  OA_ptr<NameBoolDFSet> escaped; escaped = esc_solver.perform_analysis(proc, fi->get_cfg(), inSet);
  return escaped.convert<DataFlowSet>();
}
 
//! What the analysis does for a particular call
//! use if both caller and callee are defined
OA_ptr<DataFlowSet>  EscapedCGSolver::atCallGraphEdge(
				     OA_ptr<DataFlowSet> inSet,
				     CallHandle call,
				     ProcHandle caller,
				     ProcHandle callee)
{
  return inSet;
}

//! use if callee is not defined in the call graph
OA_ptr<DataFlowSet>  EscapedCGSolver::atCallGraphEdge(
				     OA_ptr<DataFlowSet> inSet,
				     CallHandle call,
				     ProcHandle caller,
				     SymHandle callee)
{
  return inSet;
}
  
//! translate results from procedure node to callee edge if top-down
//! or to caller edge if bottom-up using data-flow set at procedure node
//! the proc could be ProcHandle(0) if the called procedure has not
//! been defined
OA_ptr<DataFlowSet>  EscapedCGSolver::nodeToEdge(
				ProcHandle proc,
				OA_ptr<DataFlow::DataFlowSet> procDFSet,
				CallHandle call)
{
  return procDFSet;
}
  
//! translate results from caller edge to procedure node if top-down
//! or from callee edge if bottom-up using call data flow set
OA_ptr<DataFlowSet>  EscapedCGSolver::edgeToNode(
				CallHandle call,
				OA_ptr<DataFlow::DataFlowSet> callDFSet,
				ProcHandle proc)
{
  return callDFSet;
}

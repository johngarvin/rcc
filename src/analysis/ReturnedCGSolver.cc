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

// File: ReturnedCGSolver.cc

// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/NameBoolDFSet.h>
#include <analysis/ReturnedDFSolver.h>
#include <analysis/VarRefFactory.h>

#include "ReturnedCGSolver.h"

using namespace RAnnot;
using namespace OA;
using OA::DataFlow::DataFlowSet;

typedef NameBoolDFSet MyDFSet;

ReturnedCGSolver::ReturnedCGSolver()
{
}

ReturnedCGSolver::~ReturnedCGSolver()
{
}

//--------------------------------------------------------
// initialization callbacks
//--------------------------------------------------------
//! Return an initialized top set
OA_ptr<DataFlowSet> ReturnedCGSolver::initializeTop()
{
  RAnnot::FuncInfo * fi;
  RAnnot::Var * m;
  VarRefFactory * const fact = VarRefFactory::get_instance();
  
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
OA_ptr<DataFlowSet> ReturnedCGSolver::initializeBottom()
{
  // Not used.
  assert(0);
}

//! Should generate an in and out DataFlowSet for node
OA_ptr<DataFlowSet>
ReturnedCGSolver::initializeNodeIN(OA_ptr<CallGraph::NodeInterface> n)
{
  return m_top->clone();
}

OA_ptr<DataFlowSet>
ReturnedCGSolver::initializeNodeOUT(OA_ptr<CallGraph::NodeInterface> n)
{
  return m_top->clone();
}

//! Should generate an initial DataFlowSet for a procedure
//! Nodes that we don't have procedure definition for will get ProcHandle(0)
OA_ptr<DataFlowSet> ReturnedCGSolver::initializeNode(ProcHandle proc)
{
  return m_top->clone();
}
  
//! Should generate an initial DataFlowSet, use if for a call if both caller
//! and callee are defined
OA_ptr<DataFlowSet> ReturnedCGSolver::initializeEdge(CallHandle call,
						     ProcHandle caller,
						     ProcHandle callee)
{
  return m_top->clone();
}

//! Should generate an initial DataFlowSet for a call,
//! called when callee is not defined in call graph and therefore
//! doesn't have a procedure definition handle
OA_ptr<DataFlowSet> ReturnedCGSolver::initializeEdge(CallHandle call,
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
OA_ptr<DataFlowSet>  ReturnedCGSolver::meet(
			  OA_ptr<DataFlowSet> set1_orig,
			  OA_ptr<DataFlowSet> set2_orig)
{
  OA_ptr<MyDFSet> set1; set1 = set1_orig.convert<MyDFSet>();
  OA_ptr<MyDFSet> set2; set2 = set2_orig.convert<MyDFSet>();
  
  OA_ptr<MyDFSet> out; out = set1->meet(set2);
  return out.convert<DataFlow::DataFlowSet>();
}

//! What the analysis does for the particular procedure
OA_ptr<DataFlowSet>  ReturnedCGSolver::atCallGraphNode(
				     OA_ptr<DataFlowSet> inSet,
				     OA::ProcHandle proc)
{
  FuncInfo * fi = getProperty(FuncInfo, HandleInterface::make_sexp(proc));
  ReturnedDFSolver ret_solver(R_Analyst::get_instance()->get_interface());
  OA::OA_ptr<NameBoolDFSet> returned; returned = ret_solver.perform_analysis(proc, fi->get_cfg());
  // TODO: make intra solver take an initial set
  return returned;
}
 
//! What the analysis does for a particular call
//! use if both caller and callee are defined
OA_ptr<DataFlowSet>  ReturnedCGSolver::atCallGraphEdge(
				     OA_ptr<DataFlowSet> inSet,
				     CallHandle call,
				     ProcHandle caller,
				     ProcHandle callee)
{
  // for each callee formal:
  //   if returned(formal), then propagate returned(call) to matching actual
}

//! use if callee is not defined in the call graph
OA_ptr<DataFlowSet>  ReturnedCGSolver::atCallGraphEdge(
				     OA_ptr<DataFlowSet> inSet,
				     CallHandle call,
				     ProcHandle caller,
				     SymHandle callee)
{
  // conservative information
  // each formal is considered returned
}
  
//! translate results from procedure node to callee edge if top-down
//! or to caller edge if bottom-up using data-flow set at procedure node
//! the proc could be ProcHandle(0) if the called procedure has not
//! been defined
OA_ptr<DataFlowSet>  ReturnedCGSolver::nodeToEdge(
				ProcHandle proc,
				OA_ptr<DataFlow::DataFlowSet> procDFSet,
				CallHandle call)
{
  return procDFSet;
}
  
//! translate results from caller edge to procedure node if top-down
//! or from callee edge if bottom-up using call data flow set
OA_ptr<DataFlowSet>  ReturnedCGSolver::edgeToNode(
				CallHandle call,
				OA_ptr<DataFlow::DataFlowSet> callDFSet,
				ProcHandle proc)
{
  return callDFSet;
}

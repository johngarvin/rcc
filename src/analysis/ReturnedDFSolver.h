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

// File: ReturnedDFSolver.h

// Author: John Garvin (garvin@cs.rice.edu)

#ifndef O_ESCAPE_DF_SOLVER_H
#define O_ESCAPE_DF_SOLVER_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/ICFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>

// #include <analysis/NameBoolDFSet.h>
#include <analysis/VarRefFactory.h>

class OA::ICFG::ICFGInterface;
class R_IRInterface;
class ReturnedDFSet;

//temporary
class NameBoolDFSet;

class ReturnedDFSolver : private OA::DataFlow::ICFGDFProblem {
public:
  explicit ReturnedDFSolver(OA::OA_ptr<R_IRInterface> rir);
  ~ReturnedDFSolver();
  OA::OA_ptr<std::map<SEXP, bool> > perform_analysis(OA::OA_ptr<OA::ICFG::ICFGInterface> icfg);

  // ----- debugging -----
  void dump_node_maps();
  void dump_node_maps(std::ostream &os);
  
private:
  // ----- callbacks for CFGDFSolver -----
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeTop();

  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeNodeIN(OA::OA_ptr<OA::ICFG::NodeInterface> n);
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeNodeOUT(OA::OA_ptr<OA::ICFG::NodeInterface> n);

  //! OK to modify set1 and return it as result, because solver
  //! only passes a tempSet in as set1
  OA::OA_ptr<OA::DataFlow::DataFlowSet> meet(OA::OA_ptr<OA::DataFlow::DataFlowSet> set1, 
					     OA::OA_ptr<OA::DataFlow::DataFlowSet> set2);

  //! OK to modify in set and return it again as result because
  //! solver clones the BB in sets. Proc is procedure that
  //! contains the statement.
  OA::OA_ptr<OA::DataFlow::DataFlowSet> transfer(OA::ProcHandle proc,
						 OA::OA_ptr<OA::DataFlow::DataFlowSet> in, 
						 OA::StmtHandle stmt); 

  //! transfer function for the entry node of the given procedure
  //! should manipulate incoming data-flow set in any special ways
  //! for procedure and return outgoing data-flow set for node
  OA::OA_ptr<OA::DataFlow::DataFlowSet> entryTransfer(OA::ProcHandle proc,
						      OA::OA_ptr<OA::DataFlow::DataFlowSet> in);

  //! transfer function for the exit node of the given procedure
  //! should manipulate outgoing data-flow set in any special ways
  //! for procedure and return incoming data-flow set for node
  OA::OA_ptr<OA::DataFlow::DataFlowSet> exitTransfer(OA::ProcHandle proc,
						     OA::OA_ptr<OA::DataFlow::DataFlowSet> out);

  //! Propagate a data-flow set from caller to callee
  OA::OA_ptr<OA::DataFlow::DataFlowSet> callerToCallee(OA::ProcHandle caller,
						       OA::OA_ptr<OA::DataFlow::DataFlowSet> dfset,
						       OA::CallHandle call,
						       OA::ProcHandle callee);
  
  //! Propagate a data-flow set from callee to caller
  OA::OA_ptr<OA::DataFlow::DataFlowSet> calleeToCaller(OA::ProcHandle callee,
						       OA::OA_ptr<OA::DataFlow::DataFlowSet> dfset,
						       OA::CallHandle call,
						       OA::ProcHandle caller);

  // MMA
  //! Propagate a data-flow set from call node to return node
  OA::OA_ptr<OA::DataFlow::DataFlowSet> callToReturn(OA::ProcHandle caller,
						     OA::OA_ptr<OA::DataFlow::DataFlowSet> dfset,
						     OA::CallHandle call,
						     OA::ProcHandle callee);
private:
  OA::OA_ptr<R_IRInterface> m_ir;
  OA::OA_ptr<OA::ICFG::ICFGInterface> m_icfg;
  OA::ProcHandle m_proc;
  OA::OA_ptr<NameBoolDFSet> m_top;
  OA::OA_ptr<OA::DataFlow::ICFGDFSolver> m_solver;
};

#endif

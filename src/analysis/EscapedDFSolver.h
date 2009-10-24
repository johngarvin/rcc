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

// File: OEscapeDFSolver.h

// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ESCAPED_DF_SOLVER_H
#define ESCAPED_DF_SOLVER_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>

#include <analysis/NameBoolDFSet.h>
#include <analysis/VarRefFactory.h>

class OA::CFG::CFGInterface;
class R_IRInterface;
class OA::DataFlow::CFGDFSolver;

class EscapedDFSolver : private OA::DataFlow::CFGDFProblem {
public:
  explicit EscapedDFSolver(OA::OA_ptr<R_IRInterface> rir);
  ~EscapedDFSolver();
  OA::OA_ptr<NameBoolDFSet> perform_analysis(OA::ProcHandle proc,
					     OA::OA_ptr<OA::CFG::CFGInterface> cfg);

  // ----- debugging -----
  void dump_node_maps();
  void dump_node_maps(std::ostream &os);
  
  static bool escaped_predicate(SEXP call, int arg);

private:
  // ----- callbacks for CFGDFSolver -----
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeTop();
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeBottom();

  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeNodeIN(OA::OA_ptr<OA::CFG::NodeInterface> n);
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeNodeOUT(OA::OA_ptr<OA::CFG::NodeInterface> n);

  // CFGDFProblem says: OK to modify set1 and return it as result, because solver
  // only passes a tempSet in as set1
  OA::OA_ptr<OA::DataFlow::DataFlowSet>
  meet (OA::OA_ptr<OA::DataFlow::DataFlowSet> set1, OA::OA_ptr<OA::DataFlow::DataFlowSet> set2);

  // CFGDFProblem says: OK to modify in set and return it again as result because
  // solver clones the BB in sets
  OA::OA_ptr<OA::DataFlow::DataFlowSet>
  transfer(OA::OA_ptr<OA::DataFlow::DataFlowSet> in, OA::StmtHandle stmt); 
  
private:
  OA::OA_ptr<R_IRInterface> m_ir;
  OA::OA_ptr<OA::CFG::CFGInterface> m_cfg;
  OA::ProcHandle m_proc;
  OA::OA_ptr<NameBoolDFSet> m_top;
  OA::OA_ptr<OA::DataFlow::CFGDFSolver> m_solver;
};

#endif

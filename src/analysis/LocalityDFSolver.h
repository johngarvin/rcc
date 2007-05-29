// -*- Mode: C++ -*-
//
// Copyright (c) 2006 Rice University
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

// File: LocalityDFSolver.h
//
// Implements the OpenAnalysis CFG data flow problem interface for a
// function to determine whether variable references refer to local or
// free variables.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef LOCALITY_DF_SOLVER_H
#define LOCALITY_DF_SOLVER_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/CFGDFSolver.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>

#include <analysis/LocalityDFSet.h>

class OA::CFG::Interface;
class R_IRInterface;

namespace Locality {

/// Implements the OpenAnalysis CFG data flow problem interface to
/// determine whether variable references refer to local or free
/// variables.
class LocalityDFSolver : private OA::DataFlow::CFGDFProblem {
public:
  LocalityDFSolver(OA::OA_ptr<R_IRInterface> _rir);
  ~LocalityDFSolver() {}
  void perform_analysis(OA::ProcHandle proc, OA::OA_ptr<OA::CFG::CFGInterface> cfg);
  void dump_node_maps();
  void dump_node_maps(ostream &os);
  //------------------------------------------------------------------
  // Implementing the callbacks for CFGDFProblem
  //------------------------------------------------------------------
private:
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
  void initialize_sets();

  OA::OA_ptr<R_IRInterface> m_ir;
  OA::OA_ptr<DFSet> m_all_top;
  OA::OA_ptr<DFSet> m_all_bottom;
  OA::OA_ptr<DFSet> m_entry_values;
  OA::OA_ptr<OA::CFG::CFGInterface> m_cfg;
  OA::ProcHandle m_proc;
  OA::OA_ptr<OA::DataFlow::CFGDFSolver> m_solver;
};

}

#endif  // LOCALITY_DF_SOLVER_H

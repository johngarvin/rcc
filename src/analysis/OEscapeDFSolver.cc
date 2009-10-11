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

// File: OEscapeDFSolver.cc

// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>
#include <analysis/NameMentionMultiMap.h>

#include "OEscapeDFSolver.h"

using namespace RAnnot;
using namespace OA;
using namespace HandleInterface;

OEscapeDFSolver::OEscapeDFSolver(OA_ptr<R_IRInterface> ir)
  : m_ir(ir)
{}

OEscapeDFSolver::~OEscapeDFSolver()
{}

OA_ptr<NameMentionMultiMap> OEscapeDFSolver::perform_analysis(ProcHandle proc,
							      OA_ptr<CFG::CFGInterface> cfg)
{
  // TODO
}

// ----- debugging -----

void OEscapeDFSolver::dump_node_maps() {
  // TODO
}

void OEscapeDFSolver::dump_node_maps(std::ostream &os) {
  // TODO
}

// ----- callbacks for CFGDFProblem -----

OA_ptr<DataFlow::DataFlowSet> OEscapeDFSolver::initializeTop() {
  // TODO
}

OA_ptr<DataFlow::DataFlowSet> OEscapeDFSolver::initializeBottom() {
  // TODO
}

OA_ptr<DataFlow::DataFlowSet> OEscapeDFSolver::initializeNodeIN(OA_ptr<CFG::NodeInterface> n) {
  // TODO
}

OA_ptr<DataFlow::DataFlowSet> OEscapeDFSolver::initializeNodeOUT(OA_ptr<CFG::NodeInterface> n) {
  // TODO
}

OA_ptr<DataFlow::DataFlowSet> OEscapeDFSolver::meet(OA_ptr<DataFlow::DataFlowSet> set1_orig,
						    OA_ptr<DataFlow::DataFlowSet> set2_orig)
{
  // TODO
}

OA_ptr<DataFlow::DataFlowSet> OEscapeDFSolver::transfer(OA_ptr<DataFlow::DataFlowSet> in_dfs,
							StmtHandle stmt_handle)
{
  // TODO
}

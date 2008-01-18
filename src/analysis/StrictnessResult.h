// -*- Mode: C++ -*-
//
// Copyright (c) 2008 Rice University
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

// File: StrictnessResult.h
//
// Container for data returned by strictness data flow solver.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef STRICTNESS_RESULT_H
#define STRICTNESS_RESULT_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>

#include <analysis/StrictnessDFSet.h>
#include <analysis/NameMentionMultiMap.h>
#include <analysis/NameStmtMultiMap.h>

namespace Strictness {

class StrictnessResult {
public:
  StrictnessResult(OA::OA_ptr<Strictness::DFSet> args_on_exit,
		   OA::OA_ptr<NameMentionMultiMap> debuts,
		   OA::OA_ptr<NameStmtMultiMap> post_debut_stmts)
    : m_args_on_exit(args_on_exit),
      m_debuts(debuts),
      m_post_debut_stmts(post_debut_stmts)
    {}
  OA::OA_ptr<Strictness::DFSet> get_args_on_exit() { return m_args_on_exit; }
  OA::OA_ptr<NameMentionMultiMap> get_debuts() { return m_debuts; }
  OA::OA_ptr<NameStmtMultiMap> get_post_debut_stmts() { return m_post_debut_stmts; }
private:
  OA::OA_ptr<Strictness::DFSet> m_args_on_exit;
  OA::OA_ptr<NameMentionMultiMap> m_debuts;
  OA::OA_ptr<NameStmtMultiMap> m_post_debut_stmts;
};

}  // end namespace Strictness

#endif

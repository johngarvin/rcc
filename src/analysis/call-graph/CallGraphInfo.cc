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

// File: CallGraphInfo.cc
//
// Contains links to in and out edges for each call graph node.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <support/DumpMacros.h>

#include <analysis/call-graph/RccCallGraphAnnotationMap.h>

#include "CallGraphInfo.h"

namespace RAnnot {

  // typedef for readability
  typedef CallGraphInfo::MySetT MySetT;

  CallGraphInfo::CallGraphInfo()
  {}

  CallGraphInfo::~CallGraphInfo()
  {}

  // ----- methods to insert edges -----

  void CallGraphInfo::insert_call_in(const CallGraphEdge * const edge) {
    m_calls_in.insert(edge);
  }

  void CallGraphInfo::insert_call_out(const CallGraphEdge * const edge) {
    m_calls_out.insert(edge);
  }

  // ----- iterators -----

  MySetT::const_iterator CallGraphInfo::begin_calls_in() const {
    return m_calls_in.begin();
  }

  MySetT::const_iterator CallGraphInfo::end_calls_in() const {
    return m_calls_in.end();
  }

  MySetT::const_iterator CallGraphInfo::begin_calls_out() const {
    return m_calls_out.begin();
  }

  MySetT::const_iterator CallGraphInfo::end_calls_out() const {
    return m_calls_out.end();
  }

  // ----- debug -----

  std::ostream & CallGraphInfo::dump(std::ostream & os) const {
    MySetT::const_iterator it;

    if (m_calls_in.empty()) {
      os << "Calls in: <empty>" << std::endl;
    } else {
      os << "Calls in:" << std::endl;
      for (it = begin_calls_in(); it != end_calls_in(); ++it) {
	const CallGraphEdge * edge = *it;
	dumpPtr(os, edge->get_source());
      }
    }

    if (m_calls_out.empty()) {
      os << "Calls out: <empty>" << std::endl;
    } else {
      os << "Calls out:" << std::endl;
      for (it = begin_calls_out(); it != end_calls_out(); ++it) {
	const CallGraphEdge * edge = *it;
	dumpPtr(os, edge->get_sink());
      }
    }
  }

} // end namespace RAnnot

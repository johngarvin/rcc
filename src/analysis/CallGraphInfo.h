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

// File: CallGraphInfo.h
//
// Contains links to in and out edges for each call graph node.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef CALL_GRAPH_INFO_H
#define CALL_GRAPH_INFO_H

#include <set>

#include <analysis/CallGraphEdge.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class CallGraphInfo {
public:
  typedef std::set<const CallGraphEdge *> MySetT;
  typedef MySetT::const_iterator const_iterator;

  explicit CallGraphInfo();
  virtual ~CallGraphInfo();

  void insert_call_in(const CallGraphEdge * const edge);
  void insert_call_out(const CallGraphEdge * const edge);

  const_iterator begin_calls_in() const;
  const_iterator end_calls_in() const;
  const_iterator begin_calls_out() const;
  const_iterator end_calls_out() const;

  std::ostream & dump(std::ostream & stream) const;

private:
  MySetT m_calls_in;
  MySetT m_calls_out;
};

} // end namespace RAnnot

#endif

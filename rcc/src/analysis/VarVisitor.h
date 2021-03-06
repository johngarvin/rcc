// -*- Mode: C++ -*-
//
// Copyright (c) 2007 Rice University
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

// File: VarVisitor.h
//
// Visitor for Vars.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef VAR_VISITOR_H
#define VAR_VISITOR_H

namespace RAnnot {

class UseVar;
class DefVar;

class VarVisitor {
public:
  virtual void visitUseVar(UseVar * uv) = 0;
  virtual void visitDefVar(DefVar * dv) = 0;
};

#if 0 
template<class R> class VarVisitor {
public:
  virtual R visitUseVar(UseVar * uv) = 0;
  virtual R visitDefVar(DefVar * dv) = 0;
};

template<class R> class VarVisitorAdapter : public VarVisitor<void *> {
private:
  VarVisitor<R> * m_visitor;
public:
  explicit VarVisitorAdapter(VarVisitor<R> * visitor) : m_visitor(visitor) {}

  void * visitUseVar(UseVar * uv) {
    return static_cast<void *>(m_visitor->visitUseVar(uv));
  }
  void * visitDefVar(DefVar * dv) {
    return static_cast<void *>(m_visitor->visitDefVar(dv));
  }
};
#endif

}

#endif

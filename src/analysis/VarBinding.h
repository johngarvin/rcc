// -*- Mode: C++ -*-

#ifndef VAR_BINDING_H
#define VAR_BINDING_H

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

// File: VarBinding.h
//
// Annotates a mention with binding information: the list of scopes in
// which the mention may be bound locally. (Most mentions are bound in
// only one scope, so the list will commonly be a singleton.) Knows
// how to compute itself using local/free information from Var.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <vector>
#include <ostream>

#include <OpenAnalysis/Utils/Iterator.hpp>

#include <analysis/Annotation.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class VarBinding : public AnnotationBase,
		   public OA::Iterator
{
public:

  // typedefs to let us function as wrapper around container
  typedef FuncInfo *                            KeyT;
  typedef std::vector<KeyT>                     MyContainerT;

  explicit VarBinding(); // constructor

  // inherited from Iterator
  bool isValid() const;
  void operator++();
  
  // other iteration methods
  KeyT current();
  void insert(KeyT key);
  void reset();

  // get handle to lookup in PropertySet
  static PropertyHndlT handle();

  bool is_global();

  // clone
  AnnotationBase * clone();

  // debug
  std::ostream & dump(std::ostream & os) const;

private:

  MyContainerT m_scopes; // sequence of scopes where var is bound
  MyContainerT::iterator m_pointer;

  static PropertyHndlT m_handle;
};

}

#endif

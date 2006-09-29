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
// which the mention may be bound locally. Knows how to compute itself
// using local/free information from Var.
//
// Most mentions are bound in only one scope, so the list will
// commonly be a singleton. An empty scope list means the variable is
// unbound or (more likely) refers to an R library/special/builtin.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <list>
#include <ostream>

#include <OpenAnalysis/Utils/Iterator.hpp>

#include <analysis/AnnotationBase.h>
#include <analysis/FuncInfo.h>
#include <analysis/PropertyHndl.h>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

namespace RAnnot {

class VarBinding : public AnnotationBase
{
public:

  // typedefs to let us function as wrapper around container
  typedef FuncInfo *                            KeyT;
  typedef std::list<KeyT>                       MyContainerT;
  typedef MyContainerT::const_iterator          const_iterator;

  explicit VarBinding(); // constructor

  // iterators for list of scopes
  const_iterator begin();
  const_iterator end();
  
  void insert(KeyT key);

  /// get handle to lookup in PropertySet
  static PropertyHndlT handle();

  /// Does this mention refer to the global as its only scope?
  bool is_global();

  /// Is this mention bound in only one scope?
  bool is_single();

  /// Is this mention bound in no scopes? (may be an internal name or just plain unbound)
  bool is_unbound();

  /// get the location in the R environment (empty string if no unique location)
  std::string get_location(SEXP name, SubexpBuffer* sb);

  // clone
  AnnotationBase * clone();

  // debug
  std::ostream & dump(std::ostream & os) const;

private:

  MyContainerT m_scopes; // sequence of scopes where var is bound

  static PropertyHndlT m_handle;
};

}

#endif

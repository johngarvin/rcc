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

// File: VarInfo.h
//
// In each SymbolTable, a name maps to a VarInfo. A VarInfo is
// associated with a coordinate (a name in a scope) and contains a
// list of defs that define the name in that scope. (The defs are not
// necessarily in the variable's scope; they may be local ("<-") in
// the variable's scope or free ("<<-") in child scopes.)
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ANNOTATION_VAR_INFO_H
#define ANNOTATION_VAR_INFO_H

#include <ostream>
#include <list>

#include <analysis/DefVar.h>

// ----- forward declarations -----

class SubexpBuffer;
class LexicalScope;

namespace RAnnot {

/// VarInfo: Location information about a variable
class VarInfo
  : public AnnotationBase
{
public:
  typedef DefVar *                                          key_type;

  typedef std::list<key_type>                               MySet_t;
  typedef key_type                                          value_type;
  typedef MySet_t::iterator                                 iterator;
  typedef MySet_t::const_iterator                           const_iterator;
  typedef MySet_t::size_type                                size_type;

public:
  // constructor for SymbolTables in LexicalScopes
  VarInfo(const SEXP name, const LexicalScope * const scope);

  // constructor for special SymbolTable for ambiguous references
  VarInfo(const SEXP name);

  virtual ~VarInfo();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual VarInfo * clone();

  // defs iterators:
  iterator begin_defs();
  const_iterator begin_defs() const;
  iterator end_defs();
  const_iterator end_defs() const;
  
  // defs capacity:
  size_type size_defs() const;
  
  // defs modifiers:
  void insert_def(const value_type& x);
  iterator insert_def(iterator position, const value_type& x);

  void erase_defs(iterator position);
  void erase_defs(iterator first, iterator last);

  void clear_defs();

  bool is_param();
  bool is_internal();

  // if there is exactly one def, return it, otherwise return 0
  key_type single_def_if_exists();

  /// get the location in the R environment
  std::string get_location(SubexpBuffer* sb);

  const SEXP get_name() const;
  const LexicalScope * const get_scope() const;
  bool has_scope() const;

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  const SEXP m_name;
  const LexicalScope * const m_scope;
  MySet_t m_defs;
  std::string m_c_location;
  bool m_param;
};

}

#endif

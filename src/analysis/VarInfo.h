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
#include <analysis/UseVar.h>

// ----- forward declarations -----

class SubexpBuffer;
class LexicalScope;

namespace RAnnot {

/// VarInfo: Location information about a variable
class VarInfo
  : public AnnotationBase
{
public:
  typedef const DefVar *                                    DefT;
  typedef const UseVar *                                    UseT;

  typedef std::list<DefT>                                   DefSetT;
  typedef std::list<UseT>                                   UseSetT;
  typedef DefSetT::iterator                                 DefIterator;
  typedef DefSetT::const_iterator                           ConstDefIterator;
  typedef DefSetT::size_type                                DefSizeT;
  typedef UseSetT::iterator                                 UseIterator;
  typedef UseSetT::const_iterator                           ConstUseIterator;
  typedef UseSetT::size_type                                UseSizeT;

public:
  // constructor for SymbolTables in LexicalScopes
  explicit VarInfo(const SEXP name, const LexicalScope * const scope);

  // constructor for special SymbolTable for ambiguous references
  explicit VarInfo(const SEXP name);

  virtual ~VarInfo();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual VarInfo * clone();

  // defs iterators:
  DefIterator begin_defs();
  ConstDefIterator begin_defs() const;
  DefIterator end_defs();
  ConstDefIterator end_defs() const;
  
  // defs capacity:
  DefSizeT size_defs() const;
  
  void erase_defs(DefIterator position);
  void erase_defs(DefIterator first, DefIterator last);
  void clear_defs();

  // uses iterators:
  UseIterator begin_uses();
  ConstUseIterator begin_uses() const;
  UseIterator end_uses();
  ConstUseIterator end_uses() const;
  
  // uses capacity:
  UseSizeT size_uses() const;
  
  void erase_uses(UseIterator position);
  void erase_uses(UseIterator first, UseIterator last);
  void clear_uses();

  // insert def or use
  void insert_var(const Var * var);

  // true if the name is a formal argument
  bool is_param();

  // true if it's a library
  bool is_internal();

  // if there is exactly one def, return it, otherwise return 0
  DefT single_def_if_exists();

  /// get the location in the R environment
  std::string get_location(SubexpBuffer * sb);

  const SEXP get_name() const;
  const LexicalScope * const get_scope() const;
  bool has_scope() const;

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream & dump(std::ostream & os) const;

private:
  const SEXP m_name;
  const LexicalScope * const m_scope;
  DefSetT m_defs;
  UseSetT m_uses;
  std::string m_c_location;
  bool m_param;
};

}

#endif

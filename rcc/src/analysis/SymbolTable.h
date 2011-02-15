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

// File: SymbolTable.h
//
// A table mapping names (SEXPs of SYMSXP type) to VarInfos describing
// them. There's a SymbolTable for each lexical scope, plus an extra
// symbol table for ambiguously scoped mentions.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <map>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationBase.h>
#include <analysis/PropertyHndl.h>

// ---------------------------------------------------------------------------
// Name (symbol table key)
// ---------------------------------------------------------------------------

typedef SEXP Name;  // should be of type SYMSXP

// ---------------------------------------------------------------------------
// Symbol table
// ---------------------------------------------------------------------------

// ----- forward declarations outside RAnnot -----

class LexicalScope;

namespace RAnnot {

// ----- forward declarations -----

class VarInfo;

// ----- class definition -----

class SymbolTable
{
public:
  enum AllocT {
    Alloc_Heap,  // the heap
    Alloc_Frame, // the local frame of the execution stack
    Alloc_Stack  // separately managed stack of objects
  };

  typedef Name                                              key_type;
  typedef VarInfo *                                         mapped_type;
  
  typedef std::map<Name, VarInfo *>                         MyMap_t;
  typedef std::pair<const key_type, mapped_type>            value_type;
  typedef MyMap_t::key_compare                              key_compare;
  typedef MyMap_t::allocator_type                           allocator_type;
  typedef MyMap_t::reference                                reference;
  typedef MyMap_t::const_reference                          const_reference;
  typedef MyMap_t::iterator                                 iterator;
  typedef MyMap_t::const_iterator                           const_iterator;
  typedef MyMap_t::size_type                                size_type;

public:
  explicit SymbolTable();
  virtual ~SymbolTable();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual SymbolTable * clone();

  // -------------------------------------------------------
  // iterator, find/insert, etc 
  // -------------------------------------------------------

  // iterators:
  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;
  
  // capacity:
  size_type size() const;

  // element access:
  mapped_type & operator[](const key_type & x);

  // modifiers:
  std::pair<iterator, bool> insert(const value_type & x);
  iterator insert(iterator position, const value_type & x);

  void erase(iterator position);
  size_type erase(const key_type & x);
  void erase(iterator first, iterator last);

  void clear();

  // map operations:
  iterator find(const key_type & x);
  const_iterator find(const key_type & x) const;
  size_type count(const key_type & x) const;

  // if value exists for the given key, return it. Otherwise, create a
  // new value, map key to value, and return the value.
  mapped_type find_or_create(const key_type & k, const LexicalScope * const scope);
  
  // -------------------------------------------------------
  // code generation
  // -------------------------------------------------------
  // genCode1: generate code to initialize ST
  // genCode2...

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream & dump(std::ostream & os) const;

  // single symbol table for mentions in ambiguous scope
  static SymbolTable * get_ambiguous_st();

private:
  MyMap_t m_vars; // (contents of map not owned)
  static SymbolTable * s_ambiguous_st;
};

} // end namespace RAnnot

#endif

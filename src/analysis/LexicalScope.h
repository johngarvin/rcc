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

// File: LexicalScope.h
//
// Represents an R lexical scope. A lexical scope can be pre-defined
// (InternalLexicalScope) (e.g. the scope where R internal names are
// bound) or created by a function definition (FundefLexicalScope). The
// global scope, because it contains code, is represented as a
// FundefLexicalScope.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef LEXICAL_SCOPE_H
#define LEXICAL_SCOPE_H

#include <string>

#include <include/R/R_RInternals.h>

namespace RAnnot { class SymbolTable; }

class LexicalScope {
public:
  explicit LexicalScope();
  virtual ~LexicalScope();

  virtual const std::string get_name() const = 0;

  RAnnot::SymbolTable * get_symbol_table() const;

  void dump(std::ostream & os) const;
private:
  RAnnot::SymbolTable * m_st;
};

class InternalLexicalScope : public LexicalScope {
  // singleton pattern
private:
  explicit InternalLexicalScope();
public:
  static InternalLexicalScope * get_instance();

  const std::string get_name() const;
private:
  static InternalLexicalScope * s_instance;
};

class FundefLexicalScope : public LexicalScope {
public:
  explicit FundefLexicalScope(SEXP fundef);
  SEXP get_fundef() const;
  const std::string get_name() const;
private:
  SEXP m_fundef;
};

#endif

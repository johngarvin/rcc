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

// File: UseVar.h
//
// An annotation representing a variable reference that is a use.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ANNOTATION_USE_VAR_H
#define ANNOTATION_USE_VAR_H

#include <include/R/R_RInternals.h>

#include <analysis/BasicVar.h>
#include <analysis/VarVisitor.h>

namespace RAnnot {

/// UseVar: A variable reference that is a use
class UseVar
  : public BasicVar
{
public:
  enum PositionT {
    UseVar_FUNCTION,
    UseVar_ARGUMENT
  };

public:
  explicit UseVar(SEXP mention_c, PositionT position, MayMustT mmt, Locality::LocalityType lt);
  virtual ~UseVar();

  // function or argument position
  PositionT get_position_type() const;

  virtual SEXP get_name() const;

  void accept(VarVisitor * v);

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual UseVar * clone();

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream & dump(std::ostream & os) const;

private:
  const PositionT m_position_type;
};

const std::string type_name(const UseVar::PositionT x);

}

#endif

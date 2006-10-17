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

// File: VarRef.h
//
// Represents a mention (use or def) of a variable.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef VAR_REF_H
#define VAR_REF_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <include/R/R_RInternals.h>

/// R_VarRef
/// An R_VarRef represents a mention (use or def) of a variable in R
/// code. Designed to be compared in different ways: two different
/// locations with the same name are "==" but not "equiv".  The name of
/// the mention is a SYMSXP in the R AST. Since all SYMSXPs with the
/// same name are the same pointer, we differentiate refs by
/// representing an R_VarRef as a CONS cell containing the symbol.
/// This is a base class: the way to get the name will be different
/// depending on what kind of reference it is.
class R_VarRef {
public:
  virtual ~R_VarRef() {};

  // Comparison operators for use in STL containers
  virtual bool operator<(R_VarRef & loc2) const = 0;
  virtual bool operator<(const R_VarRef & loc2) const = 0;
  virtual bool operator==(R_VarRef & loc2) const = 0;

  virtual bool equiv(R_VarRef & loc2) const = 0;

  virtual std::string toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir) const = 0;
  virtual std::string toString() const = 0;

  virtual SEXP get_sexp() const = 0;
  virtual SEXP get_name() const = 0;
};

/// R_BodyVarRef
/// A VarRef found in code. A CONS cell where the CAR is the symbol.
class R_BodyVarRef : public R_VarRef {
public:
  R_BodyVarRef(SEXP _loc) : m_loc(_loc) {}
  
  // Comparison operators for use in STL containers
  bool operator<(R_VarRef & loc2) const;
  bool operator<(const R_VarRef & loc2) const;
  bool operator==(R_VarRef & loc2) const;

  bool equiv(R_VarRef & loc2) const;

  std::string toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir) const;
  std::string toString() const;

  SEXP get_sexp() const;
  SEXP get_name() const;
  
private:
  SEXP m_loc;
};

/// R_ArgVarRef
/// A VarRef found as a formal argument. A CONS cell where the TAG is the symbol.
class R_ArgVarRef : public R_VarRef {
public:
  R_ArgVarRef(SEXP _loc) : m_loc(_loc) {}
  
  // Comparison operators for use in STL containers
  bool operator<(R_VarRef & loc2) const;
  bool operator<(const R_VarRef & loc2) const;
  bool operator==(R_VarRef & loc2) const;

  bool equiv(R_VarRef & loc2) const;

  std::string toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir) const;
  std::string toString() const;

  SEXP get_sexp() const;
  SEXP get_name() const;

private:
  SEXP m_loc;
};

#endif

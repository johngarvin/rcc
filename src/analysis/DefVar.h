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

// File: DefVar.h
//
// Annotation for a variable mention that is a def.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ANNOTATION_DEF_VAR
#define ANNOTATION_DEF_VAR

#include <include/R/R_RInternals.h>

#include <analysis/Var.h>

namespace RAnnot {

// ---------------------------------------------------------------------------
// DefVar: A variable reference that is a def
// ---------------------------------------------------------------------------
class DefVar
  : public Var
{
public:
  enum SourceT {
    DefVar_ASSIGN,  // defined by an assignment statement
    DefVar_FORMAL,  // defined as a formal variable by a function
    DefVar_PHI      // defined by a phi function
  };
public:
  DefVar();
  virtual ~DefVar();

  // source of definition: assignment statement, formal argument, or phi function
  SourceT getSourceType() const
    { return mSourceType; }
  void setSourceType(SourceT x)
    { mSourceType = x; }

  virtual SEXP getName() const;

  SEXP getRhs_c() const
    { return mRhs_c; }
  void setRhs_c(SEXP x)
    { mRhs_c = x; }

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual DefVar* clone() { return new DefVar(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  SourceT mSourceType;
  SEXP mRhs_c;               // cell containing right side, if assignment
  // bool mIsBindingKnown;
  // set of uses reached?
};

const std::string typeName(const DefVar::SourceT x);

}

#endif

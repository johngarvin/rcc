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

// File: ExprTreeBuilder.h
//
// Builds an OA::ExprTree given an SEXP.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>

#include <include/R/R_RInternals.h>

#ifndef EXPR_TREE_BUILDER_H
#define EXPR_TREE_BUILDER_H

class ExprTreeBuilder {
public:
  OA::OA_ptr<OA::ExprTree> build(SEXP e);

public:
  static ExprTreeBuilder * get_instance();

private:
  
  // private constructor for singleton
  explicit ExprTreeBuilder();
  static ExprTreeBuilder * s_instance;
};

#endif

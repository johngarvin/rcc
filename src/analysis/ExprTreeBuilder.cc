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

// File: ExprTreeBuilder.cc
//
// Builds an OA::ExprTree given an SEXP.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/AnalysisException.h>
#include <analysis/HandleInterface.h>
#include <analysis/Utils.h>

#include "ExprTreeBuilder.h"

using namespace OA;
using namespace HandleInterface;

OA_ptr<ExprTree> ExprTreeBuilder::build(SEXP cell) {
  assert(is_cons(cell));
  SEXP e = CAR(cell);
  OA_ptr<ExprTree> tree; tree = new ExprTree();

  if (is_const(e)) {
    OA_ptr<ExprTree::ConstValNode> n; n = new ExprTree::ConstValNode(make_const_val_h(e));
    tree->addNode(n);
  } else if (is_var(e)) {
    // use the cell to create the handle
    OA_ptr<ExprTree::ConstSymNode> n; n = new ExprTree::ConstSymNode(make_const_sym_h(e));
    tree->addNode(n);
  } else if (is_assign(e)) {
    OA_ptr<ExprTree::OpNode> assign; assign = new ExprTree::OpNode(make_op_h(e));
    OA_ptr<ExprTree> lhs = build(assign_lhs_c(e));
    OA_ptr<ExprTree> rhs = build(assign_rhs_c(e));
    tree->addNode(assign);
    tree->copyAndConnectSubTree(assign, lhs);
    tree->copyAndConnectSubTree(assign, rhs);
  } else if (is_fundef(e)) {
    // TODO
    throw AnalysisException();
  } else if (is_struct_field(e)) {
    // TODO
    throw AnalysisException();
  } else if (is_subscript(e)) {
    // TODO
    throw AnalysisException();
  } else if (is_if(e)) {
    // TODO
    throw AnalysisException();
  } else if (is_for(e)) {
    // TODO
    throw AnalysisException();
  } else if (is_while(e)) {
    // TODO
    throw AnalysisException();
  } else if (is_repeat(e)) {
    // TODO
    throw AnalysisException();
  } else if (is_curly_list(e)) {
    // TODO
    throw AnalysisException();
  } else if (is_call(e)) {  // regular function call
    // TODO
    throw AnalysisException();
  } else {
    assert(0);
  }
  return tree;
}

// private constructor for singleton
ExprTreeBuilder::ExprTreeBuilder() {
}

ExprTreeBuilder * ExprTreeBuilder::get_instance() {
  if (s_instance == 0) {
    s_instance = new ExprTreeBuilder();
  }
  return s_instance;
}

// static member
ExprTreeBuilder * ExprTreeBuilder::s_instance = 0;

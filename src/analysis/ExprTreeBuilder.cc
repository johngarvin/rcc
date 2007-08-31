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

#include <support/RccError.h>

#include "ExprTreeBuilder.h"

using namespace OA;
using namespace HandleInterface;

OA_ptr<ExprTree> ExprTreeBuilder::build(SEXP e) {
  OA_ptr<ExprTree> tree; tree = new ExprTree();

  if (is_const(e)) {
    OA_ptr<ExprTree::ConstValNode> n; n = new ExprTree::ConstValNode(make_const_val_h(e));
    tree->addNode(n);
  } else if (is_var(e)) {
    // TODO: is MemRefNode/MemRefHandle right?
    // Note: a ConstSymHandle doesn't belong here; that represents a constant bound to a symbol
    OA_ptr<ExprTree::MemRefNode> n; n = new ExprTree::MemRefNode(make_mem_ref_h(e));
    tree->addNode(n);
  } else if (is_assign(e)) {
    OA_ptr<ExprTree::OpNode> assign; assign = new ExprTree::OpNode(make_op_h(e));
    OA_ptr<ExprTree> lhs = build(CAR(assign_lhs_c(e)));
    OA_ptr<ExprTree> rhs = build(CAR(assign_rhs_c(e)));
    tree->addNode(assign);
    tree->copyAndConnectSubTree(assign, lhs);
    tree->copyAndConnectSubTree(assign, rhs);
  } else if (is_fundef(e)) {
    // TODO
    rcc_warn("ExprTreeBuilder: fundefs not yet implemented");
    throw AnalysisException();
  } else if (is_struct_field(e)) {
    // TODO
    rcc_warn("ExprTreeBuilder: structure fields not yet implemented");
    throw AnalysisException();
  } else if (is_subscript(e)) {
    // TODO
    rcc_warn("ExprTreeBuilder: subscript expressions not yet implemented");
    throw AnalysisException();
  } else if (is_if(e)) {
    // TODO
    rcc_warn("ExprTreeBuilder: if expressions not yet implemented");
    throw AnalysisException();
  } else if (is_for(e)) {
    // TODO
    rcc_warn("ExprTreeBuilder: for expressions not yet implemented");
    throw AnalysisException();
  } else if (is_while(e)) {
    // TODO
    rcc_warn("ExprTreeBuilder: while expressions not yet implemented");
    throw AnalysisException();
  } else if (is_repeat(e)) {
    // TODO
    rcc_warn("ExprTreeBuilder: repeat expressions not yet implemented");
    throw AnalysisException();
  } else if (is_curly_list(e)) {
    // TODO
    rcc_warn("ExprTreeBuilder: curly-brace lists not yet implemented");
    throw AnalysisException();
  } else if (is_call(e)) {  // regular function call
    OA_ptr<ExprTree::CallNode> call; call = new ExprTree::CallNode(make_call_h(e));
    tree->addNode(call);
    OA_ptr<ExprTree> lhs = build(call_lhs(e));
    tree->copyAndConnectSubTree(call, lhs);
    for(SEXP arg = call_args(e); arg != R_NilValue; arg = CDR(arg)) {
      OA_ptr<ExprTree> arg_tree = build(CAR(arg));
      tree->copyAndConnectSubTree(call, arg_tree);
    }
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

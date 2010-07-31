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
#include <analysis/Analyst.h>
#include <analysis/HandleInterface.h>
#include <analysis/MemRefExprInterface.h>
#include <analysis/RccBasicInterface.h>
#include <analysis/Utils.h>

#include <support/Debug.h>
#include <support/RccError.h>

#include "ExprTreeBuilder.h"

using namespace OA;
using namespace HandleInterface;

static bool debug;

// private constructor for singleton
ExprTreeBuilder::ExprTreeBuilder() {
  RCC_DEBUG("RCC_ExprTree", debug);
}

OA_ptr<ExprTree> ExprTreeBuilder::build_c(SEXP cell) {
  SEXP e = CAR(cell);
  OA_ptr<ExprTree> tree; tree = new ExprTree();

  if (debug) std::cout << "Begin ExprTreeBuilder::build_c" << std::endl;

  if (e == R_MissingArg) {
    // like consts. Will this work?
    OA_ptr<ConstValBasicInterface> val = RccBasicInterface::make_const_val(e);
    OA_ptr<ExprTree::ConstValNode> n; n = new ExprTree::ConstValNode(val);
    tree->addNode(n);
  } else if (is_const(e)) {
    //    OA_ptr<ExprTree::ConstValNode> n; n = new ExprTree::ConstValNode(make_const_val_h(e));
    OA_ptr<ConstValBasicInterface> val = RccBasicInterface::make_const_val(e);
    OA_ptr<ExprTree::ConstValNode> n; n = new ExprTree::ConstValNode(val);
    tree->addNode(n);
  } else if (is_var(e)) {
    if (debug) std::cout << "ExprTreeBuilder: building var" << std::endl;
    // TODO: is MemRefNode/MemRefHandle right?
    // Note: a ConstSymHandle doesn't belong here; that represents a constant bound to a symbol
    //    OA_ptr<ExprTree::MemRefNode> n; n = new ExprTree::MemRefNode(make_mem_ref_h(e));
    OA_ptr<ExprTree::MemRefNode> n; n = new ExprTree::MemRefNode(MemRefExprInterface::convert_sexp_c(cell));
    tree->addNode(n);
  } else if (is_assign(e)) {
    //    OA_ptr<ExprTree::OpNode> assign; assign = new ExprTree::OpNode(make_op_h(e));
    OA_ptr<OpBasicInterface> op = RccBasicInterface::make_op(e);
    OA_ptr<ExprTree::OpNode> assign; assign = new ExprTree::OpNode(op);
    OA_ptr<ExprTree> lhs = build_c(assign_lhs_c(e));
    OA_ptr<ExprTree> rhs = build_c(assign_rhs_c(e));
    tree->addNode(assign);
    tree->copyAndConnectSubTree(assign, lhs);
    tree->copyAndConnectSubTree(assign, rhs);
  } else if (is_fundef(e)) {
    // TODO
    throw AnalysisException("ExprTreeBuilder: fundefs not yet implemented");
  } else if (is_struct_field(e)) {
    if (debug) std::cout << "ExprTreeBuilder: building structure field" << std::endl;
    OA_ptr<OpBasicInterface> op = RccBasicInterface::make_op(e);
    OA_ptr<ExprTree::OpNode> expr; expr = new ExprTree::OpNode(op);
    OA_ptr<ExprTree> lhs; lhs = build_c(struct_field_lhs_c(e));
    tree->addNode(expr);
    tree->copyAndConnectSubTree(expr, lhs);
  } else if (is_subscript(e)) {
    if (debug) std::cout << "ExprTreeBuilder: building subscript" << std::endl;
    OA_ptr<IRHandlesIRInterface> iface; iface = R_Analyst::get_instance()->get_interface();
    // TODO: what about more than one subscript?
    if (debug) {
      std::cout << "ExprTreeBuilder: building subscript ";
      Rf_PrintValue(e);
    }
    OA_ptr<OpBasicInterface> op; op = RccBasicInterface::make_op(e);
    OA_ptr<ExprTree::OpNode> bracket; bracket = new ExprTree::OpNode(op);
    if (debug) {
      std::cout << "adding node ";
      bracket->dump(std::cout, iface);
    }
    tree->addNode(bracket);

    OA_ptr<ExprTree> lhs = build_c(subscript_lhs_c(e));
    if (debug) {
      std::cout << "copying and connecting ";
      lhs->dump(std::cout, iface);
    }
    tree->copyAndConnectSubTree(bracket, lhs);
    OA_ptr<ExprTree> rhs = build_c(subscript_first_sub_c(e));
    if (debug) {
      std::cout << "tree = "; tree->dump(std::cout, iface);
      std::cout << "copying and connecting "; rhs->dump(std::cout, iface);
    }
    tree->copyAndConnectSubTree(bracket, rhs);
  } else if (is_if(e)) {
    // TODO
    throw AnalysisException("ExprTreeBuilder: if expressions not yet implemented");
  } else if (is_for(e)) {
    // TODO
    throw AnalysisException("ExprTreeBuilder: for expressions not yet implemented");
  } else if (is_while(e)) {
    // TODO
    throw AnalysisException("ExprTreeBuilder: while expressions not yet implemented");
  } else if (is_repeat(e)) {
    // TODO
    throw AnalysisException("ExprTreeBuilder: repeat expressions not yet implemented");
  } else if (is_curly_list(e)) {
    // TODO
    throw AnalysisException("ExprTreeBuilder: curly-brace lists not yet implemented");
  } else if (is_call(e)) {  // regular function call
    if (debug) std::cout << "ExprTreeBuilder: function call" << std::endl;
    OA_ptr<ExprTree::CallNode> call; call = new ExprTree::CallNode(make_call_h(e));
    tree->addNode(call);
  } else {
    assert(0);
  }
  if (debug) std::cout << "End ExprTreeBuilder::build call." << std::endl;
  return tree;
}

// singleton pattern

ExprTreeBuilder * ExprTreeBuilder::get_instance() {
  if (s_instance == 0) {
    s_instance = new ExprTreeBuilder();
  }
  return s_instance;
}

// static member
ExprTreeBuilder * ExprTreeBuilder::s_instance = 0;

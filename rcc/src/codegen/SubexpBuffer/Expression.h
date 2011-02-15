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

// File: Expression.h
//
// A structure representing a subexpression in C code, including a
// variable name and other information. The op_ functions produce an
// Expression as output.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef RCC_EXPRESSION_H
#define RCC_EXPRESSION_H

#include <string>

#include <Dependence.h>
#include <Visibility.h>

///  Expression is a struct returned by the op_ functions representing a
///  subexpression in the output.
///  var = the name of the variable storing the expression
///  dependence = whether the expression depends on the current
///               environment. If false, the expression can be hoisted out
///               of an f-function.
///  visibility = whether the expression should be printed if it
///               appears at top level in R.
///  is_alloc = whether the expression is locally allocated.
///  del_text = code to clean up after the final use of the
///             expression. Most commonly a call to UNPROTECT_PTR.
struct Expression {
  Expression(std::string v, DependenceType d, VisibilityType vis, std::string dt)
    : var(v), dependence(d), visibility(vis), is_alloc(false), del_text(dt)
  { }
  Expression() {}
  std::string var;
  DependenceType dependence;
  VisibilityType visibility;
  bool is_alloc;
  std::string del_text;
  static const Expression bogus_exp;
  static const Expression nil_exp;
};

#endif

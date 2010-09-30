// -*- Mode: C++ -*-
//
// Copyright (c) 2010 Rice University
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

// File: SexpTraversal.h
//
// Traverses an SEXP; fills in both ExpressionInfoAnnotationMap and
// BasicVarAnnotationMap.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef SEXP_TRAVERSAL_H
#define SEXP_TRAVERSAL_H

#include <analysis/ExpressionInfo.h>

namespace RAnnot {

enum BasicVar::MayMustT;
  
class SexpTraversal {
public:
  static SexpTraversal * instance();

  static ExpressionInfo * make_expression_info(const SEXP & k);

private:
  explicit SexpTraversal();

private:
  enum LhsType {
    IN_LOCAL_ASSIGN,
    IN_FREE_ASSIGN
  };

  static void build_ud_rhs(ExpressionInfo * ei,
			   const SEXP cell,
			   BasicVar::MayMustT may_must_type,
			   bool is_stmt);  
  static void build_ud_lhs(ExpressionInfo * ei,
			   const SEXP cell,
			   const SEXP rhs_c,
			   BasicVar::MayMustT may_must_type,
			   LhsType lhs_type);

  static void make_use_var(ExpressionInfo * ei,
			   SEXP cell,
			   UseVar::PositionT pos,
			   BasicVar::MayMustT mmt,
			   Locality::LocalityType lt);
  static void make_def_var(ExpressionInfo * ei,
			   SEXP cell,
			   DefVar::SourceT source,
			   BasicVar::MayMustT mmt,
			   Locality::LocalityType lt,
			   SEXP rhs_c);
private:
  static SexpTraversal * s_instance;
};

}  // end namespace RAnnot

#endif

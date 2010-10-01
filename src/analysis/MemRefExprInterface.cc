// -*- Mode: C++ -*-
//
// Copyright (c) 2009 Rice University
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

// File: MemRefExprInterface.cc
//
// Interface for creating OA MemRefExprs out of SEXPs.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "MemRefExprInterface.h"

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/LexicalScope.h>
#include <analysis/ScopeAnnotationMap.h>
#include <analysis/SymbolTableFacade.h>
#include <analysis/Utils.h>
#include <analysis/VarInfo.h>

#include <support/RccError.h>
#include <support/StringUtils.h>

using namespace OA;
using namespace HandleInterface;
using namespace RAnnot;

OA_ptr<MemRefExpr> MemRefExprInterface::convert_sexp_c(SEXP cell) {
  OA_ptr<MemRefExpr> mre;
  assert(CAR(cell) != R_NilValue);
  if (is_var(CAR(cell))) {
    // TODO: make this easier
    FuncInfo * fi = dynamic_cast<FuncInfo *>(ScopeAnnotationMap::instance()->get(cell));
    VarInfo * vi = SymbolTableFacade::instance()->find_entry(cell);
    mre = MemRefExprInterface::convert_sym_handle(make_sym_h(vi));
    //  } else if (is_const(CAR(cell))) {
    //    mre = new ConstVarRef(make_const_h(cell));
  } else if (is_call(CAR(cell))) {
    mre = new UnnamedRef(MemRefExpr::USE, make_expr_h(cell));
    // TODO: if ref is strictly local, do this:
    // mre = UnnamedRef(MemRefExpr::USE, make_expr_h(cell), proc_handle);
  } else {
    // TODO: handle more cases
    rcc_warn("convert_sexp_c: expression not handled: " + to_string(CAR(cell)));
    mre = new UnknownRef(MemRefExpr::USE);
  }
  return mre;
}

OA_ptr<MemRefExpr> MemRefExprInterface::convert_mem_ref_handle(MemRefHandle mrh) {
  return convert_sexp_c(make_sexp(mrh));
}

OA_ptr<MemRefExpr> MemRefExprInterface::convert_sym_handle(SymHandle sym) {
  assert(sym != SymHandle(0));
  RAnnot::VarInfo * vi = HandleInterface::make_var_info(sym);
  // NamedRef constructor wants to know if the location is global (can
  // be seen in the scope of other procedures) or local (limited to
  // this scope). We conservatively say that it can only be local if
  // the procedure has no children (because a child procedure might
  // refer to this one's "local" variables).
  // TODO: make sharper distinction
  bool strictly_local = (vi->has_scope() &&
			 !vi->get_scope()->has_children());
  OA_ptr<NamedRef> mre; mre = new NamedRef(MemRefExpr::USE, sym, strictly_local);
  return mre;
}

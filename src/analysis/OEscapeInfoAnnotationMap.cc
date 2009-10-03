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

// File: OEscapeInfoAnnotationMap.cc
//
// Maps expression SEXPs to information from object escape analysis.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/CFG/CFG.hpp>

#include <support/Debug.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/OEscapeInfo.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/PropertyHndl.h>
#include <analysis/SymbolTable.h>
#include <analysis/Var.h>
#include <analysis/VarInfo.h>
#include <analysis/VarBinding.h>

#include "OEscapeInfoAnnotationMap.h"

namespace RAnnot {

// ----- declarations for static functions -----

static bool assignment_escapes(SEXP e);
static std::vector<SEXP> implicit_return_exps(SEXP fundef);
static void accum_implicit_returns(SEXP fundef, std::list<SEXP> * returns);

// ----- type definitions for readability -----

typedef OEscapeInfoAnnotationMap::MyKeyT MyKeyT;
typedef OEscapeInfoAnnotationMap::MyMappedT MyMappedT;

// ----- constructor/destructor ----- 

OEscapeInfoAnnotationMap::OEscapeInfoAnnotationMap()
{
  // RCC_DEBUG("RCC_OEscapeInfoAnnotationMap", debug);
}

OEscapeInfoAnnotationMap::~OEscapeInfoAnnotationMap() {
  // owns OEscapeInfo annotations, so delete them in deconstructor
  std::map<MyKeyT, MyMappedT>::const_iterator iter;
  for(iter = get_map().begin(); iter != get_map().end(); ++iter) {
    delete(iter->second);
  }
}


// ----- computation -----

void OEscapeInfoAnnotationMap::compute() {
  FuncInfo * fi;
  Var * var;
  OA::OA_ptr<OA::CFG::Node> node;
  OA::StmtHandle stmt;

  // want: whether expression gets returned or assigned upward.
  //   if it's called, no change
  //   if it's passed to a function, get whether that arg escapes from the callee
  //   if it's returned, it escapes
  //   if it's the RHS of a non-local assignment, it escapes
  //   if an escaping expression points to it, it escapes

  // simplified for variable (that is, set of all objects bound to variable)
  //   if called, no change
  //   if passed to function, escapes if arg escapes from callee
  //   if returned, it escapes
  //   if RHS of non-local assignment, it escapes

  // collect escaping expressions:
  // * RHS of non-local assignments
  // * return values
  // * arguments that escape call sites via assignment (not return)
  //
  // for each escaping expression:
  // * add expression to map
  // * if call site, recur on args that escape via assignment or return

  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_NODE(fi, node) {
      NODE_FOR_EACH_STATEMENT(node, stmt) {
	SEXP e = CAR(HandleInterface::make_sexp(stmt));
	switch(TYPEOF(e)) {
	case NILSXP:
	case REALSXP:
	case STRSXP:
	case LGLSXP:
	case SYMSXP:
	  continue;
	case LISTSXP:
	  assert(0);   // I don't think this can happen.
	  break;
	case LANGSXP:
	  if (is_assign(e)) {
	    get_map()[assign_rhs_c(e)] = new OEscapeInfo(assignment_escapes(e));
	    // now propagate to components of RHS?
	  } else if (is_return(e)) {
	    get_map()[call_nth_arg_c(e,1)] = new OEscapeInfo(true);
	    // propagate?
	  } else {        // function call other than assignment or return
	    // conservative for now
	    get_map()[e] = new OEscapeInfo(true);
	    // propagate?
	  }
	  break;
	default:
	  assert(0);
	}
      }
    }
  }
}

#if 0
	  } else if (is_library(call_lhs(e))) {
	    CEscapeInfo * ei;
	    PRIMPOINTS(library_value(call_lhs(e)))
	      PRIMESCAPE(library_value(call_lhs(e)))
#endif

// We have an assignment X <- Y or X <<- Y. If X is a symbol, then it
// escapes if we have <<- and doesn't escape if we have <-. If X is a
// more complicated expression, it escapes if and only if the base
// symbol is nonlocal. Example: if the left side of the assignment is
// a[b][c]$foo, then it escapes iff a is nonlocal regardness of the
// kind of arrow.
bool assignment_escapes(SEXP e) {
  if (is_symbol(CAR(assign_lhs_c(e)))) {
    return is_free_assign(e);
  } else {
    SEXP sym_c = assign_lhs_c(e);
    do {
      if (is_struct_field(CAR(sym_c))) {
	sym_c = struct_field_lhs_c(CAR(sym_c));
      } else if (is_subscript(CAR(sym_c))) {
	sym_c = subscript_lhs_c(CAR(sym_c));
      } else if (TYPEOF(CAR(sym_c)) == LANGSXP) {  // e.g., dim(x) <- foo
	sym_c = call_nth_arg_c(CAR(sym_c), 1);
      } else {
	assert(0);
      }
    } while (!is_symbol(CAR(sym_c)));
    return getProperty(Var, CAR(sym_c))->getScopeType() == Locality::Locality_FREE;
  }
}

std::list<SEXP> accum_implicit_returns(SEXP fundef) {
  assert(is_fundef(fundef));
  std::list<SEXP> returns;
  accum_implicit_returns(fundef_body_c(fundef), &returns);
  return returns;
}

void accum_implicit_returns(SEXP cell, std::list<SEXP> * returns) {
  SEXP e = CAR(cell);
  if (is_curly_list(e)) {
    SEXP last_c = curly_body(e);
    while (CDR(last_c) != R_NilValue) {
      last_c = CDR(last_c);
    }
    accum_implicit_returns(last_c, returns);
  } else if (is_if(e)) {
    accum_implicit_returns(if_truebody_c(e), returns);
    accum_implicit_returns(if_falsebody_c(e), returns);
  } else if (is_loop(e)) {
    accum_implicit_returns(loop_body_c(e), returns);
  } else {
    returns->push_back(cell);
  }  
}

// ----- singleton pattern -----

OEscapeInfoAnnotationMap * OEscapeInfoAnnotationMap::get_instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT OEscapeInfoAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

// Create the singleton instance and register the map in PropertySet
// for getProperty
void OEscapeInfoAnnotationMap::create() {
  s_instance = new OEscapeInfoAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}

OEscapeInfoAnnotationMap * OEscapeInfoAnnotationMap::s_instance = 0;
PropertyHndlT OEscapeInfoAnnotationMap::s_handle = "OEscapeInfo";


} // end namespace RAnnot

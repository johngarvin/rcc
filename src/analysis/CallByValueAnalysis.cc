// -*- Mode: C++ -*-
//
// Copyright (c) 2008 Rice University
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

// File: CallByValueAnalysis.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/FuncInfo.h>
#include <analysis/SideEffect.h>
#include <analysis/SideEffectAnnotationMap.h>

#include "CallByValueAnalysis.h"

using namespace RAnnot;

void CallByValueAnalysis::perform_analysis() {
  // for each function
  FuncInfoIterator fii(R_Analyst::get_instance()->get_scope_tree_root());
  for(FuncInfo *fi; fii.IsValid(); fii++) {
    fi = fii.Current();
    FuncInfo::const_call_site_iterator csi;
    for(csi = fi->begin_call_sites(); csi != fi->end_call_sites(); ++csi) {
      for(int i = 1; i <= fi->get_num_args(); i++) {
	SideEffect * arg_side_effect = getProperty(SideEffect, fi->get_arg(i));
	VarInfo * vi = SymbolTableFacade::get_instance()->find_entry(fi, call_lhs(*csi));
      }
    }

      // for each actual arg
      //   get arg expression's side effect
      //   look up function name in symbol table to get FuncInfo of callee
      //   if callee is strict in corresponding formal, arg must remain CBN
      //   let pre_def and pre_use be sets of Vars
      //   for each expression in func body
      //     continue if reached by a debut (we're looking at pre-debut)
      //     add side effect def to pre_def
      //     add side effect use to pre_use
      //   compare arg def with pre_use
      //     if any members in common, must remain CBN
      //   compare arg use with pre_def
      //     if any members in common, must remain CBN
      //   compare arg def with arg def
      //     if any members in common, must remain CBN
      //   if none in common, arg may be CBV
  }
}

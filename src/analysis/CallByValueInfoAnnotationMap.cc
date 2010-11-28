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

// File: CallByValueInfoAnnotationMap.cc
//
// Maps each call site to CallByValueInfo with eager/lazy info for actual arguments.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <set>

#include <support/Debug.h>
#include <support/RccError.h>

#include <analysis/AnalysisResults.h>
#include <analysis/CallByValueInfo.h>
#include <analysis/CallByValueTraversal.h>
#include <analysis/ExpressionSideEffect.h>
#include <analysis/ExpressionSideEffectAnnotationMap.h>
#include <analysis/FormalArgInfo.h>
#include <analysis/FormalArgInfoAnnotationMap.h>
#include <analysis/FuncInfo.h>
#include <analysis/FuncInfoAnnotationMap.h>
#include <analysis/OACallGraphAnnotation.h>
#include <analysis/OACallGraphAnnotationMap.h>
#include <analysis/PreDebutSideEffect.h>
#include <analysis/ResolvedArgsAnnotationMap.h>
#include <analysis/ResolvedArgs.h>
#include <analysis/Settings.h>
#include <analysis/SimpleIterators.h>
#include <analysis/SymbolTableFacade.h>
#include <analysis/Utils.h>

#include "CallByValueInfoAnnotationMap.h"

namespace RAnnot {

// ----- static function -----

bool is_cbv_safe(FormalArgInfo * formal, SEXP actual_c);

static bool debug;

CallByValueInfoAnnotationMap::CallByValueInfoAnnotationMap() {
  RCC_DEBUG("RCC_CallByValue", debug);
}

CallByValueInfoAnnotationMap::~CallByValueInfoAnnotationMap() {
  delete_map_values();
}

CallByValueInfoAnnotationMap * CallByValueInfoAnnotationMap::instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT CallByValueInfoAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

CallByValueInfoAnnotationMap * CallByValueInfoAnnotationMap::s_instance = 0;
PropertyHndlT CallByValueInfoAnnotationMap::s_handle = "CallByValueInfo";

void CallByValueInfoAnnotationMap::create() {
  s_instance = new CallByValueInfoAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}

void CallByValueInfoAnnotationMap::compute() {
  CallByValueTraversal::instance();
}

} // end namespace RAnnot

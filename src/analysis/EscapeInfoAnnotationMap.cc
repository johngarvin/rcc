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

// File: EscapeInfoAnnotationMap.cc
//
// Maps expression SEXPs to information from escape analysis.
//
// Author: John Garvin (garvin@cs.rice.edu)

// This map has both variable mentions and function definitions as
// keys. A mention mapped to a true EscapeInfo means the variable
// escapes its scope. A fundef mapped to a true EscapeInfo means one
// or more variables in that scope escape.

#include <OpenAnalysis/CFG/CFG.hpp>

#include <support/Debug.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/EscapeInfo.h>
#include <analysis/FuncInfo.h>
#include <analysis/PropertyHndl.h>
#include <analysis/SymbolTable.h>
#include <analysis/Var.h>
#include <analysis/VarInfo.h>
#include <analysis/VarBinding.h>

#include "EscapeInfoAnnotationMap.h"

namespace RAnnot {

// ----- type definitions for readability -----

typedef EscapeInfoAnnotationMap::MyKeyT MyKeyT;
typedef EscapeInfoAnnotationMap::MyMappedT MyMappedT;

// ----- constructor/destructor ----- 

EscapeInfoAnnotationMap::EscapeInfoAnnotationMap()
{
  // RCC_DEBUG("RCC_EscapeInfoAnnotationMap", debug);
}

EscapeInfoAnnotationMap::~EscapeInfoAnnotationMap() {
  // owns EscapeInfo annotations, so delete them in deconstructor
  std::map<MyKeyT, MyMappedT>::const_iterator iter;
  for(iter = get_map().begin(); iter != get_map().end(); ++iter) {
    delete(iter->second);
  }
}


// ----- computation -----

void EscapeInfoAnnotationMap::compute() {
  FuncInfo * fi;
  Var * var;
  OA::OA_ptr<OA::CFG::Node> node;
  OA::StmtHandle stmt;

  // find whether each variable escapes
  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_MENTION(fi, var) {
      VarBinding * binding;
      SEXP mention_c = (*var)->getMention_c();
      binding = getProperty(VarBinding, mention_c);
      if (binding->is_single() && *(binding->begin()) == fi->get_scope()) {
	// cannot escape
	get_map()[mention_c] = new EscapeInfo(false);
      } else {
	// this assignment may cause the variable to escape
	get_map()[mention_c] = new EscapeInfo(true);
      }
    }
  }

  // find whether each procedure has any escaping variables in its scope
  FOR_EACH_PROC(fi) {
    if (!fi->has_children()) {
      get_map()[fi->get_sexp()] = new EscapeInfo(false);
    } else {
      EscapeInfo * annot = new EscapeInfo(false);
      SymbolTable * st = fi->get_scope()->get_symbol_table();
      for(SymbolTable::const_iterator sym = st->begin(); sym != st->end(); sym++) {
	for(VarInfo::const_iterator def = sym->second->begin_defs(); def != sym->second->end_defs(); def++) {
	  if ((*def)->getSourceType() == DefVar::DefVar_ASSIGN &&
	      get_map()[(*def)->getMention_c()] != 0 &&
	      dynamic_cast<EscapeInfo *>(get_map()[(*def)->getMention_c()])->may_escape())
	  {
	    annot->set_may_escape(true);
	  }
	}
      }
      // annotate fundef as escaping/nonescaping
      get_map()[fi->get_sexp()] = annot;
      // annotate each formal the same as its fundef
      for(SEXP args = fi->get_args(); args != R_NilValue; args = CDR(args)) {
	get_map()[args] = annot;
      }
    }
  }
}


// ----- singleton pattern -----

EscapeInfoAnnotationMap * EscapeInfoAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT EscapeInfoAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

// Create the singleton instance and register the map in PropertySet
// for getProperty
void EscapeInfoAnnotationMap::create() {
  m_instance = new EscapeInfoAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}

EscapeInfoAnnotationMap * EscapeInfoAnnotationMap::m_instance = 0;
PropertyHndlT EscapeInfoAnnotationMap::m_handle = "EscapeInfo";


} // end namespace RAnnot

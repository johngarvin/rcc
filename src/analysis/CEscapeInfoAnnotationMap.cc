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

// File: CEscapeInfoAnnotationMap.cc
//
// Maps fundef SEXPs to information from closure escape analysis.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/CFG/CFG.hpp>

#include <support/Debug.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/CEscapeInfo.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/PropertyHndl.h>
#include <analysis/SymbolTable.h>
#include <analysis/SymbolTableFacade.h>
#include <analysis/Var.h>
#include <analysis/VarAnnotationMap.h>
#include <analysis/VarInfo.h>
#include <analysis/VarBinding.h>

#include "CEscapeInfoAnnotationMap.h"

namespace RAnnot {

// ----- declarations for static functions -----

// ----- type definitions for readability -----

typedef CEscapeInfoAnnotationMap::MyKeyT MyKeyT;
typedef CEscapeInfoAnnotationMap::MyMappedT MyMappedT;

// ----- constructor/destructor ----- 

CEscapeInfoAnnotationMap::CEscapeInfoAnnotationMap()
{
  // RCC_DEBUG("RCC_CEscapeInfoAnnotationMap", debug);
}

CEscapeInfoAnnotationMap::~CEscapeInfoAnnotationMap() {
  // owns CEscapeInfo annotations, so delete them in deconstructor
  std::map<MyKeyT, MyMappedT>::const_iterator iter;
  for(iter = get_map().begin(); iter != get_map().end(); ++iter) {
    delete(iter->second);
  }
}


// ----- computation -----

// We want to know:
// * Is this fundef ever assigned non-locally?
// * Is this fundef ever passed as an argument to a library/procedure that escapes that argument?

// The common case: only one def, and all uses are in function position
void CEscapeInfoAnnotationMap::compute() {
  FuncInfo * fi;

  FOR_EACH_PROC(fi) {
    CEscapeInfo * annot = new CEscapeInfo(false);
    
    if (fi->has_children()) { // if no lexical children, this cannot escape

      // for now, get the first name assigned to the function if it exists.
      SEXP name = fi->get_first_name_c();
      // if no name (anonymous or the whole-program procedure), conservatively say true
      if (!VarAnnotationMap::get_instance()->is_valid(name)) {
	annot->set_may_escape(true);
      } else {
	SymbolTableFacade * symbol_table = SymbolTableFacade::get_instance();
	
	VarInfo * sym = symbol_table->find_entry(getProperty(Var, name));
	VarInfo::ConstUseIterator use_iter;
	for (use_iter = sym->begin_uses(); use_iter != sym->end_uses(); use_iter++) {
	  // if in arg position, then escape is true (conservative unless
	  // we have escape information on whether procedures escape their
	  // arguments)
	  if ((*use_iter)->getPositionType() == UseVar::UseVar_ARGUMENT) {
	    annot->set_may_escape(true);
	  }
	}
      }
    }
    get_map()[fi->get_sexp()] = annot;
  }
}

#if 0
void CEscapeInfoAnnotationMap::compute() {
  FuncInfo * fi;
  Var * var;

  // find whether each variable escapes
  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_MENTION(fi, var) {
      VarBinding * binding;
      SEXP mention_c = (*var)->getMention_c();
      binding = getProperty(VarBinding, mention_c);
      if (binding->is_single() && *(binding->begin()) == fi->get_scope()) {
	// cannot escape
	get_map()[mention_c] = new CEscapeInfo(false);
      } else {
	// this assignment may cause the variable to escape
	get_map()[mention_c] = new CEscapeInfo(true);
      }
    }
  }

  // find whether each procedure has any escaping variables in its scope
  FOR_EACH_PROC(fi) {
    if (!fi->has_children()) {
      get_map()[fi->get_sexp()] = new CEscapeInfo(false);
    } else {
      // for each def that defines a variable in this scope, if it may escape, then this fundef may escape
      CEscapeInfo * annot = new CEscapeInfo(false);
      SymbolTable * st = fi->get_scope()->get_symbol_table();
      for(SymbolTable::const_iterator sym = st->begin(); sym != st->end(); sym++) {
	for(VarInfo::const_iterator def = sym->second->begin_defs(); def != sym->second->end_defs(); def++) {
	  if ((*def)->getSourceType() == DefVar::DefVar_ASSIGN &&
	      get_map()[(*def)->getMention_c()] != 0 &&
	      dynamic_cast<CEscapeInfo *>(get_map()[(*def)->getMention_c()])->may_escape())
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
#endif

// ----- singleton pattern -----

CEscapeInfoAnnotationMap * CEscapeInfoAnnotationMap::get_instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT CEscapeInfoAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

// Create the singleton instance and register the map in PropertySet
// for getProperty
void CEscapeInfoAnnotationMap::create() {
  s_instance = new CEscapeInfoAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}

CEscapeInfoAnnotationMap * CEscapeInfoAnnotationMap::s_instance = 0;
PropertyHndlT CEscapeInfoAnnotationMap::s_handle = "CEscapeInfo";


} // end namespace RAnnot

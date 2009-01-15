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

// File: VarBindingAnnotationMap.h
//
// Maps each variable to a VarBinding that describes its binding
// scopes. Owns the values in its map and must delete them in destructor.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <vector>

#include <support/RccError.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/PropertySet.h>
#include <analysis/SymbolTable.h>
#include <analysis/Utils.h>
#include <analysis/Var.h>
#include <analysis/VarBinding.h>
#include <analysis/VarBindingAnnotationMap.h>
#include <analysis/VarInfo.h>

#include "VarBindingAnnotationMap.h"

using namespace RAnnot;
using namespace HandleInterface;

// ----- forward declarations of file-scope functions

static bool defined_local_in_scope(Var * v, FuncInfo * s);

namespace RAnnot {

// ----- typedefs for readability -----

typedef VarBindingAnnotationMap::MyKeyT MyKeyT;
typedef VarBindingAnnotationMap::MyMappedT MyMappedT;
typedef VarBindingAnnotationMap::iterator iterator;
typedef VarBindingAnnotationMap::const_iterator const_iterator;

// ----- constructor/destructor -----

VarBindingAnnotationMap::VarBindingAnnotationMap() {}

VarBindingAnnotationMap::~VarBindingAnnotationMap() {
  delete_map_values();
}

// ----- singleton pattern -----

VarBindingAnnotationMap * VarBindingAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT VarBindingAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

void VarBindingAnnotationMap::create() {
  m_instance = new VarBindingAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}

VarBindingAnnotationMap * VarBindingAnnotationMap::m_instance = 0;
PropertyHndlT VarBindingAnnotationMap::m_handle = "VarBinding";

// ----- computation -----

/// Each Var is bound in some subset of the sequence of ancestor
/// scopes of its containing function. Compute the subsequence of
/// scopes in which each Var is bound and add the (Var,sequence) pair
/// into the annotation map.
void VarBindingAnnotationMap::compute() {
  create_var_bindings();
  populate_symbol_tables();
}

  /*
   for each procedure in a top down traversal:
     for each formal:
       mention.add(localscope)
       localscope.add(formal.name)
     for each mention that is a def:
       if local ("<-") then:
         mention.add(localscope)
         localscope.add(mention.name)
       else if free ("<<-") then:
         for each scope s from parent up:
           if s.includes(mention.name), then:
             mention.add(s)
	     localscope.add(mention.name)
   for each procedure:
     for each mention that is a use:
       if local or bottom then:
         mention.add(localscope)
         localscope.add(mention.name)
       if bottom or free then
         for each scope s from parent up
           if s.includes(mention.name) then:
	   // s defines name locally or with some <<- def
             mention.add(s)
             localscope.add(mention.name)
   */

void VarBindingAnnotationMap::create_var_bindings() {
  FuncInfo * fi;
  std::set<std::pair<SEXP, LexicalScope *> > free_defs;

  // first pass: formals and local and free defs. Free defs depend on
  // local defs in lexical ancestors, so the procedure traversal
  // should be top-down.
  FOR_EACH_PROC(fi) {
    assert(fi != 0);

    // create VarBindings for formal args; each one has just one
    // scope, which is the current procedure
    for(int i = 1; i <= fi->get_num_args(); i++) {
      VarBinding * binding = new VarBinding();
      binding->insert(fi->get_scope());
      get_map()[fi->get_arg(i)] = binding;
    }

    PROC_FOR_EACH_MENTION(fi, mi) {
      Var * v = *mi;
      v = getProperty(Var, v->getMention_c());
      // TODO: should make sure we always get the data-flow-solved
      // version of the Var. Shouldn't have to loop through
      // getProperty!
      if (v->getUseDefType() == Var::Var_DEF) {
	VarBinding * scopes = new VarBinding();
	switch(v->getScopeType()) {
	case Locality::Locality_LOCAL:
	  // "<-" def, bound in current scope only
	  scopes->insert(fi->get_scope());
	  break;
	case Locality::Locality_FREE:
	  // "<<-" def, bound in parent or higher
	  // start at this scope's parent; iterate upward through ancestors
	  for (FuncInfo * a = fi->Parent(); a != 0; a = a->Parent()) {
	    if (defined_local_in_scope(v,a)) {
	      free_defs.insert(std::pair<SEXP, LexicalScope *>(v->getName(), a->get_scope()));
	      scopes->insert(a->get_scope());
	    }
	  }
	  break;
	case Locality::Locality_BOTTOM:
	  rcc_error("VarBindingAnnotationMap: found def of BOTTOM type");
	  break;
	}
	get_map()[v->getMention_c()] = scopes;
      }
    }
  }

  // second pass: local and free uses. Free uses can refer to scopes
  // defined by <<- defs anywhere in the program, hence the separate
  // pass.
  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_MENTION(fi, mi) {
      Var * v = *mi;
      if (v->getUseDefType() == Var::Var_USE) {
	VarBinding * scopes = new VarBinding();
	switch(v->getScopeType()) {
	case Locality::Locality_LOCAL:
	  scopes->insert(fi->get_scope());
	  break;
	case Locality::Locality_BOTTOM:
	  scopes->insert(fi->get_scope());
	  // FALLTHROUGH
	case Locality::Locality_FREE:
	  for (FuncInfo * a = fi->Parent(); a != 0; a = a->Parent()) {
	    if (defined_local_in_scope(v,a) ||
		free_defs.find(std::pair<SEXP, LexicalScope *>(v->getName(), a->get_scope())) != free_defs.end())
	      {
		scopes->insert(a->get_scope());
	      }
	  }
	}
	// for R internal names, add the library scope
	if (is_library(CAR(v->getMention_c()))) {
	  scopes->insert(R_Analyst::get_instance()->get_library_scope());
	}
	
	get_map()[v->getMention_c()] = scopes;
      }
    }
  }
}
  
#if 0
  // old version, wrong algorithm. Need more than one pass to handle
  // uses of names defined with <<-

void VarBindingAnnotationMap::create_var_bindings() {
  FuncInfo * fi;

  FOR_EACH_PROC(fi) {
    assert(fi != 0);

    // first, create VarBindings for formal args; each one has just
    // one scope, which is the current procedure
    for(int i = 1; i <= fi->get_num_args(); i++) {
      VarBinding * binding = new VarBinding();
      binding->insert(fi->get_scope());
      get_map()[fi->get_arg(i)] = binding;
    }
    
    // now create bindings for mentions in the function body
    PROC_FOR_EACH_MENTION(fi, mi) {
      Var * v = *mi;
      VarBinding * scopes = new VarBinding();
      switch(v->getScopeType()) {
      case Locality::Locality_LOCAL:
	// bound in current scope only
	scopes->insert(fi->get_scope());
	break;
      case Locality::Locality_BOTTOM:
	// bound in current scope and one or more ancestors
	scopes->insert(fi->get_scope());
	// FALLTHROUGH
      case Locality::Locality_FREE:
	// start at this scope's parent; iterate upward through ancestors
	for(FuncInfo * a = fi->Parent(); a != 0; a = a->Parent()) {
	  if (defined_local_in_scope(v,a)) {
	    scopes->insert(a->get_scope());
	  }
	}
	
	// for R internal names, add the library scope
	if (is_library(CAR(v->getMention_c()))) {
	  scopes->insert(R_Analyst::get_instance()->get_library_scope());
	}
	
	// double-arrow (<<-) definitions declare the name in the
	// global scope if it's not local in some middle scope. Just
	// check for a def; since we're here, we already know the
	// mention is non-local.
	//
	// TODO: in the latest version of R, this is no longer the
	// case. If we attach RCC to a later version, we need to
	// change this behavior.
	if (v->getUseDefType() == Var::Var_DEF && !scopes->is_global()) {
	  scopes->insert(R_Analyst::get_instance()->get_global_scope());
	}

	// record unbound names
	if (scopes->begin() == scopes->end()) {
	  scopes->insert(UnboundLexicalScope::get_instance());
	}

	break;
      default:
	rcc_error("Unknown locality type encountered");
	break;
      }
      // whether global or not...
      get_map()[v->getMention_c()] = scopes;
    }  // next mention
  }  // next function
}

#endif

void VarBindingAnnotationMap::populate_symbol_tables() {
  // for each (mention, VarBinding) pair in our map
  std::map<MyKeyT, MyMappedT>::const_iterator iter;
  for(iter = begin(); iter != end(); ++iter) {
    // TODO: refactor AnnotationMaps to avoid downcasting
    VarBinding * vb = dynamic_cast<VarBinding *>(iter->second);
    Var * var = getProperty(Var, iter->first);
    SEXP name = var->getName();
    // for each scope in the VarBinding
    VarBinding::const_iterator scope_iter;
    for(scope_iter = vb->begin(); scope_iter != vb->end(); ++scope_iter) {
      LexicalScope * scope = *scope_iter;
      // in the scope's symbol table, associate the name with a VarInfo
      SymbolTable * st = scope->get_symbol_table();
      VarInfo * vi = st->find_or_create(name, scope);
      // each VarInfo has a list of definitions. If this mention is a
      // def, add it to the appropriate VarInfo if it's not there
      // already.
      if(DefVar * def = dynamic_cast<DefVar *>(var)) {
	vi->insert_def(def);
      }
    }
    // if VarBinding is ambiguous (more than one scope), add it to the
    // special symbol table
    if (!vb->is_single() && !vb->is_unbound()) {
      (*SymbolTable::get_ambiguous_st())[name] = new VarInfo(name);
    }
  }
}
  
}  // end namespace RAnnot

/// Is the given variable defined as local in the given scope?
static bool defined_local_in_scope(Var * v, FuncInfo * s) {

  // look at formal args
  for(int i = 1; i <= s->get_num_args(); i++) { // args are indexed from 1
    Var * formal = getProperty(Var, s->get_arg(i));
    // all formal args are local and def
    if (formal->getName() == v->getName()) {
      return true;
    }
  }

  // look at mentions
  FuncInfo::mention_iterator mi;
  for(mi = s->begin_mentions(); mi != s->end_mentions(); ++mi) {
    Var * m = *mi;
    if (m->getUseDefType() == Var::Var_DEF  &&
	m->getScopeType() == Locality::Locality_LOCAL &&
	m->getName() == v->getName())
    {
      return true;
    }
  }

  // TODO: what about double-arrow defs in other scopes?
  return false;
}

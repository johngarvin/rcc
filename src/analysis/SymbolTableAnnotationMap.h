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

// File: SymbolTableAnnotationMap.h
//
// Maps each procedure to a symbol table annotation.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef SYMBOL_TABLE_ANNOTATION_MAP_H
#define SYMBOL_TABLE_ANNOTATION_MAP_H

#include <map>

#include <OpenAnalysis/CFG/CFGInterface.hpp>

#include <analysis/AnnotationMap.h>
#include <analysis/DefVar.h>
#include <analysis/PropertyHndl.h>

// ----- forward declarations -----

class R_IRInterface;

namespace RAnnot {

class SymbolTableAnnotationMap : public AnnotationMap {
public:
  // deconstructor
  virtual ~SymbolTableAnnotationMap();

  // demand-driven analysis
  MyMappedT & operator[](const MyKeyT & k); // FIXME: remove this when refactoring is done
  MyMappedT get(const MyKeyT & k);
  bool is_computed();

  // singleton
  static SymbolTableAnnotationMap * get_instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

  // add an entry to a symbol table
  void add_def(MyKeyT func, DefVar * def);

  // iterators
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

private:
  // private constructor for singleton pattern
  SymbolTableAnnotationMap(bool ownsAnnotations = true);

  void compute();
  void compute_all_syntactic_info();
  void compute_all_locality_info();
  void compute_locality_info(OA::OA_ptr<R_IRInterface> interface,
			     OA::ProcHandle proc,
			     OA::OA_ptr<OA::CFG::CFGInterface> cfg);

  bool m_computed; // has our information been computed yet?
  std::map<MyKeyT, MyMappedT> m_map;

  static SymbolTableAnnotationMap * m_instance;
  static PropertyHndlT m_handle;
  static void create();
};

}


#endif

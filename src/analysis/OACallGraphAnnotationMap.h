// -*- Mode: C++ -*-
//
// Copyright (c) 2007 Rice University
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

// File: OACallGraphAnnotationMap.h
//
// Represents the OpenAnalysis call graph of the program.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef OA_CALL_GRAPH_ANNOTATION_MAP
#define OA_CALL_GRAPH_ANNOTATION_MAP

#include <analysis/AnnotationMap.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class OACallGraphAnnotationMap : public AnnotationMap
{
public:
  // ----- destructor -----
  virtual ~OACallGraphAnnotationMap();

  // ----- demand-driven analysis -----

  MyMappedT & operator[](const MyKeyT & k); // TODO: remove this when refactoring is done

  /// given a call site, return the list of fundef/library nodes reachable; compute if necessary
  MyMappedT get(const MyKeyT & k);

  bool is_computed();

  // ----- implement singleton pattern -----

  static OACallGraphAnnotationMap * get_instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

  // ----- iterators -----

  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

  // ----- debugging -----

  /// dump debugging information about the call graph
  void dump(std::ostream & os);

  /// dump the call graph in dot form
  void dumpdot(std::ostream & os);

private:
  // ----- implement singleton pattern -----

  // private constructor for singleton pattern
  OACallGraphAnnotationMap();

  static OACallGraphAnnotationMap * m_instance;
  static PropertyHndlT m_handle;
  static void create();
  
  // ----- compute call graph -----

  void compute();

private:
  bool m_computed; // has our information been computed yet?
};

}

#endif

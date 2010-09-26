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

// File: PropertySet.h
//
// Associates arbitrary property names with AnnotationMaps.
//
// Author: John Mellor-Crummey (johnmc@cs.rice.edu)

#ifndef PROPERTY_SET_H
#define PROPERTY_SET_H

//************************* System Include Files ****************************

#include <iostream>
#include <map>

//**************************** R Include Files ******************************

//*************************** User Include Files ****************************

#include "PropertyHndl.h"
#include "AnnotationMap.h"

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RProp {

//****************************************************************************
// R Property Information
//****************************************************************************

// ---------------------------------------------------------------------------
// PropertySet: Associates arbitrary property names with
// AnnotationMaps.  For example, a property might be the result of an
// analysis pass over the R AST.
// ---------------------------------------------------------------------------
class PropertySet
  : public std::map<PropertyHndlT, RAnnot::AnnotationMap *>
{
public:
  // -------------------------------------------------------
  // constructor/destructor
  // -------------------------------------------------------
  explicit PropertySet();
  ~PropertySet();

  // -------------------------------------------------------
  // managing annotation maps
  // -------------------------------------------------------

  /// associate the SEXP s with annotation annot for the propertyName
  /// property. TODO: adding annotations from the outside like this
  /// is bad data hiding. It would be better to create Annotation
  /// objects only inside AnnotationMaps.
  void insert(PropertyHndlT propertyName, SEXP s,
	      RAnnot::AnnotationBase * annot);

  /// Look up an annotation associated with an SEXP.
  RAnnot::AnnotationBase * lookup(PropertyHndlT propertyName, SEXP s);

  /// To be called when new AnnotationMaps are created. Register a
  /// property name with an AnnotationMap so the Annotations in the
  /// AnnotationMap can be looked up.
  void add(PropertyHndlT propertyName, RAnnot::AnnotationMap * amap);

  // -------------------------------------------------------
  // cloning (proscribe by hiding copy constructor and operator=)
  // -------------------------------------------------------
  
  // -------------------------------------------------------
  // iterator, find/insert, etc 
  // -------------------------------------------------------
  // use inherited std::map routines

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  std::ostream & dump_cout() const;
  std::ostream & dump(std::ostream & os) const;

private:
  PropertySet(const PropertySet & x);
  PropertySet & operator=(const PropertySet & x) { return *this; }

private:
};


//****************************************************************************

} // end of RProp namespace

#endif

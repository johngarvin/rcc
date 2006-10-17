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

// File: PropertySetMgr.h
//
// (description)
//
// Author: John Mellor-Crummey (johnmc@cs.rice.edu)

#ifndef PROPERTY_SET_MGR_H
#define PROPERTY_SET_MGR_H

//************************* System Include Files ****************************

#include <iostream>

//**************************** R Include Files ******************************

//*************************** User Include Files ****************************

#include "PropertySet.h"

//*************************** Forward Declarations ***************************

class RAnnot::AnnotationMap;

//****************************************************************************

namespace RProp {

//****************************************************************************
// R Property Manager
//****************************************************************************

// ---------------------------------------------------------------------------
// PropertySetMgr: Demand-driven manager for PropertySets.
// ---------------------------------------------------------------------------
class PropertySetMgr
{
public:
  // Property names
  static PropertyHndlT Prop1;
  static PropertyHndlT Prop2;
  static PropertyHndlT Prop3;

  // function type for annotation-computation functions
  typedef RAnnot::AnnotationMap* (*AnnotationComputationFn_t)(SEXP);

  // FIXME: need some way of registering property names and
  // computation routines.
  
public:
  // -------------------------------------------------------
  // constructor/destructor
  // -------------------------------------------------------

  // create my own PropertySet
  PropertySetMgr();
  
  // assume control and ownership of 'x'
  PropertySetMgr(PropertySet* x);
  
  ~PropertySetMgr();

  // -------------------------------------------------------
  // cloning (proscribe by hiding copy constructor and operator=)
  // -------------------------------------------------------

  // -------------------------------------------------------
  // member data manipulation
  // -------------------------------------------------------

  // get annotations on demand
  RAnnot::AnnotationMap* getAnnot(PropertyHndlT prop);

  // property-set (returns non-const set to allow manipulation)
  PropertySet* getPropertySet() const
    { return mSet; }
  
  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  std::ostream& dumpCout() const;
  std::ostream& dump(std::ostream& os) const;

private:
  PropertySetMgr(const PropertySetMgr& x);
  PropertySetMgr& operator=(const PropertySetMgr& x) { return *this; }

private:
  PropertySet* mSet;
};


//****************************************************************************

} // end of RProp namespace

#endif

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

// File: PropertySet.cc
//
// Associates arbitrary property names with AnnotationMaps.
//
// Author: John Mellor-Crummey (johnmc@cs.rice.edu)

//************************* System Include Files ****************************

#include <assert.h>
#include <iostream>
#include <string>

//**************************** R Include Files ******************************

//*************************** User Include Files ****************************

#include <support/RccError.h>

#include "PropertySet.h"

using namespace RAnnot;

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RProp {

PropertySet::PropertySet()
{
}


PropertySet::~PropertySet()
{
  for (iterator it = this->begin(); it != this->end(); ++it) {
    delete it->second;
  }
  this->clear();
}


void PropertySet::insert(PropertyHndlT propertyName, SEXP s,
			 AnnotationBase *annot)
{
  AnnotationMap *annotations = (*this)[propertyName];
  if (annotations == 0) {
    rcc_error("Annotation map not found in PropertySet");
  }
  (*annotations)[s] = annot;
}

AnnotationBase *PropertySet::lookup(PropertyHndlT propertyName, SEXP s)
{
  AnnotationMap *annotations = (*this)[propertyName];
  if (annotations == 0) {
    rcc_error("AnnotationMap not found; possible invalid propertyName " + std::string(propertyName));
  }
  return annotations->get(s);
}

void PropertySet::add(PropertyHndlT propertyName, RAnnot::AnnotationMap * amap)
{
  (*this)[propertyName] = amap;
}


std::ostream & PropertySet::dump_cout() const
{
  dump(std::cout);
}


std::ostream & PropertySet::dump(std::ostream & os) const
{
  os << "{ PropertySet:\n";
  for (const_iterator it = this->begin(); it != this->end(); ++it) {
    os << "(" << it->first << " --> " << it->second << ")\n";
    it->second->dump(os);
  }
  os << "}\n";
  os.flush();
}


} // end of RProp namespace

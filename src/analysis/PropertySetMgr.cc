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

// File: PropertySetMgr.cc
//
// (description)
//
// Author: John Mellor-Crummey (johnmc@cs.rice.edu)

//************************* System Include Files ****************************

#include <iostream>
#include <string>
#include <assert.h>

//**************************** R Include Files ******************************

//*************************** User Include Files ****************************

#include "PropertySetMgr.h"

//*************************** Forward Declarations ***************************

//****************************************************************************
#if 0

namespace RProp {

PropertySetMgr::PropertySetMgr()
{
  mSet = new PropertySet;
}


PropertySetMgr::PropertySetMgr(PropertySet * x)
  : mSet(x)
{
}


PropertySetMgr::~PropertySetMgr()
{
  delete mSet;
}


RAnnot::AnnotationMap * PropertySetMgr::getAnnot(PropertyHndlT prop) 
{  
  RAnnot::AnnotationMap * x = NULL;

  PropertySet::iterator it = mSet->find(prop);
  if (it == mSet->end()) {
    std::pair<PropertySet::iterator, bool> res = 
      mSet->insert(make_pair(prop, x));
    assert(!res.second && "insert() must be successful after find()");
  }
  else {
    // Property has already been computed.
    x = (*it).second;
  }

  return x;
}


std::ostream & PropertySetMgr::dump_cout() const
{
  dump(std::cout);
}


std::ostream & PropertySetMgr::dump(std::ostream & os) const
{
  os << "{ PropertySetMgr:\n";
  mSet->dump(os);
  os << "}\n";
  os.flush();
}


} // end of RProp namespace
#endif

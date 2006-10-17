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

// File: FormalArgInfo.cc
//
// Annotation for a formal argument of a function.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <ostream>

#include <support/DumpMacros.h>

#include <analysis/FormalArgInfoAnnotationMap.h>
#include <analysis/PropertyHndl.h>

#include "FormalArgInfo.h"

namespace RAnnot {

//****************************************************************************
// FormalArgInfo
//****************************************************************************

FormalArgInfo::FormalArgInfo() 
{
  isvalue = false;
}


FormalArgInfo::~FormalArgInfo()
{
}


std::ostream& FormalArgInfo::dump(std::ostream& os) const
{
  beginObjDump(os, FormalArgInfo);
  // FIXME: add implementation
  endObjDump(os, FormalArgInfo);
}

PropertyHndlT FormalArgInfo::handle() {
  return FormalArgInfoAnnotationMap::handle();
}

}

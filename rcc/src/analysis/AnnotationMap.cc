// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/AnnotationMap.cc,v 1.4 2007/07/01 02:04:12 garvin Exp $

// * BeginCopyright *********************************************************
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
// *********************************************************** EndCopyright *

// File: AnnotationMap.cc
//
// Base class for annotation maps, which map entities in the program
// to annotations.
//
// Author: John Garvin (garvin@cs.rice.edu)

//************************* System Include Files ****************************

#include <iostream>
#include <string>

//**************************** R Include Files ******************************

//*************************** User Include Files ****************************

#include "AnnotationMap.h"

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RAnnot {

AnnotationMap::AnnotationMap()
{
}

AnnotationMap::~AnnotationMap()
{
}

std::ostream & AnnotationMap::dump_cout() const
{
  dump(std::cout);
}

} // end of RAnnot namespace

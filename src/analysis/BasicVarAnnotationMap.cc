// -*- Mode: C++ -*-
//
// Copyright (c) 2010 Rice University
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

// File: BasicVarAnnotationMap.cc
//
// Set of BasicVar annotations, representing basic variable information,
// including the methods for computing the information. Maps a cons
// cell containing a SYMSXP to a BasicVar annotation.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/CFG/CFG.hpp>

#include <support/Debug.h>
#include <support/RccError.h>

#include <analysis/AnalysisResults.h>
#include <analysis/BasicVar.h>
#include <analysis/HandleInterface.h>
#include <analysis/PropertySet.h>
#include <analysis/SexpTraversal.h>

#include "BasicVarAnnotationMap.h"

using namespace OA;
using namespace HandleInterface;

typedef CFG::CFGInterface MyCFG;

static bool debug;

namespace RAnnot {
  
// ----- type definitions for readability -----

typedef BasicVarAnnotationMap::MyKeyT MyKeyT;
typedef BasicVarAnnotationMap::MyMappedT MyMappedT;
typedef BasicVarAnnotationMap::iterator iterator;
typedef BasicVarAnnotationMap::const_iterator const_iterator;

//  ----- constructor/destructor ----- 
  
BasicVarAnnotationMap::BasicVarAnnotationMap()
{
  RCC_DEBUG("RCC_BasicVarAnnotationMap", debug);
}
  
BasicVarAnnotationMap::~BasicVarAnnotationMap()
{}


// ----- singleton pattern -----

BasicVarAnnotationMap * BasicVarAnnotationMap::instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT BasicVarAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

void BasicVarAnnotationMap::create() {
  s_instance = new BasicVarAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}

BasicVarAnnotationMap * BasicVarAnnotationMap::s_instance = 0;
PropertyHndlT BasicVarAnnotationMap::s_handle = "BasicVar";

// ----- computation -----

// compute all BasicVar annotation information
void BasicVarAnnotationMap::compute() {
  SexpTraversal::instance();
}

} // end namespace RAnnot

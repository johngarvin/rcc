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

// File: ExpressionInfoAnnotationMap.cc
//
// Maps expression SEXPs to ExpressionInfo annotations.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <map>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>

#include <support/Debug.h>
#include <support/RccError.h>

#include <analysis/Analyst.h>
#include <analysis/AnalysisResults.h>
#include <analysis/BasicFuncInfo.h>
#include <analysis/BasicFuncInfoAnnotationMap.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/PropertySet.h>
#include <analysis/SexpTraversal.h>

#include "ExpressionInfoAnnotationMap.h"

static bool debug;

using namespace HandleInterface;

namespace RAnnot {

// type definitions for readability
typedef ExpressionInfoAnnotationMap::MyKeyT MyKeyT;
typedef ExpressionInfoAnnotationMap::MyMappedT MyMappedT;
typedef ExpressionInfoAnnotationMap::iterator iterator;
typedef ExpressionInfoAnnotationMap::const_iterator const_iterator;

// ----- constructor/destructor ----- 

ExpressionInfoAnnotationMap::ExpressionInfoAnnotationMap()
{
  RCC_DEBUG("RCC_ExpressionInfoAnnotationMap", debug);
}

ExpressionInfoAnnotationMap::~ExpressionInfoAnnotationMap() {
  std::map<MyKeyT, MyMappedT>::const_iterator iter;
  for(iter = get_map().begin(); iter != get_map().end(); ++iter) {
    delete(iter->second);
  }
}

// ----- computation -----

void ExpressionInfoAnnotationMap::compute() {
  SexpTraversal::instance();  // creates ExpressionInfos, fills in map
}


// ----- singleton pattern -----

ExpressionInfoAnnotationMap * ExpressionInfoAnnotationMap::instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT ExpressionInfoAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

// Create the singleton instance and register the map in PropertySet
// for getProperty
void ExpressionInfoAnnotationMap::create() {
  s_instance = new ExpressionInfoAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}

ExpressionInfoAnnotationMap * ExpressionInfoAnnotationMap::s_instance = 0;
PropertyHndlT ExpressionInfoAnnotationMap::s_handle = "ExpressionInfo";

}

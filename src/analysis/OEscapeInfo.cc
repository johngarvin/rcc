// -*- Mode: C++ -*-
//
// Copyright (c) 2009 Rice University
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

// File: OEscapeInfo.cc
//
// Annotation for information coming from object escape analysis.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/OEscapeInfoAnnotationMap.h>

#include <support/RccError.h>

#include "OEscapeInfo.h"

namespace RAnnot {

OEscapeInfo::OEscapeInfo(bool x) : m_may_escape(x) {}
OEscapeInfo::~OEscapeInfo() {}
  
bool OEscapeInfo::may_escape() {
  return m_may_escape;
}

void OEscapeInfo::set_may_escape(bool x) {
  m_may_escape = x;
}

AnnotationBase * OEscapeInfo::clone() {
  return 0;
}

std::ostream & OEscapeInfo::dump(std::ostream &) const {
  // TODO
}

PropertyHndlT OEscapeInfo::handle() {
  return OEscapeInfoAnnotationMap::handle();
}
  
}

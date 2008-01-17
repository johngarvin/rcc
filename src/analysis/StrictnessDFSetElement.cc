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

// File: StrictnessDFSetElement.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "StrictnessDFSetElement.h"

using namespace OA;

// constructors

namespace Strictness {

DFSetElement::DFSetElement(OA_ptr<R_VarRef> _loc, StrictnessType _type)
  : m_loc(_loc), m_type(_type)
  {}

DFSetElement::DFSetElement(const DFSetElement& other)
  : m_loc(other.m_loc), m_type(other.m_type)
  {}

// access

OA_ptr<R_VarRef> DFSetElement::get_loc() const { return m_loc; }
StrictnessType DFSetElement::get_strictness_type() const { return m_type; }


/// not doing a deep copy
OA_ptr<DFSetElement> DFSetElement::clone() { 
  OA_ptr<DFSetElement> retval;
  retval = new DFSetElement(*this);
  return retval;
}
  
/// copy an DFSetElement, not a deep copy, will refer to same Location
/// as other
DFSetElement& DFSetElement::operator=(const DFSetElement& other) {
  m_loc = other.get_loc();
  m_type = other.get_strictness_type();
  return *this;
}

/// Equality operator for DFSetElement.  Just inspects location contents.
bool DFSetElement::operator== (const DFSetElement &other) const {
  return (m_loc==other.get_loc());
}

/// Inequality operator.
bool DFSetElement::operator!= (const DFSetElement &other) const {
  return !(*this==other);
}


/// Just based on location, this way when insert a new DFSetElement it can
/// override the existing DFSetElement with same location
bool DFSetElement::operator< (const DFSetElement &other) const { 
  return (m_loc < other.get_loc());
}

/// Equality method for DFSetElement.
bool DFSetElement::equiv(const DFSetElement& other) {
  return (m_loc == other.get_loc() && m_type == other.get_strictness_type());
}

bool DFSetElement::sameLoc (const DFSetElement &other) const {
  return (m_loc == other.get_loc());
}

std::string DFSetElement::toString(OA_ptr<IRHandlesIRInterface> pIR) {
  return toString();
}

/// Return a string that contains a character representation of
/// an DFSetElement.
std::string DFSetElement::toString() {
  std::ostringstream oss;
  oss << "<";
  oss << m_loc->toString();
  oss << ",";
  oss << typeName(m_type);
  oss << ">";
  return oss.str();
}

} // namespace Strictness

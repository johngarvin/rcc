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

// File: StrictnessDFSetElement.h
//
// Associates an R_VarRef with a boolean. This is the unit that
// DFSet (which implements OA's DataFlowSet) contains.
// Modeled after ConstDef in ReachConstsStandard.hpp.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef STRICTNESS_DF_SET_ELEMENT_H
#define STRICTNESS_DF_SET_ELEMENT_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>

#include <analysis/VarRef.h>

namespace Strictness {

/// Associates an R_VarRef with a boolean. This is the unit that
/// DFSet (which implements OA's DataFlowSet) contains.
/// Modeled after ConstDef in ReachConstsStandard.hpp.
class DFSetElement {
public:
  // constructors
  DFSetElement(OA::OA_ptr<R_VarRef> _loc, bool _strict);
  DFSetElement(const DFSetElement& other);

  // access
  OA::OA_ptr<R_VarRef> get_loc() const;
  LocalityType get_locality_type() const;

  // relationships
  DFSetElement& operator= (const DFSetElement& other);

  /// not doing a deep copy
  OA::OA_ptr<DFSetElement> clone();
    
  /// operator== just compares content of loc
  bool operator== (const DFSetElement &other) const;
  /// method equiv compares all parts of DFSetElement as appropriate
  bool equiv(const DFSetElement& other);
  
  bool operator!= (const DFSetElement &other) const;
  bool operator< (const DFSetElement &other) const;
  bool sameLoc (const DFSetElement &other) const;

  std::string toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir);
  std::string toString();

private:
  OA::OA_ptr<R_VarRef> m_loc;
  LocalityType m_type;
};

} // namespace Strictness

#endif

// -*- Mode: C++ -*-
//
// Copyright (c) 2008 Rice University
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

// File: StrictnessType.h
//
// Enumerated type for strictness data flow solver. These form a lattice:
//    TOP
//   |   \
//   |   USED
//   |   /
//  KILLED
//
// TOP: analysis has not yet determined
// USED: must have been used before defined (lazy arg must have evaluated)
// KILLED: may have been killed before used (lazy arg may have been lost)

// Author: John Garvin (garvin@cs.rice.edu)

#ifndef STRICTNESS_TYPE_H
#define STRICTNESS_TYPE_H

#include <string>

enum StrictnessType {
  Strictness_TOP,
  Strictness_USED,
  Strictness_KILLED
};

/// Return the name of the type as a string: "TOP", "USED", etc.
const std::string typeName(StrictnessType x);

#endif

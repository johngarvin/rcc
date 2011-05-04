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

// File: StrictnessType.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "StrictnessType.h"

namespace Strictness {

const std::string type_name(StrictnessType x) {
  switch(x) {
  case Strictness_TOP: return "TOP";
  case Strictness_USED: return "USED";
  case Strictness_KILLED: return "KILLED";
  }
}

}  // end namespace Strictness
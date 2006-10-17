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

// File: RccError.cc
//
// Print errors and warnings.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "RccError.h"

#include <iostream>

void rcc_error(std::string message) {
  std::cerr << "Error: " << message << std::endl;
  exit(1);
}

void rcc_warn(std::string message) {
  std::cerr << "Warning: " << message << std::endl;
}

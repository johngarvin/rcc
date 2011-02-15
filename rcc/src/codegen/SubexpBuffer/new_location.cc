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

// File: new_location.cc
//
// Produce and return a new location (a slot in an environment)
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <GetName.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

/// Return a new location (pointer to a slot in the environment).
string SubexpBuffer::new_location() {
  string location = ParseInfo::global_constants->new_var_unp();
  string decl = "static R_varloc_t " + location + ";\n";
  ParseInfo::global_constants->append_decls(decl);
  return location;
}

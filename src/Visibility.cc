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

// File: Visibility.cc
//
// Functions to emit code related to visibility of R objects (i.e.,
// whether a value should be printed when a line in the main program
// evaluates to it).
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <Visibility.h>
#include <CodeGenUtils.h>

using namespace std;

const string Visibility::R_VISIBILITY_GLOBAL_VAR = "R_Visible";

string Visibility::emit_check_expn()
{
  string vstring = R_VISIBILITY_GLOBAL_VAR;
  return vstring;
}

string Visibility::emit_set_if_visible(VisibilityType vis)
{
  string vstring;
  if (vis == VISIBLE) vstring = emit_set(vis);
  return vstring;
}

string Visibility::emit_set(VisibilityType vis)
{
  string vstring;
  if (vis != CHECK_VISIBLE) {
    vstring += emit_assign(R_VISIBILITY_GLOBAL_VAR, 
			   (vis == INVISIBLE ? "0" : "1"));
  }
  return vstring;
}

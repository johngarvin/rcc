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

// File: Visiblilty.h
//
// Functions to emit code related to visibility of R objects (i.e.,
// whether a value should be printed when a line in the main program
// evaluates to it).
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef VISIBILITY_H
#define VISIBILITY_H

#include <string>

typedef enum {INVISIBLE, VISIBLE, CHECK_VISIBLE} VisibilityType;

class Visibility {
public:
  static std::string emit_set(VisibilityType vis);
  static std::string emit_set_if_visible(VisibilityType vis);
  static std::string emit_check_expn();
private:
  static const std::string R_VISIBILITY_GLOBAL_VAR;
};

#endif

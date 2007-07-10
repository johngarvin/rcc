// -*- Mode: C++ -*-

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

// Copyright (c) 2003-2007 Rice University
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

// File: StringUtils.cc
//
// Utilities for handling strings of C code.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <include/R/R_RInternals.h>

std::string to_string(SEXP e);
std::string make_symbol(SEXP e);
std::string make_type(int t);
std::string indent(std::string str);
std::string i_to_s(const int i);
std::string d_to_s(double d);
std::string c_to_s(Rcomplex c);
std::string escape(std::string str);
std::string make_c_id(std::string s);
std::string quote(std::string str);
std::string unp(std::string str);
std::string strip_suffix(std::string str);
int filename_pos(std::string str);

#endif

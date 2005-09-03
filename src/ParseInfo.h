// -*- Mode: C++ -*-
// Copyright (c) 2005 Rice University
//
// August 29, 2005
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
///

#ifndef PARSE_INFO_H
#define PARSE_INFO_H

#include <map>
#include <string>
#include <list>

// forward declarations
class SubexpBuffer;
class SplitSubexpBuffer;

//! ParseInfo:
//! collection of global (whole-program) information collected during code generation
// TODO: improve encapsulation
class ParseInfo {
public:

  // constructor
  ParseInfo() {}

  static std::map<std::string, std::string> func_map;
  static std::map<std::string, std::string> symbol_map;
  static std::map<std::string, std::string> string_map;
  static std::map<double, std::string> sc_real_map;
  static std::map<int, std::string> sc_logical_map;
  static std::map<int, std::string> sc_integer_map;
  static std::map<int, std::string> primsxp_map;
  static std::list<std::string> direct_funcs;
  static SubexpBuffer * global_fundefs;
  static SplitSubexpBuffer * global_constants;
  static SubexpBuffer * global_labels;
  static void flag_problem();
  static bool get_problem_flag();
  static bool is_direct(std::string func);

private:
  static bool problem_flag;
};

#endif

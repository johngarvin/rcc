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
#include <set>

// forward declarations
class SubexpBuffer;
class SplitSubexpBuffer;

//! ParseInfo:
//! collection of global (whole-program) information collected during code generation
// TODO: improve encapsulation
class ParseInfo {
public:

  static SubexpBuffer * global_fundefs;
  static SplitSubexpBuffer * global_constants;
  static SubexpBuffer * global_labels;

  static void flag_problem();
  static bool get_problem_flag();
  static bool is_direct(std::string func);

  static void set_allow_oo(bool x);
  static bool allow_oo();

  static void set_allow_envir_manip(bool x);
  static bool allow_envir_manip();
  
  static void set_allow_special_redef(bool x);
  static bool allow_special_redef();

  static void set_allow_builtin_redef(bool x);
  static bool allow_builtin_redef();

  static void set_allow_library_redef(bool x);
  static bool allow_library_redef();
  
  // map constants to the code representing them

  static std::map<std::string, std::string> func_map;
  static std::map<std::string, std::string> symbol_map;

  // constant strings
  static std::map<std::string, std::string> string_map;

  // floating-point
  static std::map<double, std::string> sc_real_map;

  // Boolean
  static std::map<int, std::string> sc_logical_map;

  // integers
  static std::map<int, std::string> sc_integer_map;

  // primitive functions (PRIMSXP)
  static std::map<int, std::string> primsxp_map;

  // locations of user global variables
  static std::map<std::string, std::string> loc_map;

  // bindings of global library functions/variables
  static std::map<std::string, std::string> binding_map;

  // functions called directly
  static std::set<std::string> direct_funcs;

private:
  static bool m_problem_flag;
  static bool m_allow_oo;
  static bool m_allow_envir_manip;
  static bool m_allow_special_redef;
  static bool m_allow_builtin_redef;
  static bool m_allow_library_redef;
};

#endif

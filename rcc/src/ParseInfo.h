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

// File: ParseInfo.h
//
// collection of global (whole-program) information collected during code generation
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef PARSE_INFO_H
#define PARSE_INFO_H

#include <map>
#include <string>
#include <set>

#include <CommandLineArgs.h>

// forward declarations
class SubexpBuffer;
class SplitSubexpBuffer;

// TODO: improve encapsulation
class ParseInfo {
public:

  static SubexpBuffer * global_fundefs;
  static SplitSubexpBuffer * global_constants;
  static SubexpBuffer * global_labels;

  static void set_command_line_args(CommandLineArgs * cl_args);
  static CommandLineArgs * get_command_line_args();

  static void flag_problem();
  static bool get_problem_flag();

  static void set_analysis_ok(bool x);
  static bool analysis_ok();

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

  static bool func_constant_exists(std::string func);
  static std::string get_func_constant(std::string func);
  static void insert_func_constant(std::string func, std::string value);

  static bool symbol_exists(std::string symbol);
  static std::string get_symbol(std::string symbol);
  static void insert_symbol(std::string symbol, std::string value);

  static bool string_constant_exists(std::string s);
  static std::string get_string_constant(std::string s);
  static void insert_string_constant(std::string s, std::string value);

  static bool real_constant_exists(double r);
  static std::string get_real_constant(double r);
  static void insert_real_constant(double r, std::string value);

  // Why not?
  static bool logical_constant_exists(int b);
  static std::string get_logical_constant(int b);
  static void insert_logical_constant(int b, std::string value);

  static bool integer_constant_exists(int i);
  static std::string get_integer_constant(int i);
  static void insert_integer_constant(int i, std::string value);

  static bool primsxp_constant_exists(int primsxp);
  static std::string get_primsxp_constant(int primsxp);
  static void insert_primsxp_constant(int primsxp, std::string value);

  static bool binding_exists(std::string binding);
  static std::string get_binding(std::string binding);
  static void insert_binding(std::string binding, std::string value);

private:

  // ----- maps of constants to their code representations -----

  // functions
  static std::map<std::string, std::string> s_func_map;

  // R identifiers
  static std::map<std::string, std::string> s_symbol_map;

  // constant strings
  static std::map<std::string, std::string> s_string_map;

  // floating-point
  static std::map<double, std::string> s_real_map;

  // Boolean
  static std::map<int, std::string> s_logical_map;

  // integers
  static std::map<int, std::string> s_integer_map;

  // primitive functions (PRIMSXP)
  static std::map<int, std::string> s_primsxp_map;

  // bindings of global library functions/variables
  static std::map<std::string, std::string> s_binding_map;

  static CommandLineArgs * s_cl_args;

  // ----- flags -----

  static bool s_problem_flag;
  static bool s_analysis_ok;
  static bool s_allow_oo;
  static bool s_allow_envir_manip;
  static bool s_allow_special_redef;
  static bool s_allow_builtin_redef;
  static bool s_allow_library_redef;
};

#endif

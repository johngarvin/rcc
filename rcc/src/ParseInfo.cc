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

// File: ParseInfo.cc
//
// collection of global (whole-program) information collected during code generation
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <assert.h>
#include <set>
#include <map>

#include "ParseInfo.h"

using namespace std;

// initialize statics
map<string, string> ParseInfo::s_func_map;
map<string, string> ParseInfo::s_symbol_map;
map<string, string> ParseInfo::s_string_map;
map<double, string> ParseInfo::s_real_map;
map<int, string> ParseInfo::s_logical_map;
map<int, string> ParseInfo::s_integer_map;
map<int, string> ParseInfo::s_primsxp_map;
map<string, string> ParseInfo::s_binding_map;
SubexpBuffer * ParseInfo::global_fundefs;
SplitSubexpBuffer * ParseInfo::global_constants;
SubexpBuffer * ParseInfo::global_labels;

CommandLineArgs * ParseInfo::s_cl_args = 0;

bool ParseInfo::s_problem_flag = false;
bool ParseInfo::s_analysis_ok  = true;

bool ParseInfo::s_allow_oo            = true;
bool ParseInfo::s_allow_envir_manip   = true;
bool ParseInfo::s_allow_special_redef = true;
bool ParseInfo::s_allow_builtin_redef = true;
bool ParseInfo::s_allow_library_redef = true;

void ParseInfo::set_command_line_args(CommandLineArgs * x) {
  s_cl_args = x;
}

CommandLineArgs * ParseInfo::get_command_line_args() {
  return s_cl_args;
}

void ParseInfo::flag_problem() {
  s_problem_flag = true;
}

bool ParseInfo::get_problem_flag() {
  return s_problem_flag;
}

void ParseInfo::set_analysis_ok(bool x) {
  s_analysis_ok = x;
}

bool ParseInfo::analysis_ok() {
  return s_analysis_ok;
}

// whether we have to account for object-oriented programming
void ParseInfo::set_allow_oo(bool x) {
  s_allow_oo = x;
}

bool ParseInfo::allow_oo() {
  return s_allow_oo;
}

// whether we have to account for explicit environment manipulation
// [get(), assign(), attach(), etc.]
void ParseInfo::set_allow_envir_manip(bool x) {
  s_allow_envir_manip = x;
}

bool ParseInfo::allow_envir_manip() {
  return s_allow_envir_manip;
}

// whether specials (control flow, etc.) might be redefined
void ParseInfo::set_allow_special_redef(bool x) {
  s_allow_special_redef = x;
}

bool ParseInfo::allow_special_redef() {
  return s_allow_special_redef;
}

// whether builtin functions (arithmetic, etc.) might be redefined
void ParseInfo::set_allow_builtin_redef(bool x) {
  s_allow_builtin_redef = x;
}

bool ParseInfo::allow_builtin_redef() {
  return s_allow_builtin_redef;
}

// whether library functions might be redefined
void ParseInfo::set_allow_library_redef(bool x) {
  s_allow_library_redef = x;
}

bool ParseInfo::allow_library_redef() {
  return s_allow_library_redef;
}

// maps for storing constants

// ----- function -----

bool ParseInfo::func_constant_exists(string func) {
  return (s_func_map.find(func) != s_func_map.end());
}

string ParseInfo::get_func_constant(string func) {
  map<string, string>::const_iterator it = s_func_map.find(func);
  assert(it != s_func_map.end());
  return it->second;
}

void ParseInfo::insert_func_constant(string func, string value) {
  s_func_map[func] = value;
}

// ----- symbol -----

bool ParseInfo::symbol_exists(string symbol) {
  return (s_symbol_map.find(symbol) != s_symbol_map.end());
}

string ParseInfo::get_symbol(string symbol) {
  map<string, string>::const_iterator it = s_symbol_map.find(symbol);
  assert(it != s_symbol_map.end());
  return it->second;
}

void ParseInfo::insert_symbol(string symbol, string value) {
  s_symbol_map[symbol] = value;
}

// ----- string -----

bool ParseInfo::string_constant_exists(string s) {
  return (s_string_map.find(s) != s_string_map.end());
}

string ParseInfo::get_string_constant(string s) {
  map<string, string>::const_iterator it = s_string_map.find(s);
  assert(it != s_string_map.end());
  return it->second;
}

void ParseInfo::insert_string_constant(string s, string value) {
  s_string_map[s] = value;
}

// ----- real -----

bool ParseInfo::real_constant_exists(double real) {
  return (s_real_map.find(real) != s_real_map.end());
}

string ParseInfo::get_real_constant(double real) {
  map<double, string>::const_iterator it = s_real_map.find(real);
  assert(it != s_real_map.end());
  return it->second;
}

void ParseInfo::insert_real_constant(double real, string value) {
  s_real_map[real] = value;
}

// ----- logical -----

bool ParseInfo::logical_constant_exists(int logical) {
  return (s_logical_map.find(logical) != s_logical_map.end());
}

string ParseInfo::get_logical_constant(int logical) {
  map<int, string>::const_iterator it = s_logical_map.find(logical);
  assert(it != s_logical_map.end());
  return it->second;
}

void ParseInfo::insert_logical_constant(int logical, string value) {
  s_logical_map[logical] = value;
}

// ----- integer -----

bool ParseInfo::integer_constant_exists(int integer) {
  return (s_integer_map.find(integer) != s_integer_map.end());
}

string ParseInfo::get_integer_constant(int integer) {
  map<int, string>::const_iterator it = s_integer_map.find(integer);
  assert(it != s_integer_map.end());
  return it->second;
}

void ParseInfo::insert_integer_constant(int integer, string value) {
  s_integer_map[integer] = value;
}

// ----- PRIMSXP -----

bool ParseInfo::primsxp_constant_exists(int primsxp) {
  return (s_primsxp_map.find(primsxp) != s_primsxp_map.end());
}

string ParseInfo::get_primsxp_constant(int primsxp) {
  map<int, string>::const_iterator it = s_primsxp_map.find(primsxp);
  assert(it != s_primsxp_map.end());
  return it->second;
}

void ParseInfo::insert_primsxp_constant(int primsxp, string value) {
  s_primsxp_map[primsxp] = value;
}

// ----- binding -----

bool ParseInfo::binding_exists(string binding) {
  return (s_binding_map.find(binding) != s_binding_map.end());
}

string ParseInfo::get_binding(string binding) {
  map<string, string>::const_iterator it = s_binding_map.find(binding);
  assert(it != s_binding_map.end());
  return it->second;
}

void ParseInfo::insert_binding(string binding, string value) {
  s_binding_map[binding] = value;
}

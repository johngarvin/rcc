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

#include <cassert>
#include <set>
#include <map>

#include "ParseInfo.h"

using namespace std;

// initialize statics
map<string, string> ParseInfo::m_func_map;
map<string, string> ParseInfo::m_symbol_map;
map<string, string> ParseInfo::m_string_map;
map<double, string> ParseInfo::m_real_map;
map<int, string> ParseInfo::m_logical_map;
map<int, string> ParseInfo::m_integer_map;
map<int, string> ParseInfo::m_primsxp_map;
map<string, string> ParseInfo::m_binding_map;
SubexpBuffer * ParseInfo::global_fundefs;
SplitSubexpBuffer * ParseInfo::global_constants;
SubexpBuffer * ParseInfo::global_labels;

bool ParseInfo::m_problem_flag = false;
bool ParseInfo::m_analysis_ok  = true;

bool ParseInfo::m_allow_oo            = true;
bool ParseInfo::m_allow_envir_manip   = true;
bool ParseInfo::m_allow_special_redef = true;
bool ParseInfo::m_allow_builtin_redef = true;
bool ParseInfo::m_allow_library_redef = true;

void ParseInfo::flag_problem() {
  m_problem_flag = true;
}

bool ParseInfo::get_problem_flag() {
  return m_problem_flag;
}

void ParseInfo::set_analysis_ok(bool x) {
  m_analysis_ok = x;
}

bool ParseInfo::analysis_ok() {
  return m_analysis_ok;
}

// whether we have to account for object-oriented programming
void ParseInfo::set_allow_oo(bool x) {
  m_allow_oo = x;
}

bool ParseInfo::allow_oo() {
  return m_allow_oo;
}

// whether we have to account for explicit environment manipulation
// [get(), assign(), attach(), etc.]
void ParseInfo::set_allow_envir_manip(bool x) {
  m_allow_envir_manip = x;
}

bool ParseInfo::allow_envir_manip() {
  return m_allow_envir_manip;
}

// whether specials (control flow, etc.) might be redefined
void ParseInfo::set_allow_special_redef(bool x) {
  m_allow_special_redef = x;
}

bool ParseInfo::allow_special_redef() {
  return m_allow_special_redef;
}

// whether builtin functions (arithmetic, etc.) might be redefined
void ParseInfo::set_allow_builtin_redef(bool x) {
  m_allow_builtin_redef = x;
}

bool ParseInfo::allow_builtin_redef() {
  return m_allow_builtin_redef;
}

// whether library functions might be redefined
void ParseInfo::set_allow_library_redef(bool x) {
  m_allow_library_redef = x;
}

bool ParseInfo::allow_library_redef() {
  return m_allow_library_redef;
}

// maps for storing constants

// ----- function -----

bool ParseInfo::func_constant_exists(string func) {
  return (m_func_map.find(func) != m_func_map.end());
}

string ParseInfo::get_func_constant(string func) {
  map<string, string>::const_iterator it = m_func_map.find(func);
  assert(it != m_func_map.end());
  return it->second;
}

void ParseInfo::insert_func_constant(string func, string value) {
  m_func_map[func] = value;
}

// ----- symbol -----

bool ParseInfo::symbol_exists(string symbol) {
  return (m_symbol_map.find(symbol) != m_symbol_map.end());
}

string ParseInfo::get_symbol(string symbol) {
  map<string, string>::const_iterator it = m_symbol_map.find(symbol);
  assert(it != m_symbol_map.end());
  return it->second;
}

void ParseInfo::insert_symbol(string symbol, string value) {
  m_symbol_map[symbol] = value;
}

// ----- string -----

bool ParseInfo::string_constant_exists(string s) {
  return (m_string_map.find(s) != m_string_map.end());
}

string ParseInfo::get_string_constant(string s) {
  map<string, string>::const_iterator it = m_string_map.find(s);
  assert(it != m_string_map.end());
  return it->second;
}

void ParseInfo::insert_string_constant(string s, string value) {
  m_string_map[s] = value;
}

// ----- real -----

bool ParseInfo::real_constant_exists(double real) {
  return (m_real_map.find(real) != m_real_map.end());
}

string ParseInfo::get_real_constant(double real) {
  map<double, string>::const_iterator it = m_real_map.find(real);
  assert(it != m_real_map.end());
  return it->second;
}

void ParseInfo::insert_real_constant(double real, string value) {
  m_real_map[real] = value;
}

// ----- logical -----

bool ParseInfo::logical_constant_exists(int logical) {
  return (m_logical_map.find(logical) != m_logical_map.end());
}

string ParseInfo::get_logical_constant(int logical) {
  map<int, string>::const_iterator it = m_logical_map.find(logical);
  assert(it != m_logical_map.end());
  return it->second;
}

void ParseInfo::insert_logical_constant(int logical, string value) {
  m_logical_map[logical] = value;
}

// ----- integer -----

bool ParseInfo::integer_constant_exists(int integer) {
  return (m_integer_map.find(integer) != m_integer_map.end());
}

string ParseInfo::get_integer_constant(int integer) {
  map<int, string>::const_iterator it = m_integer_map.find(integer);
  assert(it != m_integer_map.end());
  return it->second;
}

void ParseInfo::insert_integer_constant(int integer, string value) {
  m_integer_map[integer] = value;
}

// ----- PRIMSXP -----

bool ParseInfo::primsxp_constant_exists(int primsxp) {
  return (m_primsxp_map.find(primsxp) != m_primsxp_map.end());
}

string ParseInfo::get_primsxp_constant(int primsxp) {
  map<int, string>::const_iterator it = m_primsxp_map.find(primsxp);
  assert(it != m_primsxp_map.end());
  return it->second;
}

void ParseInfo::insert_primsxp_constant(int primsxp, string value) {
  m_primsxp_map[primsxp] = value;
}

// ----- binding -----

bool ParseInfo::binding_exists(string binding) {
  return (m_binding_map.find(binding) != m_binding_map.end());
}

string ParseInfo::get_binding(string binding) {
  map<string, string>::const_iterator it = m_binding_map.find(binding);
  assert(it != m_binding_map.end());
  return it->second;
}

void ParseInfo::insert_binding(string binding, string value) {
  m_binding_map[binding] = value;
}

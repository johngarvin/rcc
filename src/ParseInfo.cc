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

#include "ParseInfo.h"

// initialize statics
std::map<std::string, std::string> ParseInfo::func_map;
std::map<std::string, std::string> ParseInfo::symbol_map;
std::map<std::string, std::string> ParseInfo::string_map;
std::map<double, std::string> ParseInfo::sc_real_map;
std::map<int, std::string> ParseInfo::sc_logical_map;
std::map<int, std::string> ParseInfo::sc_integer_map;
std::map<int, std::string> ParseInfo::primsxp_map;
std::map<std::string, std::string> ParseInfo::loc_map;
std::map<std::string, std::string> ParseInfo::binding_map;
std::set<std::string> ParseInfo::direct_funcs;
SubexpBuffer * ParseInfo::global_fundefs;
SplitSubexpBuffer * ParseInfo::global_constants;
SubexpBuffer * ParseInfo::global_labels;

bool ParseInfo::m_problem_flag = false;

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

// Returns true if the given string represents a function specified
// for direct calling.
bool ParseInfo::is_direct(std::string func) {
  return (direct_funcs.find(func) != direct_funcs.end());
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

// -*- Mode: C++ -*-
//
// Copyright (c) 2009 Rice University
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

// File: Settings.h
//
// Settings for analyses and optimizations. Default is to turn all optimizations on.
// Note: changes here should also be reflected in CommandLineArgs.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ANALYSIS_SETTINGS_H
#define ANALYSIS_SETTINGS_H

#include <string>

#define BOOL_GETTER_SETTER(__b__) \
public: \
  bool get_ ## __b__() { return m_ ## __b__; } \
  void set_ ## __b__(bool x) { m_ ## __b__ = x; } \
private: \
  bool m_ ## __b__;

#define SETTINGS_PRETTY_PRINT(__b__) # __b__ ": " + as_string(m_ ## __b__) + "\n"

class Settings {
  BOOL_GETTER_SETTER(for_loop_range_deforestation)
  BOOL_GETTER_SETTER(subscript_assignment)
  BOOL_GETTER_SETTER(strictness)
  BOOL_GETTER_SETTER(special_case_arithmetic)
  BOOL_GETTER_SETTER(call_graph)
  BOOL_GETTER_SETTER(lookup_elimination)
  BOOL_GETTER_SETTER(stack_alloc_obj)
  BOOL_GETTER_SETTER(stack_debug)
  BOOL_GETTER_SETTER(assume_correct_program)
  BOOL_GETTER_SETTER(aggressive_cbv)
  BOOL_GETTER_SETTER(resolve_arguments)

  // Singleton pattern
public:
  static Settings * get_instance();

private:
  Settings() : m_for_loop_range_deforestation(true),
	       m_subscript_assignment(true),
	       m_strictness(true),
	       m_special_case_arithmetic(true),
	       m_call_graph(true),
	       m_lookup_elimination(true),
	       m_stack_alloc_obj(true),
	       m_stack_debug(false),
	       m_assume_correct_program(false),
	       m_aggressive_cbv(false),
	       m_resolve_arguments(true)
  { }
  static Settings * s_instance;
  static std::string as_string(bool b) {
    return (b ? "true" : "false");
  }

public:
  std::string get_pp_info() {        // pretty-print the current settings
    std::string out;
    out += SETTINGS_PRETTY_PRINT(for_loop_range_deforestation);
    out += SETTINGS_PRETTY_PRINT(subscript_assignment);
    out += SETTINGS_PRETTY_PRINT(strictness);
    out += SETTINGS_PRETTY_PRINT(special_case_arithmetic);
    out += SETTINGS_PRETTY_PRINT(call_graph);
    out += SETTINGS_PRETTY_PRINT(lookup_elimination);
    out += SETTINGS_PRETTY_PRINT(stack_alloc_obj);
    out += SETTINGS_PRETTY_PRINT(stack_debug);
    out += SETTINGS_PRETTY_PRINT(assume_correct_program);
    out += SETTINGS_PRETTY_PRINT(aggressive_cbv);
    out += SETTINGS_PRETTY_PRINT(resolve_arguments);
    return out;
  }
};

#endif

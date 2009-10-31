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
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ANALYSIS_SETTINGS_H
#define ANALYSIS_SETTINGS_H

#define BOOL_GETTER_SETTER(__b__) \
public: \
 bool get_ ## __b__() { return m_ ## __b__; } \
 void set_ ## __b__(bool x) { m_ ## __b__ = x; } \
private: \
bool m_ ## __b__;



class Settings {
  BOOL_GETTER_SETTER(for_loop_range_deforestation)
  BOOL_GETTER_SETTER(subscript_assignment)
  BOOL_GETTER_SETTER(strictness)
  BOOL_GETTER_SETTER(special_case_arithmetic)
  BOOL_GETTER_SETTER(call_graph)
  BOOL_GETTER_SETTER(lookup_elimination)
  BOOL_GETTER_SETTER(stack_alloc_obj)

  // Singleton pattern
public:
  static Settings * get_instance();
private:
  Settings() : m_for_loop_range_deforestation(true),
	       m_subscript_assignment(true),
	       m_strictness(true),
	       m_special_case_arithmetic(true),
	       m_call_graph(true),
	       m_lookup_elimination(true)
  { }
  static Settings * s_instance;
};

#endif

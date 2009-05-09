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

// File: Metrics.h
//
// Gathers compile-time metrics.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef METRICS_H
#define METRICS_H

#define INT_GETTER_INCREMENTER(__a__)		     \
public:                                              \
  int get_ ## __a__() const { return m_ ## __a__; }  \
  void inc_ ## __a__() { m_ ## __a__ ++; }           \
private:                                             \
  int m_ ## __a__;

class Metrics {
public:
  static Metrics * get_instance();

public:
  INT_GETTER_INCREMENTER(procedures)

  INT_GETTER_INCREMENTER(builtin_calls)
  INT_GETTER_INCREMENTER(special_calls)
  INT_GETTER_INCREMENTER(library_calls)
  INT_GETTER_INCREMENTER(user_calls)
  INT_GETTER_INCREMENTER(unknown_symbol_calls)
  INT_GETTER_INCREMENTER(non_symbol_calls)

//   int strict_formal_args;
//   int nonstrict_formal_args;

  INT_GETTER_INCREMENTER(eager_actual_args)
  INT_GETTER_INCREMENTER(lazy_actual_args)

//   int single_arrow_assignments;
//   int double_arrow_assignments;

//   int uses;
//   int defs;
//   int formal_arg_uses;
//   int assigned_uses;
//   int compiled_uses;
//   int lookup_uses;
//   int compiled_defs;
//   int lookup_defs;

private:
  Metrics() : m_procedures(0),
	      m_builtin_calls(0),
	      m_special_calls(0),
	      m_library_calls(0),
	      m_user_calls(0),
	      m_unknown_symbol_calls(0),
	      m_non_symbol_calls(0),
	      m_eager_actual_args(0),
	      m_lazy_actual_args(0)
  {}
private:
  static Metrics * s_instance;
};

#endif

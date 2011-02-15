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

// File: Output.cc
//
// Represents the code produced by methods of the CodeGen class.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <Output.h>

// big constructor that sets everything
Output::Output(Decls _d,
	       Code _c,
	       GDecls _gd,
	       GCode _gc, 
	       Handle _h,
	       DelText _del,
	       DependenceType _dt,
	       VisibilityType _v)
    : m_decls(_d.get()),
      m_code(_c.get()),
      m_g_decls(_gd.get()),
      m_g_code(_gc.get()), 
      m_handle(_h.get()),
      m_del_text(_del.get()),
      m_dependence(_dt),
      m_visibility(_v)
  {}

Output::Output()
  : m_decls(""),
    m_code(""),
    m_g_decls(""),
    m_g_code(""),
    m_handle(""),
    m_del_text(""),
    m_dependence(CONST),
    m_visibility(INVISIBLE)
  {}

const Output & Output::bogus = Output(Decls(""), Code(""),
				      GDecls(""), GCode(""),
				      Handle(""), DelText(""),
				      CONST, INVISIBLE);

const Output & Output::nil = Output(Decls(""), Code(""),
				    GDecls(""), GCode(""),
				    Handle("R_NilValue"), DelText(""),
				    CONST, INVISIBLE);

Output Output::global(GDecls _gd, GCode _gc, Handle _h, VisibilityType _v) {
  return Output(Decls(""), Code(""), _gd, _gc, _h, DelText(""), CONST, _v);
}

Output Output::dependent(Decls _d, Code _c, Handle _h, DelText _dt, VisibilityType _v) {
  return Output(_d, _c, GDecls(""), GCode(""), _h, _dt, DEPENDENT, _v);
}

Output Output::visible_const(Handle _h) {
  return Output(Decls(""), Code(""), GDecls(""), GCode(""),
		_h, DelText(""), CONST, VISIBLE);
}

Output Output::invisible_const(Handle _h) {
  return Output(Decls(""), Code(""), GDecls(""), GCode(""),
		_h, DelText(""), CONST, INVISIBLE);
}


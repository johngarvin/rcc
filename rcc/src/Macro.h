#ifndef MACRO_H
#define MACRO_H

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

// File: Macro.h
//
// Output common string patterns.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

class Macro {
public:
  explicit Macro(const std::string b) : body(b) { }
  const std::string call(const int nargs, const std::string args[]) const;
private:
  const std::string body;
};

class MacroFactory {
public:
  static MacroFactory * instance();
  const Macro get_macro(const std::string name) const;
protected:
  explicit MacroFactory() {}
private:
  static MacroFactory * s_instance;
  static const std::string s_path;
};

extern const MacroFactory mf;
extern const Macro mac_primsxp;
extern const Macro mac_ifelse;

#endif

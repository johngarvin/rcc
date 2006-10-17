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

// File: Macro.cc
//
// Output common string patterns.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <support/StringUtils.h>
#include <support/RccError.h>

#include "Macro.h"

MacroFactory * MacroFactory::_instance = 0;

const std::string MacroFactory::m_path = RCC_MACRO_PATH;

const MacroFactory mf = *MacroFactory::instance();
const Macro mac_primsxp = mf.get_macro("primsxp");
const Macro mac_ifelse = mf.get_macro("ifelse");

MacroFactory * MacroFactory::instance() {
  if (_instance == 0) {
    _instance = new MacroFactory;
  }
  return _instance;
}

const Macro MacroFactory::get_macro(const std::string name) const {
  std::ostringstream ss;
  const std::string filename = m_path + "/" + name + ".macro";
  std::ifstream mac_file(filename.c_str());
  if (mac_file.fail()) {
    rcc_error("Couldn't open macro file " + filename);
  }
  ss << mac_file.rdbuf();
  mac_file.close();
  const std::string body = ss.str();
  return Macro(body);
}

// TODO: make it so calling with the wrong number of arguments
// fails gracefully. Varargs?

const std::string Macro::call(const int nargs, const std::string args[]) const {
  std::string var;
  std::string::size_type pos;
  std::string result = body;
  for (int i=0; i<nargs; i++) {
    var = "$" + i_to_s(i+1) + "$";
    while((pos = result.find(var)) != std::string::npos) {
      result.replace(pos, var.length(), args[i]);
    }
  }
  return result;
}

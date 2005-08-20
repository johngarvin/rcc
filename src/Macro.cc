// Copyright (c) 2005 John Garvin 
//
// May 27, 2005
//
// RCC uses macros to output common patterns.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
//

#include "Macro.h"

#define __USE_STD_IOSTREAM

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

static std::string i_to_s(const int i);

MacroFactory * MacroFactory::_instance = 0;

const std::string MacroFactory::directory = RCC_MACRO_PATH;

const MacroFactory mf = *MacroFactory::Instance();
const Macro mac_primsxp = mf.getMacro("primsxp");
const Macro mac_ifelse = mf.getMacro("ifelse");

MacroFactory * MacroFactory::Instance() {
  if (_instance == 0) {
    _instance = new MacroFactory;
  }
  return _instance;
}

const Macro MacroFactory::getMacro(const std::string name) const {
  std::ostringstream ss;
  const std::string filename = directory + "/" + name + ".macro";
  std::ifstream mac_file(filename.c_str());
  if (mac_file.fail()) {
    std::cerr << "Error: Couldn't open macro file " + filename + "\n";
    exit(1);
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
  for (int i=1; i<=nargs; i++) {
    var = "$" + i_to_s(i) + "$";
    while((pos = result.find(var)) != std::string::npos) {
      result.replace(pos, var.length(), args[i-1]);
    }
  }
  return result;
}

static std::string i_to_s(const int i) {
  if (i == (int)0x80000000) {
    return "0x80000000"; // Doesn't work as a decimal constant
  } else {
    std::ostringstream ss;
    ss << i;
    return ss.str();
  }
}

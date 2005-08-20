#ifndef MACRO_H
#define MACRO_H

/* Copyright (c) 2005 John Garvin 
 *
 * May 27, 2005
 *
 * RCC uses macros to output common patterns.
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#include <string>

class Macro {
public:
  Macro(const std::string b) : body(b) { }
  const std::string call(const int nargs, const std::string args[]) const;
private:
  const std::string body;
};

class MacroFactory {
public:
  static MacroFactory * Instance();
  const Macro getMacro(const std::string name) const;
protected:
  MacroFactory() {}
private:
  static MacroFactory * _instance;
  static const std::string directory;
};

extern const MacroFactory mf;
extern const Macro mac_primsxp;
extern const Macro mac_ifelse; 

#endif

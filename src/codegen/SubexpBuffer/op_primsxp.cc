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

// File: op_primsxp.cc
//
// Output a primitive application (application where the left side is of type PRIMSXP).
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_Defn.h>

#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <support/RccError.h>
#include <Macro.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_primsxp(SEXP e, string rho) {
  string var;
  int is_builtin;
  switch (TYPEOF(e)) {
  case SPECIALSXP:
    is_builtin = 0;
    break;
  case BUILTINSXP:
    is_builtin = 1;
    break;
  default:
    rcc_error("Internal error: op_primsxp called on non-(special or builtin)");
    is_builtin = 0; // silence the -Wall Who Cried "Uninitialized variable."
  }
  
  // unique combination of offset and is_builtin for memoization
  int value = 2 * PRIMOFFSET(e) + is_builtin;
  map<int,string>::iterator pr = ParseInfo::primsxp_map.find(value);
  if (pr != ParseInfo::primsxp_map.end()) {  // primsxp already defined
    return Expression(pr->second, DEPENDENT, INVISIBLE, "");
  } else {
    string var = new_sexp();
    const string args[] = {var, i_to_s(PRIMOFFSET(e)),
			   i_to_s(is_builtin), string(PRIMNAME(e))};
    append_defs(mac_primsxp.call(4, args));
    ParseInfo::primsxp_map.insert(pair<int,string>(value, var));
    return Expression(var, DEPENDENT, INVISIBLE, "");
  }
}

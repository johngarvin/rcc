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

// File: op_vector.cc
//
// Output a vector or array.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>
#include <map>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <support/RccError.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_vector(SEXP vec) {
  int len = Rf_length(vec);
  switch(TYPEOF(vec)) {
  case LGLSXP:
    if (len == 1) {
      int value = INTEGER(vec)[0];
      map<int,string>::iterator pr = ParseInfo::sc_logical_map.find(value);
      if (pr == ParseInfo::sc_logical_map.end()) {  // not found
	string var = ParseInfo::global_constants->appl1("ScalarLogical",
					    i_to_s(value));
	ParseInfo::sc_logical_map.insert(pair<int,string>(value, var));
	return Expression(var, FALSE, VISIBLE, "");
      } else {
	return Expression(pr->second, FALSE, VISIBLE, "");
      }
    } else {
      ParseInfo::flag_problem();
      return Expression("<<unimplemented logical vector>>",
			FALSE, INVISIBLE, "");
    }
    break;
  case INTSXP:
    if (len == 1) {
      int value = INTEGER(vec)[0];
      map<int,string>::iterator pr = ParseInfo::sc_integer_map.find(value);
      if (pr == ParseInfo::sc_integer_map.end()) {  // not found
	string var = ParseInfo::global_constants->appl1("ScalarInteger",
					    i_to_s(value));
	ParseInfo::sc_integer_map.insert(pair<int,string>(value, var));
	return Expression(var, FALSE, VISIBLE, "");
      } else {
	return Expression(pr->second, FALSE, VISIBLE, "");
      }
    } else {
      ParseInfo::flag_problem();
      return Expression("<<unimplemented integer vector>>",
			FALSE, INVISIBLE, "");
    }
    break;
  case REALSXP:
    if (len == 1) {
      double value = REAL(vec)[0];
      map<double,string>::iterator pr = ParseInfo::sc_real_map.find(value);
      if (pr == ParseInfo::sc_real_map.end()) {  // not found
	string var = ParseInfo::global_constants->appl1("ScalarReal",
					    d_to_s(value));
	ParseInfo::sc_real_map.insert(pair<double,string>(value, var));
	return Expression(var, FALSE, VISIBLE, "");
      } else {
	return Expression(pr->second, FALSE, VISIBLE, "");
      }
    } else {
      ParseInfo::flag_problem();
      return Expression("<<unimplemented real vector>>",
			FALSE, INVISIBLE, "");
    }
    break;
  case CPLXSXP:
    if (len == 1) {
      Rcomplex value = COMPLEX(vec)[0];
      string var = ParseInfo::global_constants->appl1("ScalarComplex",
					  c_to_s(value));
      return Expression(var, FALSE, VISIBLE, "");
    } else {
      ParseInfo::flag_problem();
      return Expression("<<unimplemented complex vector>>",
			FALSE, INVISIBLE, "");
    }
    break;
  default:
    rcc_error("Internal error: op_vector encountered bad vector type");
    return Expression::bogus_exp; // not reached
    break;
  }
}

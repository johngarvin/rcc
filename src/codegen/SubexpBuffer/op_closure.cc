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

// File: op_closure.cc
//
// Output a closure.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <cassert>
#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>
#include <support/StringUtils.h>
#include <CodeGen.h>
#include <ParseInfo.h>
#include <Dependence.h>
#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_closure(SEXP e, string rho, Protection resultProtection) {
  assert(TYPEOF(e) == CLOSXP);
  Expression formals = op_list(FORMALS(e), rho, true, Protected);
  Expression body = op_literal(BODY(e), rho);
  if (rho == "R_GlobalEnv" &&
      formals.dependence == CONST &&
      body.dependence == CONST)
  {
    string v = ParseInfo::global_constants->appl3("mkCLOSXP",
						  to_string(e),
						  formals.var,
						  body.var,
						  rho,
						  resultProtection);
    ParseInfo::global_constants->del(formals);
    ParseInfo::global_constants->del(body);
    return Expression(v, CONST, INVISIBLE, "");
  } else {
    string v = appl3("mkCLOSXP",
		     to_string(e),
		     formals.var,
		     body.var,
		     rho,
		     resultProtection);
    del(formals);
    del(body);
    string del_text = (resultProtection == Protected ? unp(v) : "");
    return Expression(v, DEPENDENT, INVISIBLE, del_text);
  }
}

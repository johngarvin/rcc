// -*- Mode: C++ -*-
//
// Copyright (c) 2010 Rice University
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

// File: op_subset.cc
//
// Output a single-bracket subset expression, e.g. a[b]
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <cassert>
#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_Defn.h>

#include <analysis/Utils.h>

#include <support/StringUtils.h>

#include <CodeGenUtils.h>
#include <ParseInfo.h>
#include <Visibility.h>

using std::string;

Expression SubexpBuffer::op_subscript(SEXP e, SEXP op, string rho, Protection resultProtection) {
  assert(is_subscript(e));

  Expression op1 = ParseInfo::global_constants->op_primsxp(op, rho);
  Expression args1 = op_list(CDR(e), rho, false, Protected, true);
  string call_str = appl2("lcons", "", op1.var, args1.var);
  Expression call = Expression(call_str, CONST, VISIBLE, unp(call_str));
  string out = appl4("do_subset",
		     "op_subscript: " + to_string(e),
		     call.var,
		     op1.var,
		     args1.var,
		     rho,
		     resultProtection);
  string cleanup;
  if (resultProtection == Protected) cleanup = unp(out);
  del(call);
  del(op1);
  del(args1);
  return Expression(out, DEPENDENT,
		    1 - PRIMPRINT(op) ? VISIBLE : INVISIBLE,
		    cleanup);
}

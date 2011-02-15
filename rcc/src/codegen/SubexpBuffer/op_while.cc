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

// File: op_while.cc
//
// Output a while loop.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>

#include <support/StringUtils.h>

#include <CodeGenUtils.h>
#include <LoopContext.h>
#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_while(SEXP e, string rho, ResultStatus resultStatus) {
  string in_loop;
  SubexpBuffer loop;
  LoopContext loop_context;

  if (resultStatus == ResultNeeded) {
    append_decls("SEXP ans;\n");
    append_decls("PROTECT_INDEX api;\n");
    append_defs("ans = R_NilValue;\n");
    append_defs("PROTECT_WITH_INDEX(ans, &api);\n");
  }

  // output code in loop
  Expression condition = loop.op_exp(while_cond_c(e), rho, Unprotected, false);
  loop.append_defs("if (!my_asLogicalNoNA(" + condition.var + ")) break;\n");
  Expression body = loop.op_exp(while_body_c(e), rho, Unprotected, false, resultStatus);
  if (resultStatus == ResultNeeded) {
    loop.append_defs("REPROTECT(ans = " + body.var + ", api);\n");
  }
  in_loop = "/* while loop */\n" + loop.output_decls() + loop.output_defs();

  // output loop
  append_defs("while(1) " + emit_in_braces(in_loop));
  append_defs(loop_context.breakLabel() + ":\n");
  if (resultStatus == ResultNeeded) {
    append_defs(emit_unprotect("ans"));
  }
  return Expression("ans", DEPENDENT, VISIBLE, "");
}


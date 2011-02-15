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

// File: TypeInference.h
//
// Extends the R AST to perform McCosh type inference. Not yet implemented.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef R_TYPE_INFERENCE_H
#define R_TYPE_INFERENCE_H

#include <include/R/R_RInterface.h>

// resolve conflict: different version in OpenAnalysis and Mint
#undef Iterator_H
#undef Exception_H

#include <LangFree/Utils/Iterator.h>
#include <LangFree/Parse/AST.h>
#include <LangFree/Annot/Annotation.h>
// #include "PrimFunctionMap.h"
// #include "MatlabEnvironment.h"

class Type_R_IRInterface : public R_IRInterface {
  
  // make_constraints adds the current statement's information to the
  // constraints for the current procedure. It uses the compiler's
  // annotation of the operation/function being called (info) and the
  // current function (curr_func) and adds the current information to
  // the statement constraint list (cnstr_list).
  void make_constraints(OA::StmtHandle stmt,
			Annotation::StmtConstraintList & cnstr_list, Annotation::OperationInfo & info,
			const Annotation::FuncSig & curr_func);

  // Model make_constraints from MATLAB:

  // make_constraints for ASSIGNMENT:
  // info_L = new OperationInfo
  // leftchild.make_constraints(cnstr_list, info_L, curr_func)
  // rightchild.make_constraints(cnstr_list, info, curr_func)
  // info = merge information in info and info_L
  // cnstr_list.add_constraint(info, curr_func)

  // make_constraints for FUNCTION CALL/ARRAY REFERENCE:
  // make_constraints(stmt, info):
  // if stmt is an array expression:
  //   for each subscript:
  //     (right now, only considers constants)
  //     if node type = "named value"  (?)
  //       subscript_size = 1
  //     else (assuming node is an InternalNode)
  //       if node is
  //         variable:                  subscript_size = 1
  //         empty:                     subscript_size = 0
  //         otherwise, including NULL: subscript_size = -1
  //     insert subscript_size for this subscript into info

  // make_constraints for FUNCTION DEFINITION:
  // For each argument a:
  //   make_constraints(a);  // will be identity
  //   info.add_operand(identity);
  // For each return value v:
  //   make_constraints(v);  // will be identity
  //   info.add_result(identity);
  // add new info to constraints
};

#endif

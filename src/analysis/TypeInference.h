// Extends the R AST to perform type inference.

#ifndef R_TYPE_INFERENCE_H
#define R_TYPE_INFERENCE_H

#include "MyIRInterface.h"

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
			Annotation::StmtConstraintList& cnstr_list, Annotation::OperationInfo& info,
			const Annotation::FuncSig& curr_func);

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

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

// File: BasicFuncInfo.h
//
// Annotation for a function.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ANNOTATION_BASIC_FUNC_INFO_H
#define ANNOTATION_BASIC_FUNC_INFO_H

#include <set>

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/CFG/CFG.hpp>

#include <support/trees/NonUniformDegreeTreeTmpl.h>

#include <analysis/AnnotationBase.h>
#include <analysis/LexicalScope.h>
#include <analysis/PropertyHndl.h>

// iterator macros for convenience and readability. Be sure to declare
// the variables somewhere above the for loop. Note: don't put
// initializations in macros except inside for-loop initializers

#define PROC_FOR_EACH_NODE(fi, node) \
  for(OA::OA_ptr<OA::CFG::NodesIteratorInterface> ni = (fi)->get_cfg()->getCFGNodesIterator(); \
      ni->isValid() && ((node) = ni->current().convert<OA::CFG::Node>()) != BasicFuncInfo::iterator_dummy_node; \
      ++*ni)

#define CFG_FOR_EACH_NODE(cfg, node) \
  for(OA::OA_ptr<OA::CFG::NodesIteratorInterface> ni = (cfg)->getCFGNodesIterator(); \
      ni->isValid() && ((node) = ni->current().convert<OA::CFG::Node>()) != BasicFuncInfo::iterator_dummy_node; \
      ++*ni)

#define NODE_FOR_EACH_STATEMENT(node, stmt) \
  for(OA::OA_ptr<OA::CFG::NodeStatementsIteratorInterface> si = (node)->getNodeStatementsIterator(); \
      si->isValid() && ((stmt) = si->current()) != OA::StmtHandle(0);	\
      ++*si)

namespace RAnnot {

class BasicFuncInfo : public NonUniformDegreeTreeNodeTmpl<BasicFuncInfo>,
		      public AnnotationBase
{
public:
  explicit BasicFuncInfo(BasicFuncInfo * parent, SEXP name, SEXP defn);
  virtual ~BasicFuncInfo();
  
  virtual AnnotationBase * clone();
  
  static PropertyHndlT handle();
  
  void perform_analysis();

  // ----- basic information -----

  SEXP get_sexp() const;

  /// cell containing first R name assigned
  SEXP get_first_name_c() const;

  /// name of C function
  const std::string& get_c_name(); // not const: fills in m_c_name if empty

  /// name of C variable storing the closure (CLOSXP)
  const std::string& get_closure();  // not const: fills in m_closure if empty

  // ----- arguments -----

  unsigned int get_num_args() const;
  void set_num_args(unsigned int x);
  SEXP get_args() const; 
  bool is_arg(SEXP sym) const;
  bool are_all_value() const;
  bool is_arg_value(SEXP arg) const;
  
  /// get the cell containing the formal arg at the given position. Indexed from 1.
  SEXP get_arg(int position) const;

  /// find the numerical position of the formal with the given name. Indexed from 1.
  int find_arg_position(char* name) const;

  // has variable arguments
  bool get_has_var_args() const;
  void set_has_var_args(bool x);

  // ----- CFG -----

  OA::OA_ptr<OA::CFG::CFG> get_cfg() const;
  void set_cfg(OA::OA_ptr<OA::CFG::CFG> x);
  
  // ----- lexical scope and tree structure -----

  FundefLexicalScope * get_scope() const;

  /// Get lexical parent. Returns 0 if no parent (top-level procedure
  /// or library procedure)
  BasicFuncInfo * get_parent() const;

  bool has_children() const;

  // ----- return statements -----

  const std::set<SEXP> * get_implicit_returns() const;
  bool is_return(SEXP) const;
  SEXP return_value_c(const SEXP) const;
  
  // ----- context -----
  bool requires_context() const;

  // ----- debugging -----

  virtual std::ostream& dump(std::ostream& os) const;

  // ----- dummy node for iterator -----

  /// dummy node not equal to any genuine CFG node. Useful for
  /// iteration macros like CFG_FOR_EACH_NODE.
  static const OA::OA_ptr<OA::CFG::NodeInterface> iterator_dummy_node;

private:
  void accum_implicit_returns(SEXP cell);

private:
  unsigned int m_num_args;         // number of known arguments
  bool m_has_var_args;             // variable number of arguments (uses "...")
  std::string m_c_name;            // C linkage name
  std::string m_closure;           // C closure (CLOSXP) name
  bool m_requires_context;         // is an R context object needed for the function?
  FundefLexicalScope * m_scope;    // lexical scope
  OA::OA_ptr<OA::CFG::CFG> m_cfg;  // control flow graph
  SEXP m_sexp;                     // function definition
  SEXP m_first_name_c;             // cell containing name of function at original definition
  BasicFuncInfo * m_parent;        // scope tree parent
  std::set<SEXP> m_returns;        // implicit return statements
};

class BasicFuncInfoIterator : public NonUniformDegreeTreeIteratorTmpl<BasicFuncInfo> {
public:
  explicit BasicFuncInfoIterator(const BasicFuncInfo * bfi,
				 TraversalOrder torder = PreOrder,
				 NonUniformDegreeTreeEnumType how =
				 NON_UNIFORM_DEGREE_TREE_ENUM_ALL_NODES);
};

} // namespace RAnnot

#endif  // ANNOTATION_BASIC_FUNC_INFO_H

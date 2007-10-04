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

// File: FuncInfo.h
//
// Annotation for a function.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ANNOTATION_FUNC_INFO_H
#define ANNOTATION_FUNC_INFO_H

#include <list>

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/CFG/CFG.hpp>

#include <support/trees/NonUniformDegreeTreeTmpl.h>

#include <analysis/AnnotationBase.h>
#include <analysis/LexicalScope.h>
#include <analysis/PropertyHndl.h>
#include <analysis/Var.h>

namespace RAnnot {

//****************************************************************************
// Annotations: Function info
//****************************************************************************


// ---------------------------------------------------------------------------
// FuncInfo: 'Definition' information about a function
// ---------------------------------------------------------------------------
class FuncInfo : public NonUniformDegreeTreeNodeTmpl<FuncInfo>, 
		 public AnnotationBase
{
public:
  typedef Var*                                             MentionT;

  typedef std::list<MentionT>                              MentionSetT;
  typedef MentionSetT::iterator                            mention_iterator;
  typedef MentionSetT::const_iterator                      const_mention_iterator;

  typedef SEXP                                             CallSiteT;
  typedef std::list<CallSiteT>                             CallSiteSetT;
  typedef CallSiteSetT::iterator                           call_site_iterator;
  typedef CallSiteSetT::const_iterator                     const_call_site_iterator;
public:
  FuncInfo(FuncInfo* parent, SEXP name, SEXP defn);
  virtual ~FuncInfo();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual AnnotationBase* clone() { return 0; } // don't support this since it is linked! 

  static PropertyHndlT handle();

  // -------------------------------------------------------
  // member data manipulation
  // -------------------------------------------------------
  
  // arguments
  unsigned int get_num_args() const;
  void set_num_args(unsigned int x);
  SEXP get_args() const; 
  bool is_arg(SEXP sym) const;
  bool are_all_value() const;
  bool is_arg_value(SEXP arg) const;
  
  /// get the cell containing the formal arg at the given position. Indexed from 1.
  SEXP get_arg(int position) const;

  // find the numerical position of the formal with the given name. Indexed from 1.
  int find_arg_position(char* name) const;

  void analyze_args();

  SEXP get_defn() const;

  /// cell containing first R name assigned
  SEXP get_first_name_c() const;

  // has variable arguments
  bool get_has_var_args() const;
  void set_has_var_args(bool x);

  /// name of C function
  const std::string& get_c_name(); // not const: fills in m_c_name if empty

  /// name of C variable storing the closure (CLOSXP)
  const std::string& get_closure();  // not const: fills in m_closure if empty

  // context
  void set_requires_context(bool requires_context); 
  bool requires_context() const;

  // add information about mentions and call sites
  void insert_mention(MentionT v);
  void insert_call_site(CallSiteT e);

  // CFG
  OA::OA_ptr<OA::CFG::CFG> get_cfg() const;
  void set_cfg(OA::OA_ptr<OA::CFG::CFG> x);
  
  // mentions and call sites
  void collect_mentions_and_call_sites();

  // mention iterators
  mention_iterator begin_mentions();
  const_mention_iterator begin_mentions() const;
  mention_iterator end_mentions();
  const_mention_iterator end_mentions() const;
  
  // call site iterators
  call_site_iterator begin_call_sites();
  const_call_site_iterator begin_call_sites() const;
  call_site_iterator end_call_sites();
  const_call_site_iterator end_call_sites() const;

  // lexical scope
  FundefLexicalScope * get_scope() const;

  void perform_analysis();

  bool has_children() const;

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  unsigned int m_num_args;        // number of known arguments
  bool m_has_var_args;            // variable number of arguments
  std::string m_c_name;           // C linkage name
  std::string m_closure;          // C closure (CLOSXP) name
  bool m_requires_context;        // is an R context object needed for the function?
  FundefLexicalScope * m_scope;   // lexical scope
  OA::OA_ptr<OA::CFG::CFG> m_cfg; // control flow graph

  MentionSetT m_mentions; // uses and defs inside function (NOT including nested functions)
  CallSiteSetT m_call_sites; // call sites inside function (NOT including nested functions)

  SEXP m_defn;         // function definition
  SEXP m_first_name_c; // cell containing name of function at original definition 
  FuncInfo *m_parent;  // parent scope definition

  // argument description: types, strict?
};

class FuncInfoChildIterator: public NonUniformDegreeTreeNodeChildIteratorTmpl<FuncInfo> {
public:
  FuncInfoChildIterator(const FuncInfo *fi, bool firstToLast = true) :
	NonUniformDegreeTreeNodeChildIteratorTmpl<FuncInfo>(fi, firstToLast) {};
};

class FuncInfoIterator: public NonUniformDegreeTreeIteratorTmpl<FuncInfo> {
public:
  FuncInfoIterator(const FuncInfo* fi, TraversalOrder torder = PreOrder,
				   NonUniformDegreeTreeEnumType how =
				     NON_UNIFORM_DEGREE_TREE_ENUM_ALL_NODES) :
	NonUniformDegreeTreeIteratorTmpl<FuncInfo>(fi, torder, how) {};
};

}

#endif

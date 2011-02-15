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

// File: LibraryFuncInfo.h
//
// Annotation for a closure in R's library.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ANNOTATION_LIBRARY_FUNC_INFO_H
#define ANNOTATION_LIBRARY_FUNC_INFO_H

#include <set>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationBase.h>
#include <analysis/LexicalScope.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class LibraryFuncInfo : public AnnotationBase
{
public:
  explicit LibraryFuncInfo(SEXP name, SEXP defn);
  virtual ~LibraryFuncInfo();
  
  virtual AnnotationBase * clone();
  
  static PropertyHndlT handle();
  
  // ----- basic information -----

  SEXP get_sexp() const;

  /// cell containing first R name assigned
  SEXP get_first_name_c() const;

  /// name of C function
  const std::string & get_c_name(); // not const: fills in m_c_name if empty

  /// name of C variable storing the closure (CLOSXP)
  const std::string & get_closure() const;

  // ----- arguments -----

  unsigned int get_num_args() const;
  SEXP get_args() const; 
  bool is_arg(SEXP sym) const;
  
  /// get the cell containing the formal arg at the given position. Indexed from 1.
  SEXP get_arg(int position) const;

  /// find the numerical position of the formal with the given name. Indexed from 1.
  int find_arg_position(char * name) const;
  int find_arg_position(std::string name) const;

  // has variable arguments
  bool get_has_var_args() const;

  // ----- context -----
  bool requires_context() const;

  // ----- debugging -----

  virtual std::ostream & dump(std::ostream & os) const;

private:
  void perform_analysis();

private:
  unsigned int m_num_args;         // number of known arguments
  bool m_has_var_args;             // variable number of arguments (uses "...")
  std::string m_c_name;            // C linkage name
  const std::string m_closure;     // C closure (CLOSXP) name
  const bool m_requires_context;   // is an R context object needed for the function?
  const SEXP m_sexp;               // function definition
  const SEXP m_first_name_c;       // cell containing name of function at original definition
};

} // namespace RAnnot

#endif  // ANNOTATION_BASIC_FUNC_INFO_H

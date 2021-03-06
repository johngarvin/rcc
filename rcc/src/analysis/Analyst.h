// -*-Mode: C++;-*-
#ifndef ANALYST_H
#define ANALYST_H

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

// File: Analyst.h
//
// Singleton manager for annotations.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <ostream>

#include <OpenAnalysis/Utils/OA_ptr.hpp>

#include <include/R/R_RInternals.h>

#include <analysis/IRInterface.h>
#include <analysis/LexicalScope.h>

class R_Analyst {

  // implement Singleton pattern
public:
  static R_Analyst * instance(SEXP _program); // regular Singleton: construct or return
  static R_Analyst * instance();  // only get the existing instance; error if not instantiated

public:
  /// Perform analysis.
  void perform_analysis();

  OA::OA_ptr<R_IRInterface> get_interface();

  SEXP get_program();

  const LexicalScope * get_library_scope();
  const LexicalScope * get_global_scope();

  //  ----- debugging -----

  void dump_scope_tree(std::ostream &);

  void dump_cfg(std::ostream &, SEXP proc);
  void dump_all_cfgs(std::ostream &);

private:
  /// construct an R_Analyst by providing an SEXP representing the whole program
  explicit R_Analyst(SEXP _program);

private:
  static R_Analyst * s_instance;

private:
  SEXP m_program;
  OA::OA_ptr<R_IRInterface> m_interface;

  const LexicalScope * m_library_scope;
  const LexicalScope * m_global_scope;
};

#endif

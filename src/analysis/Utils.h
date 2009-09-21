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

// File: Utils.h
//
// Simple utilities for analyzing SEXPs.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef UTILS_H
#define UTILS_H

#include <string>

#include <include/R/R_Defn.h>

bool is_assign_prim(const SEXP e);
bool is_local_assign_prim(const SEXP e);
bool is_free_assign_prim(const SEXP e);
bool is_local_assign(const SEXP e);
bool is_free_assign(const SEXP e);
bool is_assign(const SEXP e);
bool is_simple_assign(const SEXP e);
SEXP assign_lhs_c(const SEXP e);
SEXP assign_rhs_c(const SEXP e);

bool is_fundef(const SEXP e);
SEXP fundef_args(const SEXP e);
SEXP fundef_body_c(const SEXP e);
bool is_closure(const SEXP e);
SEXP closure_args(const SEXP e);
SEXP closure_body(const SEXP e);
SEXP closure_body_c(const SEXP e);
bool is_procedure(const SEXP e);
SEXP procedure_args(const SEXP e);
SEXP procedure_body(const SEXP e);
SEXP procedure_body_c(const SEXP e);

bool is_struct_field(const SEXP e);
SEXP struct_field_lhs_c(const SEXP e);
SEXP struct_field_rhs_c(const SEXP e);
bool is_simple_subscript(const SEXP e);
bool is_subscript(const SEXP e);
SEXP subscript_lhs_c(const SEXP e);
SEXP subscript_first_sub_c(const SEXP e);
SEXP subscript_subs(const SEXP e);
bool is_const(const SEXP e);
bool is_var(const SEXP e);
bool is_symbol(const SEXP e);  // same as is_var
std::string var_name(const SEXP e);
bool is_library(const SEXP e);
SEXP library_value(const SEXP e);
bool is_cons(const SEXP e);
bool is_string(const SEXP e);
bool is_call(const SEXP e);
SEXP call_lhs(const SEXP e);
SEXP call_args(const SEXP e);
// get the cons containing nth arg, indexed from 1
SEXP call_nth_arg_c(const SEXP e, int n);

bool is_if(const SEXP e);
SEXP if_cond_c(const SEXP e);
SEXP if_truebody_c(const SEXP e);
SEXP if_falsebody_c(const SEXP e);
bool is_return(const SEXP e);
bool is_break(const SEXP e);
bool is_stop(const SEXP e);
bool is_next(const SEXP e);

bool is_loop(const SEXP e);
bool is_loop_header(const SEXP e);
bool is_loop_increment(const SEXP e);
SEXP loop_body_c(const SEXP e);
bool is_for(const SEXP e);
SEXP for_iv_c(const SEXP e);
SEXP for_range_c(const SEXP e);
SEXP for_body_c(const SEXP e);
bool is_for_colon(const SEXP e);
bool is_while(const SEXP e);
SEXP while_cond_c(const SEXP e);
SEXP while_body_c(const SEXP e);
bool is_repeat(const SEXP e);
SEXP repeat_body_c(const SEXP e);

bool is_paren_exp(const SEXP e);
SEXP paren_body_c(const SEXP e);
bool is_curly_list(const SEXP e);
SEXP curly_body(const SEXP e);

bool is_rcc_assert_def(const SEXP e);
bool is_rcc_assertion(const SEXP e);
bool is_rcc_assert(const SEXP e);
bool is_rcc_assert_sym(const SEXP e);
bool is_rcc_assert_exp(const SEXP e);
bool is_assert_sym(const SEXP e);
bool is_value_assert(const SEXP e);

#endif

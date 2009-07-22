/*  -*- Mode: C -*-
 *
 *  Copyright (c) 2008 Rice University 
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *  File: rcc_lib.h
 *
 *  Library of functions for RCC-generated code to call at runtime.
 *
 *  Author: John Garvin (garvin@cs.rice.edu)
 */

void my_init_memory(SEXP mem, int n);
SEXP tagged_cons(SEXP car, SEXP tag, SEXP cdr);
Rcomplex mk_complex(double r, double i);
SEXP rcc_cons(SEXP car, SEXP cdr, int unp_car, int unp_cdr);
Rboolean my_asLogicalNoNA(SEXP s);
SEXP R_binary(SEXP call, SEXP op, SEXP x, SEXP y);
SEXP R_unary(SEXP call, SEXP op, SEXP x);
SEXP rcc_do_arith(SEXP call, SEXP op, SEXP args, SEXP env);
SEXP rcc_subassign_0(SEXP x,  SEXP y);
SEXP rcc_subassign_1(SEXP x, SEXP sub, SEXP y);
SEXP rcc_subassign_cons(SEXP x, SEXP subs, SEXP y);
SEXP rcc_subassign_varargs(SEXP x, SEXP y, int nsubs, ...);
SEXP rcc_promise_args(SEXP args, SEXP rho);

/*  Given a cons cell arg_c containing an actual argument list, return
    an R_varloc_t representing the location of the argument in its
    environment. Currently this is very easy to do, because the R
    implementation uses argument lists as environment frames, and the
    location is just the given cons cell recast as an R_varloc_t
    type. Change this function if the R implementation changes. */
R_varloc_t get_R_location(SEXP arg_c);

/*  Make a promise in already-evaluated form. The promise's value is
    the given value; body and environment are null */
SEXP make_thunked_promise(SEXP value);

SEXP applyClosureNoMatching(SEXP call, SEXP op, SEXP arglist, SEXP rho, SEXP suppliedenv);

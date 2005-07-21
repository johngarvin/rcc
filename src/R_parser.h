/* Copyright (c) 2003-2004 John Garvin 
 *
 * July 11, 2003 
 *
 * Parses R code, turns into C code that uses internal R functions.
 * Attempts to output some code in regular C rather than using R
 * functions.
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#include <stdio.h>

/* Why isn't this handled in IOStuff.h? */
#ifndef R_IOSTUFF_H
#  define R_IOSTUFF_H
#  include <IOStuff.h>
#endif

#include <Parse.h>
extern int Rf_initialize_R(int argc, char **argv);
extern void setup_Rmainloop(void);

void init_R();
void parse_R(FILE *in_file, SEXP *exps[]);
SEXP parse_R_as_function(FILE *in_file);

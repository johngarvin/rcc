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

#include <list>
#include <iostream>

#include "util.h"

using namespace std;

extern "C" {

  // Why isn't this handled in IOStuff.h?
#ifndef R_IOSTUFF_H
#  define R_IOSTUFF_H
#  include <IOStuff.h>
#endif
#include <Parse.h>
extern int Rf_initialize_R(int argc, char **argv);
extern void setup_Rmainloop(void);

} //extern "C"

int parse_R(std::list<SEXP> & e, char *inFile);

/* Copyright (c) 2003 John Garvin 
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

#include <iostream>
#include <sstream>
#include <string>
#include <map>

using namespace std;

extern "C" {

#include <Defn.h>

} //extern "C"

// Prevent conflict with basicstring::{append, length} on Alpha
#undef append
#undef length

string make_symbol(SEXP e);
string make_type(int t);
string indent(string str);
string i_to_s(const int i);
string d_to_s(double d);
string c_to_s(Rcomplex c);
string escape(string str);
string make_c_id(string s);
string quote(string str);
string unp(string str);
string strip_suffix(string str);
int filename_pos(string str);
void err(string message);

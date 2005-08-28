/* Copyright (c) 2003 John Garvin
 *
 * Preliminary version v06, July 11, 2003 
 *
 * get_name maps R internal function table indices to R function names.
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

#include <include/R/R_Defn.h>
#include <GetName.h>

struct map {
  char *name;
  CCODE cfun;
};

const char *get_name(int n) {
  if (n >= 0 && n < R_FunTab_NumEntries) {
    return R_FunTab[n].cfun_name;
  } else {
    fprintf (stderr, "get_name: Illegal name index (%d)\n", n);
    return ("illegal_name");
  }
}


// -*- Mode: C++ -*-
//
// Copyright (c) 2009 Rice University
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

// File: CallType.h
//
// For a procedure call, gives information: whether it's a call to a user
// procedure, library procedure, etc.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef RCC_CALL_TYPE_H
#define RCC_CALL_TYPE_H

#include <include/R/R_Defn.h>

typedef enum {
  CALL_NON_SYMBOL,       // non-symbol expression, e.g. a[b](foo, bar)
  CALL_USER_MULTIPLE,    // user-defined procedure defined more than once
  CALL_USER_SINGLETON,   // user-defined procedure defined once
  CALL_LIBRARY_CLOSURE,  // library procedure with CLOSXP value
  CALL_LIBRARY_BUILTIN,  // library procedure with BUILTINSXP value
  CALL_LIBRARY_SPECIAL,  // library procedure with SPECIALSXP value 
  CALL_UNKNOWN           // unknown or unbound
} CallType;

CallType get_call_type(SEXP call);

#endif

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

// File: CodeGenUtils.h
//
// Utilities for emitting C code.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef CODE_GEN_UTILS_H
#define CODE_GEN_UTILS_H

#include <string>

#include <include/Protection.h>

//------------------------------------------------------------------------------
// emit conditional: if(expn) 
//------------------------------------------------------------------------------
std::string emit_logical_if(const std::string expn); 


//------------------------------------------------------------------------------
// emit conditional: if(expn) stmt;
//------------------------------------------------------------------------------
std::string emit_logical_if_stmt(const std::string expn, 
				 const std::string stmt); 


//------------------------------------------------------------------------------
// emit assignment: lhs = rhs;
//------------------------------------------------------------------------------
std::string emit_assign(const std::string lhs, const std::string rhs, 
			Protection resultProtected = Unprotected); 

std::string emit_prot_assign(const std::string lhs, const std::string rhs);

//------------------------------------------------------------------------------
// emit call: fname(...)
//    note: sigh. arguments cannot be specified by varargs list for strings
//------------------------------------------------------------------------------
std::string emit_call0(const std::string fname);

std::string emit_call1(const std::string fname, const std::string arg1);

std::string emit_call2(std::string fname, const std::string arg1, 
		       const std::string arg2);

std::string emit_call3(std::string fname, const std::string arg1,
		       const std::string arg2, const std::string arg3);

std::string emit_call4(std::string fname, const std::string arg1,
		       const std::string arg2, const std::string arg3,
		       const std::string arg4);

//------------------------------------------------------------------------------
// emit call statement: fname(arg1, arg2, ...);
//    note: arguments are specified as varargs list
//          all will be interpreted as strings 
//    note: statement terminated 
//------------------------------------------------------------------------------
// std::string emit_call_stmt(const std::string fname, ...);

std::string emit_unprotect(std::string code);

std::string emit_in_braces(std::string code, bool balanced = true);

std::string emit_decl(std::string var);
std::string emit_static_decl(std::string var);

#endif

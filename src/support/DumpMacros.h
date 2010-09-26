// -*-Mode: C++ -*-
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

// File: DumpMacros.h
//
// a set of stylized macros for variable and object dumping 
//
// Author: John Mellor-Crummey (johnmc@cs.rice.edu)

#ifndef DUMP_MACROS_H
#define DUMP_MACROS_H

//************************* System Include Files ****************************

#include <ostream>

#include <support/StringUtils.h>

//************************* Macro Definitions *******************************


#define beginObjDump(os, name) os << "{ " << #name << ":" << std::endl

#define dumpVarStart(os, var) os << "{ " << #var << " = " 

#define dumpVarEnd(os, var) os << "}" << std::endl

#define dumpVar(os, var) \
  dumpVarStart(os, var); \
  os << var << " "; \
  dumpVarEnd(os, var)

#define dumpName(os, var)     \
  dumpVarStart(os, var);      \
  os << type_name(var) << " "; \
  dumpVarEnd(os, var)

#define dumpPtr(os, ptr) \
  dumpVarStart(os, ptr); \
  os << (void *) ptr;    \
  dumpVarEnd(os, ptr)

#define dumpSEXP(os, sexp) \
  dumpVarStart(os, sexp);  \
  os << to_string(sexp) << " ";   \
  dumpVarEnd(os, sexp)

#define dumpObj(os, obj) \
  dumpVarStart(os, obj); os << std::endl; \
  obj->dump(os); \
  dumpVarEnd(os, obj)

#define dumpString(os, str) \
  dumpVarStart(os, str);    \
  os << str;                \
  dumpVarEnd(os, str)

#define endObjDump(os, name) os << "}" << std::endl; os.flush()

#endif

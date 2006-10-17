// -*- Mode: C++ -*-
//
// Copyright (c) 2003-2006 Rice University
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

// File: Main.h
//
// Main RCC program. Processes command line arguments, initializes,
// calls the R parser, calls analysis routines, and outputs the result.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef MAIN_H
#define MAIN_H

#define __USE_STD_IOSTREAM

#include <fstream>
#include <string>

extern "C" {

#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>

} //extern "C"

#include <include/R/R_RInternals.h>

#include <support/StringUtils.h>
#include <support/Parser.h>

#include <analysis/Analyst.h>
#include <analysis/Utils.h>

#include <GetName.h>
#include <Visibility.h>
#include <Macro.h>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

bool is_special(std::string func);

static void arg_err();
static void set_funcs(int argc, char *argv[]);
std::string make_symbol(SEXP e);

#endif

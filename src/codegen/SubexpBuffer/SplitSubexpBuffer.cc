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

// File: SplitSubexpBuffer.cc
//
// A C code buffer split into multiple curly-brace scopes to be more
// compiler-friendly.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <support/StringUtils.h>

#include "SplitSubexpBuffer.h"

//--------------------------------------------------------------------
// SplitSubexpBuffer methods
//--------------------------------------------------------------------

SplitSubexpBuffer::SplitSubexpBuffer(std::string pref /* = "v" */, 
				     bool is_c        /* = FALSE */,
				     int thr          /* = 300 */,
				     std::string is   /* = "init" */)
  : SubexpBuffer(pref, is_c), threshold(thr), init_str(is) {
  init_fns = 0;
}

void SplitSubexpBuffer::finalize() {
  flush_defs();
}

void SplitSubexpBuffer::append_defs(std::string d) {
  split_defs += d;
}

int SplitSubexpBuffer::defs_location() {
  flush_defs();
  return edefs.length();
}

void SplitSubexpBuffer::insert_def(int loc, std::string d) { 
  flush_defs();
  edefs.insert(loc, d); 
}

unsigned int SplitSubexpBuffer::get_n_inits() {
  return init_fns;
}

std::string SplitSubexpBuffer::get_init_str() {
  return init_str;
}

std::string SplitSubexpBuffer::new_var() {
  prot++;
  return new_var_unp();
}

std::string SplitSubexpBuffer::new_var_unp() {
  if ((SubexpBuffer::n % threshold) == 0) {
    flush_defs();
  }
  return prefix + i_to_s(SubexpBuffer::n++);
}

std::string SplitSubexpBuffer::new_var_unp_name(std::string name) {
  return new_var_unp() + "_" + make_c_id(name);
}

void SplitSubexpBuffer::flush_defs() { 
  if (split_defs.length() > 0) {
    edefs += "\n";
    if (is_const) {
      decls += "static ";
      edefs += "static ";
    }
    decls += "void " + init_str + i_to_s(init_fns) + "();\n";
    edefs += "void " + init_str + i_to_s(init_fns) + "() {\n";
    edefs += indent(split_defs);
    edefs += "}\n";
    split_defs = "";
    init_fns++;
  }
}

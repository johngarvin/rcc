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

// File: SplitSubexpBuffer.h
//
// A C code buffer split into multiple curly-brace scopes to be more compiler-friendly.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef SPLIT_SUBEXP_BUFFER_H
#define SPLIT_SUBEXP_BUFFER_H

#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

/// Huge functions are hard on compilers like gcc. To generate code
/// that goes down easy, we split up the constant initialization into
/// several functions.
class SplitSubexpBuffer : public SubexpBuffer {
public:
  explicit SplitSubexpBuffer(std::string pref = "v", bool is_c = false, int thr = 300, std::string is = "init");

  static SplitSubexpBuffer global_constants;

  virtual void finalize();
  void virtual append_defs(std::string d);
  int virtual defs_location();
  void virtual insert_def(int loc, std::string d);
  unsigned int get_n_inits();
  std::string get_init_str();
  virtual std::string new_var();
  virtual std::string new_var_unp();
  virtual std::string new_var_unp_name(std::string name);

private:
  const unsigned int threshold;
  const std::string init_str;
  unsigned int init_fns;
  std::string split_defs;
  void flush_defs();
};

#endif

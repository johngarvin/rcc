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

// File: LoopContext.h
//
// Represents a loop for code generation.
//
// Author: John Mellor-Crummey (johnmc@cs.rice.edu)

#ifndef LoopContext_h
#define LoopContext_h

#include <string>

class LoopContext {
public:
  static LoopContext *Top();
public:
  explicit LoopContext();
  ~LoopContext();
  std::string breakLabel();
  std::string doBreak();

private:
  std::string mContextName;
  LoopContext *enclosing;

private:
  static unsigned int mContextId;
  static LoopContext *top; 
};

#endif

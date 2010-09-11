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

// File: LoopContext.cc
//
// (description)
//
// Author: John Mellor-Crummey (johnmc@cs.rice.edu)

#include <LoopContext.h>
#include <support/StringUtils.h>

using namespace std;

unsigned int LoopContext::mContextId = 0;
LoopContext * LoopContext::top = NULL;

LoopContext * LoopContext::Top()
{
  return top;
}

LoopContext::LoopContext()
{
  mContextName = "loop_" + i_to_s(mContextId++);

  // link with chain of enclosing contexts 
  enclosing = top;
  top = this;
}

LoopContext::~LoopContext()
{
  top = enclosing; 
}

string LoopContext::breakLabel()
{
  return "break_" + mContextName; 
}
  
string LoopContext::doBreak()
{
  return "goto " + breakLabel();
}

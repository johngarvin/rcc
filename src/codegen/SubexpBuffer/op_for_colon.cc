// -*- Mode: C++ -*-
//
// Copyright (c) 2008 Rice University
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

// File: op_for_colon.cc
//
// Output a for loop with a colon expression as the range.
// 
// Example:
// for (i in (n-1):(m+k/2)) ...
//
// Author: John Garvin (garvin@cs.rice.edu)


#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

using namespace std;

Expression SubexpBuffer::op_for_colon(SEXP e, string rho,
				      ResultStatus resultStatus)
{
  //TODO: write special case; now just defaulting to regular for loop
  return op_for(e, rho, resultStatus);
}

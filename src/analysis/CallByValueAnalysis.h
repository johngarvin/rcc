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

// File: CallByValueAnalysis.h
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef CALL_BY_VALUE_ANALYSIS_H
#define CALL_BY_VALUE_ANALYSIS_H

#include <include/R/R_RInternals.h>

namespace RAnnot {
  class SideEffect;
  class FuncInfo;
  class FormalArgInfo;
}

class CallByValueAnalysis {
public:
  CallByValueAnalysis();
  void perform_analysis();

private:
  RAnnot::SideEffect * compute_pre_debut_side_effect(RAnnot::FuncInfo * fi, RAnnot::FormalArgInfo * formal);
};

#endif

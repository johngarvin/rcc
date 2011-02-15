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

// File: FormalArgInfo.cc
//
// Annotation for a formal argument of a function.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <assert.h>
#include <ostream>

#include <support/DumpMacros.h>

#include <analysis/FormalArgInfoAnnotationMap.h>
#include <analysis/PropertyHndl.h>

#include "FormalArgInfo.h"

namespace RAnnot {

//****************************************************************************
// FormalArgInfo
//****************************************************************************

FormalArgInfo::FormalArgInfo(SEXP sexp)
  : m_sexp(sexp),
    m_default(CAR(sexp) == R_MissingArg ? 0 : CAR(sexp)),
    m_name(TAG(sexp) == R_NilValue ? 0 : TAG(sexp)),
    m_is_value(false),
    m_is_strict(false)
{
}

FormalArgInfo::~FormalArgInfo() {
}

FormalArgInfo * FormalArgInfo::clone() {
  return 0;
}

bool FormalArgInfo::is_strict() const {
  return m_is_strict;
}

void FormalArgInfo::set_is_strict(bool x) {
  m_is_strict = x;
}

bool FormalArgInfo::has_default() const {
  return (m_default != 0);
}

SEXP FormalArgInfo::get_default() const {
  assert(m_default != 0);
  return m_default;
}

bool FormalArgInfo::is_named() const {
  return (m_name != 0);
}

SEXP FormalArgInfo::get_name() const {
  assert(m_name != 0);
  return m_name;
}

SEXP FormalArgInfo::get_sexp() const {
  return m_sexp;
}

std::ostream & FormalArgInfo::dump(std::ostream & os) const
{
  beginObjDump(os, FormalArgInfo);
  SEXP name = TAG(m_sexp);
  dumpSEXP(os, name);
  dumpVar(os, m_is_value);
  dumpVar(os, m_is_strict);
  endObjDump(os, FormalArgInfo);
}

PropertyHndlT FormalArgInfo::handle() {
  return FormalArgInfoAnnotationMap::handle();
}

}

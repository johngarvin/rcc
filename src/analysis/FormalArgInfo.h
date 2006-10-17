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

// File: FormalArgInfo.h
//
// Annotation for a formal argument of a function.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ANNOTATION_FORMAL_ARG_INFO_H
#define ANNOTATION_FORMAL_ARG_INFO_H

#include <analysis/AnnotationBase.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

// ---------------------------------------------------------------------------
// FormalArgInfo
// ---------------------------------------------------------------------------
class FormalArgInfo
  : public AnnotationBase
{
public:
  FormalArgInfo();
  virtual ~FormalArgInfo();

  static PropertyHndlT handle();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual FormalArgInfo* clone() { return new FormalArgInfo(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;
  
  bool isValue() { return isvalue; }
  void setIsValue(bool _isvalue) { isvalue = _isvalue; }

private:
  bool isvalue; // value/promise
};


}

#endif

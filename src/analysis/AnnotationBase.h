// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/AnnotationBase.h,v 1.3 2006/10/17 22:09:49 garvin Exp $

#ifndef ANNOTATION_BASE_H
#define ANNOTATION_BASE_H

// * BeginCopyright *********************************************************
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
// *********************************************************** EndCopyright *

// File: AnnotationBase.h
//
// Abstract base class for all R Annotations
//
// Author: John Garvin (garvin@cs.rice.edu)

//************************* System Include Files ****************************

#include <iostream>

//**************************** R Include Files ******************************

// R sexp includes

//*************************** User Include Files ****************************

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RAnnot {

class AnnotationBase
{
public:
  explicit AnnotationBase();
  virtual ~AnnotationBase();
  
  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual AnnotationBase* clone() = 0;
  
  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  std::ostream& dumpCout() const; // overloading can confuse debuggers
  virtual std::ostream& dump(std::ostream& os) const = 0;
  
protected:
private:

};


//****************************************************************************

} // end of RAnnot namespace

#endif

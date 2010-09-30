// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/AnnotationMap.h,v 1.9 2007/12/12 00:46:14 garvin Exp $

#ifndef ANNOTATION_MAP_H
#define ANNOTATION_MAP_H

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

// File: AnnotationMap.cc
//
// Base class for annotation maps, which map entities in the program
// to annotations.
//
// Author: John Garvin (garvin@cs.rice.edu)

//************************* System Include Files ****************************

#include <ostream>
#include <map>

//**************************** R Include Files ******************************

#include <include/R/R_RInternals.h>

//*************************** User Include Files ****************************

#include <analysis/AnnotationBase.h>

namespace RAnnot {

//****************************************************************************
// R Property Information
//****************************************************************************

// ---------------------------------------------------------------------------
// AnnotationMap: A mapping of IRHandles to Annotations.
// ---------------------------------------------------------------------------
class AnnotationMap
{  
public:
  // -------------------------------------------------------
  // type definitions
  // -------------------------------------------------------
  typedef SEXP MyKeyT;
  typedef RAnnot::AnnotationBase * MyMappedT;
  typedef std::map<MyKeyT, MyMappedT>::iterator iterator;
  typedef std::map<MyKeyT, MyMappedT>::const_iterator const_iterator;

  // -------------------------------------------------------
  // constructor/destructor
  // -------------------------------------------------------
  explicit AnnotationMap();
  virtual ~AnnotationMap();

  // -------------------------------------------------------
  // iterators
  // -------------------------------------------------------
  virtual iterator begin() = 0;
  virtual const_iterator begin() const = 0;
  virtual iterator end() = 0;
  virtual const_iterator end() const = 0;

  // -------------------------------------------------------
  // get the annotation given a key (performs analysis if necessary)
  // -------------------------------------------------------
  //  virtual MyMappedT & operator[](const MyKeyT & k) = 0; // TODO: remove when refactoring is done
  virtual void put(const MyKeyT & k, MyMappedT value) = 0;
  virtual MyMappedT get(const MyKeyT & k) = 0;
  virtual bool is_computed() const = 0;
  virtual bool computation_in_progress() const = 0;
  
  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream & dump_cout() const;
  virtual std::ostream & dump(std::ostream & os) const;

  // prevent cloning
private:
  AnnotationMap(const AnnotationMap &);
  AnnotationMap & operator=(const AnnotationMap &);

};


//****************************************************************************

} // end of RAnnot namespace

#endif

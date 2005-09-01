// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/Attic/AnnotationSet.h,v 1.4 2005/09/01 19:46:55 garvin Exp $

#ifndef ANNOTATION_SET_HPP
#define ANNOTATION_SET_HPP

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/Attic/AnnotationSet.h,v $
//
// Purpose:
//   [The purpose of this file]
//
// Description:
//   [The set of functions, macros, etc. defined in the file]
//
//***************************************************************************

//************************* System Include Files ****************************

#include <iostream>
#include <map>

//**************************** R Include Files ******************************

#include <include/R/R_RInternals.h>

//*************************** User Include Files ****************************

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include "Annotation.h"

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RAnnot {

//****************************************************************************
// R Property Information
//****************************************************************************

// ---------------------------------------------------------------------------
// AnnotationSet: A mapping of IRHandles to Annotations.
// ---------------------------------------------------------------------------
class AnnotationSet
  : public std::map<OA::IRHandle, RAnnot::AnnotationBase*>
{  
public:
  // -------------------------------------------------------
  // constructor/destructor
  // -------------------------------------------------------
  AnnotationSet(bool ownsAnnotations = true); 
  ~AnnotationSet();

  // -------------------------------------------------------
  // cloning (proscribe by hiding copy constructor and operator=)
  // -------------------------------------------------------

  // -------------------------------------------------------
  // iterator, find/insert, etc 
  // -------------------------------------------------------
  // use inherited std::multimap routines
  
  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  std::ostream& dumpCout() const;
  std::ostream& dump(std::ostream& os) const;
  
private:
  AnnotationSet(const AnnotationSet& x);
  AnnotationSet& operator=(const AnnotationSet& x) { return *this; }

private:
  bool mOwnsAnnotations; // does set own all AnnotationBases within map?
};


//****************************************************************************

} // end of RAnnot namespace

#endif

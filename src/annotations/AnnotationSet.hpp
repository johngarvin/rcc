// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/annotations/Attic/AnnotationSet.hpp,v 1.4 2005/08/26 14:14:11 johnmc Exp $

#ifndef ANNOTATION_SET_HPP
#define ANNOTATION_SET_HPP

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/annotations/Attic/AnnotationSet.hpp,v $
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

#include <MyRInternals.h>

//*************************** User Include Files ****************************

#include "Annotation.hpp"

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RAnnot {

//****************************************************************************
// R Property Information
//****************************************************************************

// ---------------------------------------------------------------------------
// AnnotationSet: A multi-mapping of SEXPs to Annotations.  The
// multi-map allows multiple Annotations to be associated with the
// same SEXP.  For example a SEXP variable defintion might be annotated
// with a RAnnot::Var and a RAnnot::VarInfo.
// ---------------------------------------------------------------------------
class AnnotationSet
  : public std::multimap<SEXP, RAnnot::AnnotationBase*>
{  
public:
  // -------------------------------------------------------
  // constructor/destructor
  // -------------------------------------------------------
  AnnotationSet();
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
  // owns all AnnotationBases within map
};


//****************************************************************************

} // end of RAnnot namespace

#endif

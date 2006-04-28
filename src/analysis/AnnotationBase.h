// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/AnnotationBase.h,v 1.2 2006/04/28 09:03:35 garvin Exp $

#ifndef ANNOTATION_BASE_H
#define ANNOTATION_BASE_H

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/AnnotationBase.h,v $
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

//**************************** R Include Files ******************************

// R sexp includes

//*************************** User Include Files ****************************

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RAnnot {

//****************************************************************************
// AnnotationBase
//****************************************************************************

// ---------------------------------------------------------------------------
// AnnotationBase: Abstract base class for all R Annotations
// ---------------------------------------------------------------------------
class AnnotationBase
{
public:
  AnnotationBase();
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

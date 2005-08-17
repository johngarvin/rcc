// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/annotations/Attic/AnnotationBase.hpp,v 1.1 2005/08/17 19:01:14 johnmc Exp $

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/annotations/Attic/AnnotationBase.hpp,v $
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


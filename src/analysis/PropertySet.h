// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/PropertySet.h,v 1.1 2005/08/29 18:18:21 johnmc Exp $

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/PropertySet.h,v $
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

//*************************** User Include Files ****************************

#include "AnnotationSet.h"

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RProp {

//****************************************************************************
// R Property Information
//****************************************************************************

typedef const char* const* PropertyHndlT;

// ---------------------------------------------------------------------------
// PropertySet: Associates arbitrary property names with
// AnnotationSets.  For example, a property might be the result of an
// analysis pass over the R AST.
// ---------------------------------------------------------------------------
class PropertySet
  : public std::map<PropertyHndlT, RAnnot::AnnotationSet*>
{
public:
  // -------------------------------------------------------
  // constructor/destructor
  // -------------------------------------------------------
  PropertySet();
  ~PropertySet();

  // -------------------------------------------------------
  // cloning (proscribe by hiding copy constructor and operator=)
  // -------------------------------------------------------
  
  // -------------------------------------------------------
  // iterator, find/insert, etc 
  // -------------------------------------------------------
  // use inherited std::map routines

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  std::ostream& dumpCout() const;
  std::ostream& dump(std::ostream& os) const;

private:
  PropertySet(const PropertySet& x);
  PropertySet& operator=(const PropertySet& x) { return *this; }

private:
};


//****************************************************************************

} // end of RProp namespace

// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/PropertySet.h,v 1.4 2005/09/01 19:46:56 garvin Exp $

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

#include "PropertyHndl.h"
#include "AnnotationSet.h"

//*************************** Forward Declarations ***************************

class OA::IRHandle;

//****************************************************************************

namespace RProp {

//****************************************************************************
// R Property Information
//****************************************************************************

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

  void insert(PropertyHndlT propertyName, OA::IRHandle h,
	      RAnnot::AnnotationBase *annot, bool ownsAnnotations);

  RAnnot::AnnotationBase *lookup(PropertyHndlT propertyName, OA::IRHandle h);

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

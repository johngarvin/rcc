// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/PropertySet.h,v 1.7 2006/04/26 22:09:45 garvin Exp $

#ifndef PROPERTY_SET_H
#define PROPERTY_SET_H

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
#include "AnnotationMap.h"

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RProp {

//****************************************************************************
// R Property Information
//****************************************************************************

// ---------------------------------------------------------------------------
// PropertySet: Associates arbitrary property names with
// AnnotationMaps.  For example, a property might be the result of an
// analysis pass over the R AST.
// ---------------------------------------------------------------------------
class PropertySet
  : public std::map<PropertyHndlT, RAnnot::AnnotationMap*>
{
public:
  // -------------------------------------------------------
  // constructor/destructor
  // -------------------------------------------------------
  PropertySet();
  ~PropertySet();

  // -------------------------------------------------------
  // managing annotation maps
  // -------------------------------------------------------

  /// associate the SEXP s with annotation annot for the propertyName
  /// property. FIXME: adding annotations from the outside like this
  /// is bad data hiding. It would be better to create Annotation
  /// objects only inside AnnotationMaps.
  void insert(PropertyHndlT propertyName, SEXP s,
	      RAnnot::AnnotationBase * annot, bool ownsAnnotations);

  /// Look up an annotation associated with an SEXP.
  RAnnot::AnnotationBase * lookup(PropertyHndlT propertyName, SEXP s);

  /// To be called when new AnnotationMaps are created. Register a
  /// property name with an AnnotationMap so the Annotations in the
  /// AnnotationMap can be looked up.
  void add(PropertyHndlT propertyName, RAnnot::AnnotationMap * amap);

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

#endif

// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/PropertySetMgr.h,v 1.4 2006/03/31 16:37:27 garvin Exp $

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/PropertySetMgr.h,v $
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

//*************************** User Include Files ****************************

#include "PropertySet.h"

//*************************** Forward Declarations ***************************

class RAnnot::AnnotationMap;

//****************************************************************************

namespace RProp {

//****************************************************************************
// R Property Manager
//****************************************************************************

// ---------------------------------------------------------------------------
// PropertySetMgr: Demand-driven manager for PropertySets.
// ---------------------------------------------------------------------------
class PropertySetMgr
{
public:
  // Property names
  static PropertyHndlT Prop1;
  static PropertyHndlT Prop2;
  static PropertyHndlT Prop3;

  // function type for annotation-computation functions
  typedef RAnnot::AnnotationMap* (*AnnotationComputationFn_t)(SEXP);

  // FIXME: need some way of registering property names and
  // computation routines.
  
public:
  // -------------------------------------------------------
  // constructor/destructor
  // -------------------------------------------------------

  // create my own PropertySet
  PropertySetMgr();
  
  // assume control and ownership of 'x'
  PropertySetMgr(PropertySet* x);
  
  ~PropertySetMgr();

  // -------------------------------------------------------
  // cloning (proscribe by hiding copy constructor and operator=)
  // -------------------------------------------------------

  // -------------------------------------------------------
  // member data manipulation
  // -------------------------------------------------------

  // get annotations on demand
  RAnnot::AnnotationMap* getAnnot(PropertyHndlT prop);

  // property-set (returns non-const set to allow manipulation)
  PropertySet* getPropertySet() const
    { return mSet; }
  
  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  std::ostream& dumpCout() const;
  std::ostream& dump(std::ostream& os) const;

private:
  PropertySetMgr(const PropertySetMgr& x);
  PropertySetMgr& operator=(const PropertySetMgr& x) { return *this; }

private:
  PropertySet* mSet;
};


//****************************************************************************

} // end of RProp namespace

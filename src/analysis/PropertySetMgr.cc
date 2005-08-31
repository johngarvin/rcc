// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/PropertySetMgr.cc,v 1.2 2005/08/31 23:28:25 johnmc Exp $

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/PropertySetMgr.cc,v $
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
#include <string>
#include <cassert>

//**************************** R Include Files ******************************

//*************************** User Include Files ****************************

#include "PropertySetMgr.h"

//*************************** Forward Declarations ***************************

//****************************************************************************
#if 0

namespace RProp {

PropertySetMgr::PropertySetMgr()
{
  mSet = new PropertySet;
}


PropertySetMgr::PropertySetMgr(PropertySet* x)
  : mSet(x)
{
}


PropertySetMgr::~PropertySetMgr()
{
  delete mSet;
}


RAnnot::AnnotationSet* 
PropertySetMgr::getAnnot(PropertyHndlT prop) 
{  
  RAnnot::AnnotationSet* x = NULL;

  PropertySet::iterator it = mSet->find(prop);
  if (it == mSet->end()) {
    // Property has not been computed.  Do so and insert into PropertySet.
    // x = ComputeTheProperty(); // FIXME: who should I call?
    std::pair<PropertySet::iterator, bool> res = 
      mSet->insert(make_pair(prop, x));
    assert(!res.second && "insert() must be successful after find()");
  }
  else {
    // Property has already been computed.
    x = (*it).second;
  }

  return x;
}


std::ostream&
PropertySetMgr::dumpCout() const
{
  dump(std::cout);
}


std::ostream&
PropertySetMgr::dump(std::ostream& os) const
{
  os << "{ PropertySetMgr:\n";
  mSet->dump(os);
  os << "}\n";
  os.flush();
}


} // end of RProp namespace
#endif

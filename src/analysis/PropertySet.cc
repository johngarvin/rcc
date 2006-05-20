// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/PropertySet.cc,v 1.8 2006/05/20 14:50:00 garvin Exp $

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/PropertySet.cc,v $
//
// Purpose:
//   [The purpose of this file]
//
// Description:
//   [The set of functions, macros, etc. defined in the file]
//
//***************************************************************************

//************************* System Include Files ****************************

#include <assert.h>
#include <iostream>
#include <string>

//**************************** R Include Files ******************************

//*************************** User Include Files ****************************

#include <support/RccError.h>

#include "PropertySet.h"

using namespace RAnnot;

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RProp {

PropertySet::PropertySet()
{
}


PropertySet::~PropertySet()
{
  for (iterator it = this->begin(); it != this->end(); ++it) {
    delete it->second;
  }
  this->clear();
}


void PropertySet::insert(PropertyHndlT propertyName, SEXP s,
			 AnnotationBase *annot, bool ownsAnnotations) 
{
  AnnotationMap *annotations = (*this)[propertyName];
  if (annotations == 0) {
    rcc_error("Annotation map not found in PropertySet");
  }
  (*annotations)[reinterpret_cast<OA::irhandle_t>(s)] = annot;
}

AnnotationBase *PropertySet::lookup(PropertyHndlT propertyName, SEXP s)
{
  AnnotationMap *annotations = (*this)[propertyName];
  if (annotations == 0) {
    rcc_error("AnnotationMap not found; possible invalid propertyName " + std::string(propertyName));
  }
  return annotations->get(OA::IRHandle(reinterpret_cast<OA::irhandle_t>(s)));
}

void PropertySet::add(PropertyHndlT propertyName, RAnnot::AnnotationMap * amap)
{
  (*this)[propertyName] = amap;
}


std::ostream&
PropertySet::dumpCout() const
{
  dump(std::cout);
}


std::ostream&
PropertySet::dump(std::ostream& os) const
{
  os << "{ PropertySet:\n";
  for (const_iterator it = this->begin(); it != this->end(); ++it) {
    os << "(" << it->first << " --> " << it->second << ")\n";
    it->second->dump(os);
  }
  os << "}\n";
  os.flush();
}


} // end of RProp namespace

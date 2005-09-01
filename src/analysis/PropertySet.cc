// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/PropertySet.cc,v 1.3 2005/09/01 17:43:06 johnmc Exp $

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
  AnnotationSet *annotations = (*this)[propertyName];
  if (annotations == NULL) {
    annotations = new AnnotationSet(ownsAnnotations);
    (*this)[propertyName] = annotations; 
  } 

  (*annotations)[(OA::irhandle_t) s] = annot;
}


AnnotationBase *PropertySet::lookup(PropertyHndlT propertyName, SEXP s)
{
  AnnotationSet *annotations = (*this)[propertyName];
  if (annotations == NULL) return NULL;
  return (*annotations)[(OA::irhandle_t) s];
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

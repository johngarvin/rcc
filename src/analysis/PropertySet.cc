// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/PropertySet.cc,v 1.4 2005/09/01 19:46:56 garvin Exp $

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

class OA::IRHandle;

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


void PropertySet::insert(PropertyHndlT propertyName, OA::IRHandle h,
			 AnnotationBase *annot, bool ownsAnnotations) 
{
  AnnotationSet *annotations = (*this)[propertyName];
  if (annotations == NULL) {
    annotations = new AnnotationSet(ownsAnnotations);
    (*this)[propertyName] = annotations; 
  } 

  (*annotations)[h] = annot;
}


AnnotationBase *PropertySet::lookup(PropertyHndlT propertyName, OA::IRHandle h)
{
  AnnotationSet *annotations = (*this)[propertyName];
  if (annotations == NULL) return NULL;
  return (*annotations)[h];
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

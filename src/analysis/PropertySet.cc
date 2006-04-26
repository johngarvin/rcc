// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/PropertySet.cc,v 1.7 2006/04/26 22:09:45 garvin Exp $

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

// FIXME: Delete the following when insert() goes away.
#include <analysis/VarAnnotationMap.h>
#include <analysis/FuncInfoAnnotationMap.h>
#include <analysis/ExpressionInfoAnnotationMap.h>
#include <analysis/FormalArgInfoAnnotationMap.h>

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
#if 0
  if (annotations == 0) {
    // temporary ugly conditional. After refactoring is complete this
    // method will disappear anyway.
    if (propertyName == VarAnnotationMap::handle()) {
      annotations = new VarAnnotationMap(ownsAnnotations);
    } else if (propertyName == FuncInfo::FuncInfoProperty) {
      annotations = new FuncInfoAnnotationMap(ownsAnnotations);
    } else if (propertyName == FormalArgInfo::FormalArgInfoProperty) {
      annotations = new FormalArgInfoAnnotationMap(ownsAnnotations);
    } else if (propertyName == ExpressionInfo::ExpressionInfoProperty) {
      annotations = new ExpressionInfoAnnotationMap(ownsAnnotations);
    } else {
      rcc_error("Unhandled property type " + std::string(propertyName));
    }
    (*this)[propertyName] = annotations; 
  }
#endif
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

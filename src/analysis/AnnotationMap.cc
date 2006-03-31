// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/AnnotationMap.cc,v 1.1 2006/03/31 16:37:26 garvin Exp $

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/AnnotationMap.cc,v $
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

//**************************** R Include Files ******************************

//*************************** User Include Files ****************************

#include "AnnotationMap.h"

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RAnnot {

AnnotationMap::AnnotationMap() {}

AnnotationMap::~AnnotationMap()
{
  /*
  if (mOwnsAnnotations) {
    for (iterator it = this->begin(); it != this->end(); ++it) {
      delete it->second;
    }
  }
  this->clear();
  */
}

std::ostream&
AnnotationMap::dumpCout() const
{
  dump(std::cout);
}


std::ostream&
AnnotationMap::dump(std::ostream& os) const
{
  os << "{ AnnotationMap:\n";
  for (const_iterator it = this->begin(); it != this->end(); ++it) {
    os << "(" << it->first.hval() << " --> " << it->second << ")\n";
    it->second->dump(os);
  }
  os << "}\n";
  os.flush();
}


} // end of RAnnot namespace

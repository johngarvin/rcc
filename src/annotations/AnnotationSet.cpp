// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/annotations/Attic/AnnotationSet.cpp,v 1.2 2005/08/29 09:30:17 garvin Exp $

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/annotations/Attic/AnnotationSet.cpp,v $
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

#include "AnnotationSet.hpp"

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RAnnot {

AnnotationSet::AnnotationSet()
{
}


AnnotationSet::~AnnotationSet()
{
  for (iterator it = this->begin(); it != this->end(); ++it) {
    delete it->second;
  }
  this->clear();
}


std::ostream&
AnnotationSet::dumpCout() const
{
  dump(std::cout);
}


std::ostream&
AnnotationSet::dump(std::ostream& os) const
{
  os << "{ AnnotationSet:\n";
  for (const_iterator it = this->begin(); it != this->end(); ++it) {
    os << "(" << it->first.hval() << " --> " << it->second << ")\n";
    it->second->dump(os);
  }
  os << "}\n";
  os.flush();
}


} // end of RAnnot namespace

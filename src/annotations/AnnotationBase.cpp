// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/annotations/Attic/AnnotationBase.cpp,v 1.1 2005/08/17 19:01:14 johnmc Exp $

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/annotations/Attic/AnnotationBase.cpp,v $
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

#include "AnnotationBase.hpp"

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RAnnot {

//****************************************************************************
// AnnotationBase
//****************************************************************************

AnnotationBase::AnnotationBase()
{
}


AnnotationBase::~AnnotationBase()
{
}


std::ostream&
AnnotationBase::dumpCout() const
{
  dump(std::cout);
}


} // end of RAnnot namespace

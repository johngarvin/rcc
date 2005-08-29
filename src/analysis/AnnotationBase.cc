// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/AnnotationBase.cc,v 1.1 2005/08/29 18:04:08 johnmc Exp $

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/AnnotationBase.cc,v $
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

#include "AnnotationBase.h"

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

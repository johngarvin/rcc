// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/Attic/foo.cpp,v 1.1 2005/08/17 19:01:14 johnmc Exp $

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/Attic/foo.cpp,v $
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

#include "PropertySetMgr.hpp"

//*************************** Forward Declarations ***************************

//****************************************************************************

int 
main()
{
  RProp::PropertySetMgr mgr;
  RAnnot::AnnotationSet* annot = NULL; // mgr.getAnnot(PropertySetMgr::Prop1);
  mgr.dump(std::cout);
  return 0;
}

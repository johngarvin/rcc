// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/annotations/Attic/Annotation.cpp,v 1.1 2005/08/17 19:01:14 johnmc Exp $

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/annotations/Attic/Annotation.cpp,v $
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

#include "Annotation.hpp"

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RAnnot {

//****************************************************************************
// Environment
//****************************************************************************

Environment::Environment()
{
}


Environment::~Environment()
{
}


std::ostream&
Environment::dump(std::ostream& os) const
{
  os << "{ Environment:\n";
  for (const_iterator it = this->begin(); it != this->end(); ++it) {
    os << "(" << it->first << " --> " << it->second << ")\n";
  }
  os << "}\n";
  os.flush();
}


//****************************************************************************
// Expression
//****************************************************************************

Expression::Expression()
{
}


Expression::~Expression()
{
}


std::ostream&
Expression::dump(std::ostream& os) const
{
  os << "{ Expression:\n";
  os << "}\n";
  os.flush();
}


//****************************************************************************
// Term
//****************************************************************************

Term::Term()
{
}


Term::~Term()
{
}


std::ostream&
Term::dump(std::ostream& os) const
{
  os << "{ Term:\n";
  os << "}\n";
  os.flush();
}


//****************************************************************************
// Var
//****************************************************************************

Var::Var()
{
}


Var::~Var()
{
}


std::ostream&
Var::dump(std::ostream& os) const
{
  os << "{ Var:\n";
  os << "  mReachingDef:" << mReachingDef->dump(os);
  os << "}\n";
  os.flush();
}


//****************************************************************************
// FuncVar
//****************************************************************************

FuncVar::FuncVar()
{
}


FuncVar::~FuncVar()
{
}


std::ostream&
FuncVar::dump(std::ostream& os) const
{
  os << "{ FuncVar:\n";
  os << "  mIsReachingDefKnown:" << mIsReachingDefKnown;
  os << "}\n";
  os.flush();
}


//****************************************************************************
// Literal
//****************************************************************************

Literal::Literal()
{
}


Literal::~Literal()
{
}


std::ostream&
Literal::dump(std::ostream& os) const
{
  os << "{ Literal:\n";
  os << "}\n";
  os.flush();
}


//****************************************************************************
// VarInfo
//****************************************************************************

VarInfo::VarInfo()
{
}


VarInfo::~VarInfo()
{
}


std::ostream&
VarInfo::dump(std::ostream& os) const
{
  os << "{ VarInfo:\n";
  // FIXME: add implementation
  os << "}\n";
  os.flush();
}


//****************************************************************************
// FuncInfo
//****************************************************************************

FuncInfo::FuncInfo()
{
}


FuncInfo::~FuncInfo()
{
}


std::ostream&
FuncInfo::dump(std::ostream& os) const
{
  os << "{ FuncInfo:\n";
  // FIXME: add implementation
  os << "}\n";
  os.flush();
}


//****************************************************************************
// Phi
//****************************************************************************

Phi::Phi()
{
}


Phi::~Phi()
{
}


std::ostream&
Phi::dump(std::ostream& os) const
{
  os << "{ Phi:\n";
  // FIXME: add implementation
  os << "}\n";
  os.flush();
}


//****************************************************************************
// ArgInfo
//****************************************************************************

ArgInfo::ArgInfo()
{
}


ArgInfo::~ArgInfo()
{
}


std::ostream&
ArgInfo::dump(std::ostream& os) const
{
  os << "{ ArgInfo:\n";
  os << "}\n";
  os.flush();
}


//****************************************************************************
// FormalArg
//****************************************************************************

FormalArg::FormalArg()
{
}


FormalArg::~FormalArg()
{
}


std::ostream&
FormalArg::dump(std::ostream& os) const
{
  os << "{ FormalArg:\n";
  // FIXME: add implementation
  os << "}\n";
  os.flush();
}


} // end of RAnnot namespace

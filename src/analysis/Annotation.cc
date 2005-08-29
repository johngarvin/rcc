// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/Attic/Annotation.cc,v 1.1 2005/08/29 18:04:08 johnmc Exp $

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/Attic/Annotation.cc,v $
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

#include <support/DumpMacros.h>
#include <analysis/Utils.h>

#include "Annotation.h"

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
  beginObjDump(os, Environment);
  for (const_iterator it = this->begin(); it != this->end(); ++it) {
    os << "(" << it->first << " --> " << it->second << ")\n";
  }
  endObjDump(os, Environment);
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
  beginObjDump(os, Expression);
  endObjDump(os, Expression);
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
  beginObjDump(os, Term);
  endObjDump(os, Term);
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
  beginObjDump(os, Var);
  dumpObj(os, mReachingDef);
  endObjDump(os, Var);
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
  beginObjDump(os, FuncVar);
  dumpVar(os, mIsReachingDefKnown);
  endObjDump(os, FuncVar);
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
  beginObjDump(os, Literal);
  endObjDump(os, Literal);
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
  beginObjDump(os, VarInfo);
  // FIXME: add implementation
  endObjDump(os, VarInfo);
}


//****************************************************************************
// FuncInfo
//****************************************************************************

FuncInfo::FuncInfo(FuncInfo *lparent, SEXP name, SEXP defn) :
  mRequiresContext(true), mFirstName(name), mDefn(defn), mLexicalParent(lparent)
{
  mEnv = new Environment();
}


FuncInfo::~FuncInfo()
{
}


void FuncInfo::setRequiresContext(bool requiresContext) 
{ 
  mRequiresContext = requiresContext; 
}


bool FuncInfo::getRequiresContext() 
{ 
  return mRequiresContext;
}



SEXP FuncInfo::get_args() 
{ 
  return CAR(fundef_args_c(mDefn)); 
}

std::ostream&
FuncInfo::dump(std::ostream& os) const
{
  beginObjDump(os, FuncInfo);
  dumpVar(os, mHasVarArgs);
  dumpVar(os, mCName);
  dumpVar(os, mRequiresContext);
  dumpObj(os, mEnv);
  dumpSEXP(os, mFirstName);
  dumpSEXP(os, mDefn);
  dumpPtr(os, mLexicalParent);
  endObjDump(os, FuncInfo);
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
  beginObjDump(os, Phi);
  // FIXME: add implementation
  endObjDump(os, Phi);
  
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
  beginObjDump(os, ArgInfo);
  endObjDump(os, ArgInfo);
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
  beginObjDump(os, FormalArg);
  // FIXME: add implementation
  endObjDump(os, FormalArg);
}


} // end of RAnnot namespace

// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/Attic/Annotation.cc,v 1.14 2006/02/19 02:56:12 garvin Exp $

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
#include <analysis/AnalysisResults.h>
#include <analysis/RequiresContext.h>

#include "Annotation.h"

//*************************** Forward Declarations ***************************

//****************************************************************************


namespace RAnnot {


//****************************************************************************
// SymbolTable
//****************************************************************************

SymbolTable::SymbolTable()
{
}


SymbolTable::~SymbolTable()
{
}


std::ostream&
SymbolTable::dump(std::ostream& os) const
{
  beginObjDump(os, SymbolTable);
  for (const_iterator it = this->begin(); it != this->end(); ++it) {
    os << "(";
    Rf_PrintValue(it->first);
    os << " --> ";
    dumpObj(os, it->second);
    os << ")\n";
  }
  endObjDump(os, SymbolTable);
}


//****************************************************************************
// ExpressionInfo
//****************************************************************************

PropertyHndlT ExpressionInfo::ExpressionInfoProperty = "ExpressionInfo";

ExpressionInfo::ExpressionInfo()
{
}


ExpressionInfo::~ExpressionInfo()
{
}


std::ostream&
ExpressionInfo::dump(std::ostream& os) const
{
  beginObjDump(os, ExpressionInfo);
  SEXP definition = CAR(mDefn);
  dumpSEXP(os, definition);
  MySet_t::iterator var_iter;
  for(var_iter = mVars.begin(); var_iter != mVars.end(); ++var_iter) {
    (*var_iter)->dump(os);
  }
  endObjDump(os, ExpressionInfo);
}


//****************************************************************************
// TermInfo
//****************************************************************************

TermInfo::TermInfo()
{
}


TermInfo::~TermInfo()
{
}


std::ostream&
TermInfo::dump(std::ostream& os) const
{
  beginObjDump(os, TermInfo);
  endObjDump(os, TermInfo);
}


//****************************************************************************
// Var
//****************************************************************************

PropertyHndlT Var::VarProperty = "Var";

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

const std::string typeName(const Var::UseDefT x)
{
  switch(x) {
  case Var::Var_USE: return "USE";
  case Var::Var_DEF: return "DEF";
  }
}

const std::string typeName(const Var::MayMustT x)
{
  switch(x) {
  case Var::Var_MAY: return "MAY";
  case Var::Var_MUST: return "MUST";
  }
}

const std::string typeName(const Var::ScopeT x)
{
  switch(x) {
  case Var::Var_TOP: return "TOP";
  case Var::Var_LOCAL: return "LOCAL";
  case Var::Var_GLOBAL: return "GLOBAL";
  case Var::Var_FREE_ONE_SCOPE: return "FREE_ONE_SCOPE";
  case Var::Var_FREE: return "FREE";
  case Var::Var_INDEFINITE: return "INDEFINITE";
  }
}

//****************************************************************************
// UseVar
//****************************************************************************

UseVar::UseVar()
{
  mUseDefType = Var_USE;
}

UseVar::~UseVar()
{
}

SEXP UseVar::getName() const
{
  return CAR(mSEXP);
}

std::ostream&
UseVar::dump(std::ostream& os) const
{
  beginObjDump(os,UseVar);
  //dumpSEXP(os,mSEXP);
  SEXP name = getName();
  dumpSEXP(os,name);
  dumpName(os,mUseDefType);
  dumpName(os,mMayMustType);
  dumpName(os,mScopeType);
  //dumpPtr(os,mReachingDef);
  dumpPtr(os,mBoundScope);
  dumpName(os,mPositionType);
  endObjDump(os,UseVar);
}

const std::string typeName(const UseVar::PositionT x)
{
  switch(x) {
  case UseVar::UseVar_FUNCTION: return "FUNCTION";
  case UseVar::UseVar_ARGUMENT: return "ARGUMENT";
  }
}

//****************************************************************************
// DefVar
//****************************************************************************

DefVar::DefVar()
{
  mUseDefType = Var_DEF;
}

DefVar::~DefVar()
{
}

SEXP DefVar::getName() const
{
  if (mSourceType == DefVar_ASSIGN) {
    return CAR(mSEXP);
  } else if (mSourceType == DefVar_FORMAL) {
    return TAG(mSEXP);
  } else {
    assert(0);
    return R_NilValue;
  }
}

std::ostream&
DefVar::dump(std::ostream& os) const
{
  beginObjDump(os,DefVar);
  //dumpSEXP(os,mSEXP);
  SEXP name = getName();
  dumpSEXP(os,name);
  dumpName(os,mUseDefType);
  dumpName(os,mMayMustType);
  dumpName(os,mScopeType);
  // dumpPtr(os,mReachingDef);
  dumpPtr(os,mBoundScope);
  dumpName(os,mSourceType);
  endObjDump(os,DefVar);
}

const std::string typeName(const DefVar::SourceT x)
{
  switch(x) {
  case DefVar::DefVar_ASSIGN: return "ASSIGN";
  case DefVar::DefVar_FORMAL: return "FORMAL";
  case DefVar::DefVar_PHI:    return "PHI";
  }
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
  dumpVar(os, mIsSingleton);
  endObjDump(os, VarInfo);
}


//****************************************************************************
// FuncInfo
//****************************************************************************

PropertyHndlT FuncInfo::FuncInfoProperty = "FuncInfo";

FuncInfo::FuncInfo(FuncInfo *lexParent, SEXP name, SEXP defn) :
  mLexicalParent(lexParent),
  mFirstName(name),
  mDefn(defn),
  NonUniformDegreeTreeNodeTmpl<FuncInfo>(lexParent)
{
  mST = new SymbolTable();
  mRequiresContext = functionRequiresContext(defn);
  SEXP args = getArgs();
  for (SEXP e = args; e != R_NilValue; e = CDR(e)) {
    FormalArgInfo* fargInfo = new FormalArgInfo();
    putProperty(FormalArgInfo, e, fargInfo, false);
  }
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
#if 1
  return mRequiresContext;
#else
  return false ; // for now 
#endif
}

SEXP FuncInfo::getArgs() 
{ 
  return CAR(fundef_args_c(mDefn)); 
}

int FuncInfo::findArgPosition(char* name)
{
  SEXP args = getArgs();
  int pos = 1;
  SEXP e;
  for (e = args; e != R_NilValue; e = CDR(e), pos++) {
    char* argName = CHAR(PRINTNAME(INTERNAL(e)));
    if (!strcmp(argName, name)) break;
  }
  assert (e != R_NilValue);
  return pos;
}

SEXP FuncInfo::getArg(int position)
{
  SEXP args = getArgs();
  int p = 1;
  SEXP e;
  for (e = args; e != R_NilValue && p != position; e = CDR(e), p++);
  assert (e != R_NilValue);
  return e;
}

bool FuncInfo::isArgValue(SEXP arg)
{
  FormalArgInfo* fargInfo = getProperty(FormalArgInfo, arg);
  bool isvalue = fargInfo->isValue();
  return isvalue;
}

bool FuncInfo::areAllValue()
{
  bool allvalue = true;
  SEXP args = getArgs();
  for (SEXP e = args; e != R_NilValue && allvalue; e = CDR(e)) {
    if (!isArgValue(e)) allvalue = false;
  }
  return allvalue;
}

void FuncInfo::insertMention(Var * v)
{
  mMentions.insert(v);
}

void FuncInfo::insertCallIn(FuncInfo* fi)
{
  mCallsIn.insert(fi);
}

void FuncInfo::insertCallOut(FuncInfo* fi)
{
  mCallsOut.insert(fi);
}

std::ostream&
FuncInfo::dump(std::ostream& os) const
{
  beginObjDump(os, FuncInfo);
  dumpVar(os, mNumArgs);
  dumpVar(os, mHasVarArgs);
  dumpVar(os, mCName);
  dumpVar(os, mRequiresContext);
  dumpObj(os, mST);
  //dumpObj(os, mCFG);      // can't call CFG::dump; it requires the IRInterface
  dumpSEXP(os, mFirstName);
  dumpSEXP(os, mDefn);
  dumpPtr(os, mLexicalParent);
  os << "Begin mentions:" << std::endl;
  for(mention_iterator i = mMentions.begin(); i != mMentions.end(); ++i) {
    (*i)->dump(os);
  }
  os << "End mentions" << std::endl;
  os << "Begin calls" << std::endl;
  os << "In:" << std::endl;
  call_iterator i;
  for(i = mCallsIn.begin(); i != mCallsIn.end(); ++i) {
    dumpPtr(os, *i);
  }
  os << "Out:" << std::endl;
  for(i = mCallsOut.begin(); i != mCallsOut.end(); ++i) {
    dumpPtr(os, *i);
  }
  os << "End calls" << std::endl;
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
// FormalArgInfo
//****************************************************************************

PropertyHndlT FormalArgInfo::FormalArgInfoProperty = "FormalArgInfo";

FormalArgInfo::FormalArgInfo() 
{
  isvalue = false;
}


FormalArgInfo::~FormalArgInfo()
{
}


std::ostream&
FormalArgInfo::dump(std::ostream& os) const
{
  beginObjDump(os, FormalArgInfo);
  // FIXME: add implementation
  endObjDump(os, FormalArgInfo);
}


} // end of RAnnot namespace

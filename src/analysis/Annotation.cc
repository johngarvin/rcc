// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/Attic/Annotation.cc,v 1.23 2006/05/18 21:19:55 garvin Exp $

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
#include <analysis/VarAnnotationMap.h>
#include <analysis/VarBinding.h>
#include <analysis/FuncInfoAnnotationMap.h>
#include <analysis/ExpressionInfoAnnotationMap.h>
#include <analysis/FormalArgInfoAnnotationMap.h>

#include "Annotation.h"

//*************************** Forward Declarations ***************************

//****************************************************************************


namespace RAnnot {

//****************************************************************************
// ExpressionInfo
//****************************************************************************

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

  os << "Begin mentions:" << std::endl;
  const_var_iterator var_iter;
  for(var_iter = mVars.begin(); var_iter != mVars.end(); ++var_iter) {
    (*var_iter)->dump(os);
  }
  os << "End mentions" << std::endl;

  os << "Begin call sites:" << std::endl;
  const_call_site_iterator cs_iter;
  for(cs_iter = mCallSites.begin(); cs_iter != mCallSites.end(); ++cs_iter) {
    dumpSEXP(os, *cs_iter);
  }
  os << "End call sites" << std::endl;

  endObjDump(os, ExpressionInfo);
}

PropertyHndlT
ExpressionInfo::handle() {
  return ExpressionInfoAnnotationMap::handle();
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
  endObjDump(os, Var);
}

PropertyHndlT
Var::handle() {
  return VarAnnotationMap::handle();
}

//****************************************************************************
// typeName
//****************************************************************************

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
  const_iterator it;
  for(it = beginDefs(); it != endDefs(); ++it) {
    DefVar * def = *it;
    SEXP name = def->getName();
    SEXP rhs = CAR(def->getRhs_c());

    os << "(";
    dumpSEXP(os, name);
    os << " ---> ";
    dumpSEXP(os, rhs);
    os << ")" << std::endl;
  }
  endObjDump(os, VarInfo);
}

//****************************************************************************
// FuncInfo
//****************************************************************************

FuncInfo::FuncInfo(FuncInfo *lexParent, SEXP name, SEXP defn) :
  mLexicalParent(lexParent),
  mFirstName(name),
  mDefn(defn),
  NonUniformDegreeTreeNodeTmpl<FuncInfo>(lexParent)
{
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
  return mRequiresContext;
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

void FuncInfo::insertMention(FuncInfo::MentionT v)
{
  mMentions.push_back(v);
}

void FuncInfo::insertCallSite(FuncInfo::CallSiteT cs)
{
  mCallSites.push_back(cs);
}

std::ostream&
FuncInfo::dump(std::ostream& os) const
{
  beginObjDump(os, FuncInfo);
  dumpPtr(os, this);
  dumpSEXP(os, mFirstName);
  dumpVar(os, mNumArgs);
  dumpVar(os, mHasVarArgs);
  dumpVar(os, mCName);
  dumpVar(os, mRequiresContext);
  //dumpObj(os, mCFG);      // can't call CFG::dump; it requires the IRInterface
  dumpSEXP(os, mDefn);
  dumpPtr(os, mLexicalParent);
  os << "Begin mentions:" << std::endl;
  for(const_mention_iterator i = beginMentions(); i != endMentions(); ++i) {
    Var * v = *i;
    v->dump(os);
    VarBinding * vb = getProperty(VarBinding, (*i)->getMention_c());
    vb->dump(os);
  }
  os << "End mentions" << std::endl;
  endObjDump(os, FuncInfo);
}

PropertyHndlT
FuncInfo::handle() {
  return FuncInfoAnnotationMap::handle();
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

PropertyHndlT
FormalArgInfo::handle() {
  return FormalArgInfoAnnotationMap::handle();
}

} // end of RAnnot namespace

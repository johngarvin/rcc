#include <include/R/R_RInternals.h>

#include <ParseInfo.h>
#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <support/DumpMacros.h>
#include <support/StringUtils.h>

#include <analysis/AnalysisResults.h>
#include <analysis/FormalArgInfo.h>
#include <analysis/FuncInfoAnnotationMap.h>
#include <analysis/RequiresContext.h>
#include <analysis/Utils.h>
#include <analysis/VarBinding.h>

#include "FuncInfo.h"

namespace RAnnot {

//****************************************************************************
// FuncInfo
//****************************************************************************

FuncInfo::FuncInfo(FuncInfo *lexParent, SEXP name, SEXP defn) :
  mLexicalParent(lexParent),
  mFirstName(name),
  mDefn(defn),
  mCName(""),
  mClosure(""),
  NonUniformDegreeTreeNodeTmpl<FuncInfo>(lexParent)
{
  mRequiresContext = functionRequiresContext(defn);
  SEXP args = getArgs();
  for (SEXP e = args; e != R_NilValue; e = CDR(e)) {
    FormalArgInfo * fargInfo = new FormalArgInfo();
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

const std::string& FuncInfo::getCName()
{
  if (mCName == "") {
    SEXP name_sym = getFirstName();
    if (name_sym == R_NilValue) {
      mCName = make_c_id("anon" + ParseInfo::global_fundefs->new_var_unp());
    } else {
      mCName = make_c_id(ParseInfo::global_fundefs->new_var_unp_name(var_name(getFirstName())));
    }
  }
  return mCName;
}

const std::string& FuncInfo::getClosure()
{
  if (mClosure == "") {
    mClosure = ParseInfo::global_fundefs->new_sexp_unp();
  }
  return mClosure;
}

void FuncInfo::insertMention(FuncInfo::MentionT v)
{
  mMentions.push_back(v);
}

void FuncInfo::insertCallSite(FuncInfo::CallSiteT cs)
{
  mCallSites.push_back(cs);
}

std::ostream& FuncInfo::dump(std::ostream& os) const
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

PropertyHndlT FuncInfo::handle() {
  return FuncInfoAnnotationMap::handle();
}

}

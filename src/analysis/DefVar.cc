#include "DefVar.h"

#include <support/DumpMacros.h>

namespace RAnnot {


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

}

#include <include/R/R_RInternals.h>

#include <support/DumpMacros.h>

#include "UseVar.h"

namespace RAnnot {

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

const std::string typeName(const Var::UseDefT x)
{
  switch(x) {
  case Var::Var_USE: return "USE";
  case Var::Var_DEF: return "DEF";
  }
}

const std::string typeName(const UseVar::PositionT x)
{
  switch(x) {
  case UseVar::UseVar_FUNCTION: return "FUNCTION";
  case UseVar::UseVar_ARGUMENT: return "ARGUMENT";
  }
}

}

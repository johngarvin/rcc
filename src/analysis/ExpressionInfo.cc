#include <support/DumpMacros.h>

#include <analysis/ExpressionInfoAnnotationMap.h>
#include <analysis/Var.h>

#include "ExpressionInfo.h"

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

} // end namespace RAnnot

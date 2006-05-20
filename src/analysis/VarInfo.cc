#include <include/R/R_RInternals.h>

#include <support/DumpMacros.h>

#include <analysis/DefVar.h>

#include "VarInfo.h"

namespace RAnnot {

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

}

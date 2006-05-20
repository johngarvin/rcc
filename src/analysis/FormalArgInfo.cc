#include <ostream>

#include <support/DumpMacros.h>

#include <analysis/FormalArgInfoAnnotationMap.h>
#include <analysis/PropertyHndl.h>

#include "FormalArgInfo.h"

namespace RAnnot {

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


std::ostream& FormalArgInfo::dump(std::ostream& os) const
{
  beginObjDump(os, FormalArgInfo);
  // FIXME: add implementation
  endObjDump(os, FormalArgInfo);
}

PropertyHndlT FormalArgInfo::handle() {
  return FormalArgInfoAnnotationMap::handle();
}

}

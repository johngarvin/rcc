#include <support/DumpMacros.h>

#include "Phi.h"

namespace RAnnot {

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

}

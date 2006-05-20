#include <ostream>

#include <support/DumpMacros.h>

#include <analysis/PropertyHndl.h>
#include <analysis/VarAnnotationMap.h>

#include "Var.h"

namespace RAnnot {

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

}

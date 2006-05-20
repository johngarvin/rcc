//****************************************************************************
// SymbolTable
//****************************************************************************

#include <ostream>

#include <support/DumpMacros.h>

#include <analysis/SymbolTableAnnotationMap.h>
#include <analysis/VarInfo.h>

#include "SymbolTable.h"

namespace RAnnot {

SymbolTable::SymbolTable()
{
}


SymbolTable::~SymbolTable()
{
}

PropertyHndlT SymbolTable::handle() {
  return SymbolTableAnnotationMap::handle();
}

std::ostream& SymbolTable::dump(std::ostream& os) const
{
  beginObjDump(os, SymbolTable);
  for (const_iterator it = this->begin(); it != this->end(); ++it) {
    dumpObj(os, it->second);
  }
  endObjDump(os, SymbolTable);
}

} // end namespace RAnnot

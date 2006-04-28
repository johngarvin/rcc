
//****************************************************************************
// SymbolTable
//****************************************************************************

#include <ostream>

#include <support/DumpMacros.h>

#include <analysis/Annotation.h>
#include <analysis/SymbolTableAnnotationMap.h>

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
    os << "(";
    Rf_PrintValue(it->first);
    os << " --> ";
    dumpObj(os, it->second);
    os << ")\n";
  }
  endObjDump(os, SymbolTable);
}

} // end namespace RAnnot

// -*-Mode: C++;-*-
// $Header

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source
//
// Purpose:
//   DumpMacros.h 
//
// Description:
//   a set of stylized macros for variable and object dumping 
//
//***************************************************************************

//************************* System Include Files ****************************

#include <ostream>


//************************* Macro Definitions *******************************


#define beginObjDump(os, name) os << "{ " << #name << ":" << std::endl

#define dumpVarStart(os, var) os << "{ " << #var << " = " 

#define dumpVarEnd(os, var) os << "}" << std::endl

#define dumpVar(os, var) \
  dumpVarStart(os, var); \
  os << var << " "; \
  dumpVarEnd(os, var)

#define dumpName(os, var)     \
  dumpVarStart(os, var);      \
  os << typeName(var) << " "; \
  dumpVarEnd(os, var)

#define dumpPtr(os, ptr) \
  dumpVarStart(os, ptr); \
  os << (void *) ptr; \
  dumpVarEnd(os, ptr)

#define dumpSEXP(os, sexp) \
  dumpVarStart(os, sexp); \
  os << "(WARNING: Rf_PrintValue does not dump in target ostream)" << std::endl;	\
  Rf_PrintValue(sexp);	  \
  dumpVarEnd(os, sexp)

#define dumpObj(os, obj) \
  dumpVarStart(os, obj); os << std::endl; \
  obj->dump(os); \
  dumpVarEnd(os, obj)

#define dumpString(os, str) \
  dumpVarStart(os, str);    \
  os << str;                \
  dumpVarEnd(os, str)

#define endObjDump(os, name) os << "}" << std::endl; os.flush()

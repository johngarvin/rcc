#ifndef ASSERTION_H
#define ASSERTION_H

#include <include/R/R_RInternals.h>
#include <analysis/Annotation.h>

namespace RAnnot {

// forward declarations

class FuncInfo;

void process_assert(SEXP e, FuncInfo* fi);

} // end of RAnnot namespace

#endif

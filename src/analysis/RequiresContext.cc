#include <assert.h>
#include <set>

#include <include/R/R_Internal.h>
#include <analysis/Utils.h>

#if 0
using namespace _GLIBCXX_STD;
#else
using namespace std;
#endif

#define PRIM(x) (CCODE) x
static CCODE unsafePrimitives[] = {

  // "as" functions,
  PRIM(do_ascharacter),
  PRIM(do_asvector),

  // "dim" functions),
  PRIM(do_dim),
  PRIM(do_dimgets),
  PRIM(do_dimnames),
  PRIM(do_dimnamesgets),

  // "is" functions),
  PRIM(do_is),
  PRIM(do_isna),
  PRIM(do_isnan),

  // "subassign" functions),
  PRIM(do_subassign),
  PRIM(do_subassign2),
  PRIM(do_subassign3),

  // "subset" functions),
  PRIM(do_subset),
  PRIM(do_subset2),
  PRIM(do_subset3)
};


class PrimitiveRequiresContext {
private:
  set<CCODE> theSet;
public:
  PrimitiveRequiresContext(); 
  bool getPrimitiveRequiresContext(CCODE prim);
};

PrimitiveRequiresContext primRequiresContext;

//------------------------------------------------------------------------------
// forward declarations
//------------------------------------------------------------------------------

static bool listRequiresContext(SEXP e);
static bool expressionRequiresContext(SEXP e);



//------------------------------------------------------------------------------
// class methods
//------------------------------------------------------------------------------

PrimitiveRequiresContext::PrimitiveRequiresContext()
{
  int count = sizeof(unsafePrimitives)/sizeof(CCODE);
  for (int i=0;i < count; i++) {
    theSet.insert(unsafePrimitives[i]);
  }
}

bool PrimitiveRequiresContext::getPrimitiveRequiresContext(CCODE prim)
{
  return theSet.find(prim) != theSet.end();
}



//------------------------------------------------------------------------------
// interface functions
//------------------------------------------------------------------------------

bool functionRequiresContext(SEXP fundef)
{
  SEXP code = CAR(fundef_body_c(fundef));
  return expressionRequiresContext(code);
}

static bool listRequiresContext(SEXP e)
{
  if (e == R_NilValue) return false;

  if (expressionRequiresContext(CAR(e))) return true;

  return expressionRequiresContext(CDR(e));
}

static bool expressionRequiresContext(SEXP e)
{
  switch(TYPEOF(e)) {
  case NILSXP:
  case STRSXP:
  case LGLSXP:
  case INTSXP:
  case REALSXP:
  case CPLXSXP:
  case VECSXP:
  case SYMSXP:
  case CHARSXP:
  case CLOSXP:
  case ENVSXP:
  case EXPRSXP:
  case EXTPTRSXP:
  case WEAKREFSXP:
    return false;
  case LISTSXP:
    return listRequiresContext(e);
  case PROMSXP:
    return expressionRequiresContext(PREXPR(e));
  case LANGSXP:
    return expressionRequiresContext(Rf_findFunUnboundOK(CAR(e), 
							 R_GlobalEnv, TRUE));
  case SPECIALSXP:
  case BUILTINSXP:
    return primRequiresContext.getPrimitiveRequiresContext(PRIMFUN(e)); 
  default:
    assert(0 && "Internal error: requiresContext encountered bad type\n");
    return false;
  }
}

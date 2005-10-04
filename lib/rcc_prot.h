#include "R/Defn.h"

#if 1
#undef PROTECT
#define PROTECT(s) rcc_protect(s)
#define SAFE_PROTECT(s) R_PPStack[R_PPStackTop++] = s;
#endif

#undef UNPROTECT
#define UNPROTECT(n) R_PPStackTop -= n

extern void protect_error();

static inline SEXP rcc_protect(SEXP s)
{
    if (R_PPStackTop >= R_PPStackSize) {
      Rf_protect_error();
    }
    R_PPStack[R_PPStackTop++] = s;
    return s;
}

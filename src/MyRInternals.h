#ifndef MYRINTERNALS_H
#define MYRINTERNALS_H

#define USE_RINTERNALS
#include <Rinternals.h>

// Prevent conflict with basicstring::{append, length} on Alpha
#undef append
#undef length

#endif

source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# This exposes a bug in requiresContext.

f <- function(x) 3 + x

g <- function() f(30)

g()

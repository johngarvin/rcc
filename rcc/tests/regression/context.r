source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# This exposes a bug that used to exist in requiresContext.

f <- function(x) 3 + x

g <- function() f(30)

g()

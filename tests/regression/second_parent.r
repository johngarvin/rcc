source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# Exposes an ExprTreeBuilder bug: OA throws SecondParent exception if
# we don't do this right

f <- function(x) x
i <- 2

f(cos(i + i))

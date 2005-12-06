source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- function(x) x + 2
foo(0)
foo <- function(x) x + 3
foo(0)

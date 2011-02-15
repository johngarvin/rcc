source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- 1
f <- function(x) x + foo
foo <- 2
f(100)
foo <- 3
f(100)

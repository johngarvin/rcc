source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- function (x) return(x + 12)

foo(30)

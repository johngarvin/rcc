source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

outer <- function(a) a
outer(6)

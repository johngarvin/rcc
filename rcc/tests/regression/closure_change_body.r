source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

f <- function() 10
f()
body(f) <- list(100)
f()

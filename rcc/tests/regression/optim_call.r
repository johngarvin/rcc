source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

f <- function(x) { x[1] * (x[2] + 5) }

optim(c(10,20), f, "BFGS")

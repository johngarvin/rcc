source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

f <- function(x,y) x + 10*y
f(1,2)
formals(f) <- list(y=missing,x=missing)
f(1,2)

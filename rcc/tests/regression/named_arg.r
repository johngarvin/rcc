source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

f <- function(x,y) 2*x + y

f(y=3,x=4)

# a bug in ExprTreeBuilder causes this to terminate with a SecondParent exception in OA

source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

f <- function(x) {
  x + 100
}

a <- 1
b <- 2
f(a + (b + 1))

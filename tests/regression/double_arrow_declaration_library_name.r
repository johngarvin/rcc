source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

g <- function() {
  print(qnorm)
}
f <- function() {
  qnorm <<- 37
}
f()
g()

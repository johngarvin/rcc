source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

g <- function() {
  print(v)
}
f <- function() {
  v <<- 37
}
f()
g()

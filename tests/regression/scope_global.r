source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

x <- 10
foo <- function() x
bar <- function() {
  x <- 20
  foo()
}
bar()

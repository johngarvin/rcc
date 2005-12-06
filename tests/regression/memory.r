source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

baz <- function() {
  bar <- 12
  for (i in 1:10000) {
    foo <- list(i)
  }
  return(bar)
}
baz()

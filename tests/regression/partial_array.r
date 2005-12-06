source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- matrix("foo",3,4)
baz <- function(x) {
  bar <- foo[,1]
  bar
}
baz(2)

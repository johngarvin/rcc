source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

bar <- function() {
  foo <- 34
  foo
}

foo <- 12
bar()
foo

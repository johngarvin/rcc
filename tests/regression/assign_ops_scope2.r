source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- function() {
  n <<- 10
}

bar <- function() {
  n <<- 5
  foo()
  n
}

bar()

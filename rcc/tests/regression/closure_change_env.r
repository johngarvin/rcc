source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- 1.2

f <- function() {
  foo <- 3.4
  g <- function() foo
  print(g())
  environment(g) <- .GlobalEnv
  g()
}
f()

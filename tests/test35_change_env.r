foo <- 1.2

f <- function() {
  foo <- 3.4
  g <- function() foo
  print(g())
  environment(g) <- .GlobalEnv
  g()
}
f()

x <- 10
foo <- function() x
bar <- function() {
  x <- 20
  foo()
}
bar()

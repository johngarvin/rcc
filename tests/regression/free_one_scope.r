f <- function() {
  x <- 1
  g <- function() {
    x <<- 2
    x
  }
  g()
  x
}
f()

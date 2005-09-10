f <- function() {
  x <- 1
  g <- function() {
    x <- x
    h <- function() {
      x <- x
      x <<- x
      x
    }
    h()
  }
  g()
}
f()

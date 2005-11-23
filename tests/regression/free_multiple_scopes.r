rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

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

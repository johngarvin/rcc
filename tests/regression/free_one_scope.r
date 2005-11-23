rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

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

source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

factorial <- function(n) {
  if (n == 0) {
    return(1)
  } else {
    return(n * factorial(n-1))
  }
}

factorial(10)

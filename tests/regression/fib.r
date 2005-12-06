source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

Rprof("prof-output")
fib <- function(n) {
  if (n == 0) return(1)
  else if (n == 1) return(1)
  else return (fib(n-1) + fib(n-2))
}

fib(3)
Rprof(NULL)

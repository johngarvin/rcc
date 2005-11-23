rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

fib <- function(n) {
  if (n == 0) 1
  else if (n == 1) 1
  else fib(n-1) + fib(n-2)
}

fib(3)

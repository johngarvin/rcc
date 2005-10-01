Rprof()
fib <- function(n) {
  if (n == 0) return(1)
  else if (n == 1) return(1)
  else return (fib(n-1) + fib(n-2))
}

fib(32)
Rprof(NULL)

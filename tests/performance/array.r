Rprof()
f <- function() {
  foo <- array(1:1000000, dim=c(100,50,200))
  bar <- c(1:1000, dim=c(1000000,3))
  for (i in 1:100) {
    foo[bar>42] <- foo[bar>42]
  }
  print(foo[10,3,2])
}
f()
Rprof(NULL)

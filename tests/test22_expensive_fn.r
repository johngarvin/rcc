Rprof("prof-output")
bar <- 0
foo <- function(a,b) 3*a - 4*b
for(i in 1:4000000) {
  bar <- bar + foo(20,15)
}
print(bar)
Rprof(NULL)

foo <- function(a,b) 3*a - 4*b
bar <- 0
for(i in 1:100000) bar <- bar + foo(20,15)
bar

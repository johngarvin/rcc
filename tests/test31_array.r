foo <- array(1:10000000, dim=c(1000,50,200))
bar <- c(1:1000, dim=c(10000000,3))
#print(foo)
#print(bar)
foo[bar] <- 42
foo <- foo
foo <- foo
foo <- foo
print(foo[4,2,10])

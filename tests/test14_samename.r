x <- 14
y <- 15

baz <- function(z) {
  for (i in 1:3) {
    bar <- foo(i,x,y)
    x <- bar$x
    y <- bar$y
    print(bar)
  }
}

foo <- function(i,x,y) {
  for(i in 1:10) {
    x <- 16
    y <- 17
  }
  return (i,x,y)
}

bar <- baz(10)
x <- bar$x
y <- bar$y

foo <- matrix("foo",3,4)
baz <- function(x) {
  bar <- foo[,1]
  bar
}
baz(2)

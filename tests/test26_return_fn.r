foo <- function(n)
{
  bar <- function(x) return(x)
  return(bar)
}

baz <- function(n)
{
  if (n == 0) foo(3) else foo(baz(n-1))
}
baz(50)

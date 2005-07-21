"t#%*%#t" <- function(x,y)
{
  return(.Call("do_matprod_t", as.vector(x), as.vector(y), "t", "t"))
}

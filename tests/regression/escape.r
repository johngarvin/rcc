source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

x <- c(1,2,3)

f <- function() {
  esc <- c(4,5,6)
  ret <- c(7,8,9)
  x <<- esc
  return(ret)
}

f()
x

source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- function(n) {
  n[2] <- sqrt(49)
  return(n)
}

bar <- function(n) {
  n <- 40
  return(n)
}

foo(c(20,30))
bar(50)

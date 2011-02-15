source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# a bug in FirstMention information may cause this to segfault

foo <- function(x) {
  if (x == 1) x <- 20
  return(x)
}

foo(1)

source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

f <- function(n) {

  print(n)
  
  for (i in 1:n) {
    print(i)
  }
}

f(10)

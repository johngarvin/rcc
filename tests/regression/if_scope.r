source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

a <- 3

function foo() {
  if (a == 1) x <- 10
  else if (a == 2) x <- 20
  else x <- 30
  print(x)
}

foo()

source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# this test finds out whether we're using call-by-need or call-by-value

foo <- function(x) {
  print("I'm a side effect.")
  print(x)
}

foo(print("I'm a side effect too."))

source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

for (i in 1:5) {
  foo <- "pizza"
  foo <- function (x) 5 * x
  print(foo(2))
}

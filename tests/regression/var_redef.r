rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

for (i in 1:5) {
  foo <- "pizza"
  foo <- function (x) 5 * x
  print(foo(2))
}

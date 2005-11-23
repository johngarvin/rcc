rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

foo <- function(n) {
  bar <- function (n) {
    n + 1
  }
  bar(n) + 3
}
foo(10)

source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- function () {
  bar(10,20)
}
bar <- function (x, y) {
  8 * x + y
}
foo()

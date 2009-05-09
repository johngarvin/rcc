source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

myfun <- function(x) { x * (x + 5) }

call.sum <- function(f, a, b) {
  f(a) + f(b)
}

call.sum(myfun, 10, 20)

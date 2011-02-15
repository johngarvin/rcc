source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# simplified version of a function in beeson.r that seems to give the
# compiler trouble

f <- function(x) {
  while (max(x > pi))
    x <- x - 1
  while (max(x < -pi))
    x <- x + 1
  x
}
f(10)
f(-11)

# Adapted from a scheme example in Olin Shivers's dissertation. Tests
# for naive constant propagation.

source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

f <- function(x, h) {
  if (x == 0) {
    h()
  } else {
    function() x
  }
}

f(0, f(3, 100))

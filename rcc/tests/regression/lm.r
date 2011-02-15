source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# Based on geneshaving. Formerly exposed a bug in argument resolution

set.seed(602)

f <- function(x) {
  foo <- lm(as.vector(x[1:(length(x)/2)]) ~ x[(length(x)/2 + 1):(length(x))])
  foo$fit
}

f(c(11,21,31))

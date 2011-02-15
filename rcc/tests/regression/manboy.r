# Knuth's "man vs. boy" test, translated from Algol

# Note: original has 10 as first argument to A, but the R interpreter
# runs out of protect stack space when given 10

source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

A <- function(k, x1, x2, x3, x4, x5) {
  B <- function() {
    k <<- k - 1
    return(A(k, B, x1, x2, x3, x4))
  }
  if (k <= 0) {
    return(x4() + x5())
 } else {
   return(B())
 }
}
print(A(9, function() 1, function() -1, function() -1, function() 1, function() 0))

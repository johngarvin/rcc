source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# redefinition of a library name as a function side effect

# Create a binding in the user's global scope to be redefined.
# Otherwise, R produces an error as it tries to redefine an immutable
# name in the library scope.
qnorm <- qnorm

foo <- function() {
  # redefine 'qnorm' in the global scope
  # old qnorm: quantile function for the normal distribution
  # new qnorm: quote Norm from "Cheers"
  qnorm <<- function(x) {
    "Just the usual, Coach. I'll have a trough of beer and a snorkel."
  }
}

qnorm(0.123)
foo()
qnorm(0.123)

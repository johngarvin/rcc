source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# redefinition of a library name with a free variable use

foo <- function() {
  qnorm(0.123)
}

foo()
# old qnorm: quantile function for the normal distribution
# new qnorm: quote Norm from "Cheers"
qnorm <- function(x) {
  "Just the usual, Coach. I'll have a trough of beer and a snorkel."
}
foo()

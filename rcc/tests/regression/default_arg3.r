source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# call to a function with some non-default arguments not supplied as actuals

approx(c(1,2,3))

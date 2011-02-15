source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

x <- length(c(1,2,3))
foo <- 2
for (i in 1:x) { foo <- foo + i }
foo + 12

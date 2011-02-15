source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- 3
n <- 11
for (i in 2:n) { foo <- foo * i + 1 }
foo + 12

source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- 3
n1 <- 4
n2 <- 5
for (i in 2:(n1+n2)) { foo <- foo * i + 1 }
foo + 12

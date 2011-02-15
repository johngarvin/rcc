source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- 3
lst <- list(1, 11, 111, 1111, 42, 81)
for (i in lst) { foo <- foo * i + 1 }
foo + 12

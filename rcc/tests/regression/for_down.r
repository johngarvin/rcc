source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- 3
for (i in 10:1) { foo <- foo * i + 1 }
foo + 12

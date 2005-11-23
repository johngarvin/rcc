rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

foo <- 2
for (i in 1:10) { foo <- foo + i }
foo + 12

source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

a <- 3
if (a == 3) print("good") else print("bad")

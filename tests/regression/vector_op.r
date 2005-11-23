rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

foo <- 4
bar <- 6
foo <- 2
bar1 <- c(1,2,3,4)
foo + bar1

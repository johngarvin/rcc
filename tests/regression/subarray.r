rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

prob <- matrix(0,3,2)
prob
prob[,1] <- 4
prob
prob[,1:1] <- 3

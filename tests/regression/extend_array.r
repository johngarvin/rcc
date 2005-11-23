rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

a <- 1:10
a[7] <- 10
a[14] <- 20
a[21] <- 30
a[20]
a[21]

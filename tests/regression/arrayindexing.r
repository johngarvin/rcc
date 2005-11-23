rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

foo <- c(10,20,30,40,50,60,70,80,90,100)
bar <- foo[3]
bar

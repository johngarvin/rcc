source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- matrix(c(10,20,30,40,50,60,70,80,90,100,110,120), nrow=3)
bar <- foo[3,4]
bar

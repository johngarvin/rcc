source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- c(10,20,30,40,50,60,70,80,90,100)
foo[] <- c(1,2,3,4,5)
foo

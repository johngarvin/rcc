source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- c(10,20,30,40,50,60,70,80,90,100,110,120)
fooi <- c(2,4,6,8,10,12,1,3,5,7,9,11)
bar <- foo[fooi[3]]
bar

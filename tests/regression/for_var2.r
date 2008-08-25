source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- 3
bar <- c(0,0,0)
baz <- c(1,1,1,2,2,2,2,3,3)
for (i in 1:foo) { bar[i] <- sum(baz == i) }
bar

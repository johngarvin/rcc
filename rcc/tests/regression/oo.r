source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

d <- data.frame(cbind(x=1, y=1:10), fac=sample(c("A","B","C"), 10, repl=TRUE))
print(d$y)

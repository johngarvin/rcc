source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- 2
for (i in 1:10) { 
	if (i %% 2 == 1) 
		next
	foo <- foo + i 
}
foo + 12

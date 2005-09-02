foo <- 2
for (i in 1:10) { 
	if (i %% 2 == 1) 
		next
	foo <- foo + i 
}
foo + 12

rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

foo <- 2
for (k in 1:10){
for (i in k:10) { 
	if (i == k+3) 
		break
	foo <- foo + i 
}
for (j in k:10) { 
for (i in j:j+5) { 
	if (i == j+4) 
		break
	foo <- foo + i 
}
	if (j == k+3) 
		break
	foo <- foo + i 
}
}
foo + 12

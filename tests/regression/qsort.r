source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

qsort <- function(x) {
  if (length(x) <= 1) return(x)
  pivot <- x[1]
  return(c(qsort(x[x < pivot]), x[x == pivot], qsort(x[x > pivot])))
}

qsort(c(10,2,30,40,0,0,3,4,100,5))
qsort(c(1,2,3,4,5,6,7,8,9,10))
qsort(c(10,20,340,3,5,30.6,10,0,0,10))

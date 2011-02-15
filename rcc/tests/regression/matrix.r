source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# Exposes a strange bug with MatrixAssign.

stoporcontfn <- function() {

  reject <- matrix(0,nrow=150,ncol=150)
  reject[1,1] <- 3

  print(reject[1,1])
  print(reject[2,2])
}
stoporcontfn()

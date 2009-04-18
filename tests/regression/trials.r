source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# Exposes a strange bug with MatrixAssign.

stoporcontfn <- function() {

  reject <- matrix(0,nrow=150,ncol=150)
  reject[1,1] <- 0

  print(reject)
  return(reject)
}
stoporcontfn()

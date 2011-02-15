source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

nsim <- 300
mustar.rstar.list <- vector("list", nsim)

for (i in 1:nsim) {
  mustar.rstar.list[[i]] <- cbind(1,2,3)
}

mustar.rstar.list[200]

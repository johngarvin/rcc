source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# based on mgg00. This will give an error if we let rcc_subassign_cons
# mutate the arg list in the constant pool.

for(i in 4:5) {
  x <- vector("integer", i)
  x[i] <- 10
  print(x)
  x <- matrix(111, 3, i)
  x[3,] <- x[3,] + 1
  print(x)
  x <- array(71, c(3, i, 4))
  x[2,,] <- x[2,,] + 1
  print(x)
}

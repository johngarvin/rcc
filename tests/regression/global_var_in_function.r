source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

f <- function() {
  x <<- 3
  if (x == 3) print("ok")
}

f()

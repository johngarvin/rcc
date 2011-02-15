source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

a <- 10

f <- function(x = a + 5, y) {
  if (y) a <- 20
  print(x)
}

f(y = TRUE)
f(y = FALSE)
